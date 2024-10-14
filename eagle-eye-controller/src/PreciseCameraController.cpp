#include "PreciseCameraController.h"
#include <sys/mman.h>
#include <unistd.h>

#include <chrono>
#include "rclcpp/rclcpp.hpp"

namespace camera_utility
{

const char PreciseCameraController::s_cameraSensorName[] = "arducam_64mp";

PreciseCameraController::PreciseCameraController(const libcamera::PixelFormat &format, uint32_t width, uint32_t height) :
    m_width{width}
  , m_height{height}
  , m_scalerCropChanged{false}
  , m_status{PreciseCameraController::Status::NOT_READY}
{
    m_cm.start();

    const auto camList = m_cm.cameras();
    if (camList.empty()) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Cant find active camera");
        return;
    }

    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Detected cameras:");
    int eagle_number = -1;
    for (size_t i = 0; i < camList.size(); i++) {
        RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "      %s", camList[i]->id().c_str());
        auto pos = camList[i]->id().find(s_cameraSensorName);
        if (pos != std::string::npos) {
            eagle_number = i;
        }
    }
    if (eagle_number == -1) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Failed to find high resulution camera");
        return;
    }
    const std::string cameraId = camList[eagle_number]->id();
    m_camera = m_cm.get(cameraId);
    m_camera->acquire();

    std::unique_ptr<libcamera::CameraConfiguration> configs =
        m_camera->generateConfiguration({libcamera::StreamRole::Raw});

    if (configs->empty()) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Empty camera config");
        m_camera.reset();
        return;
    }

    auto &config = configs->at(0);
    config.pixelFormat = format;
    config.size.width = width;
    config.size.height = height;
    config.bufferCount = 1;

    if (configs->validate() == libcamera::CameraConfiguration::Status::Invalid) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Wrong config");
        m_camera.reset();
        return;
    }

    m_camera->configure(configs.get());

    m_allocator = std::make_unique<libcamera::FrameBufferAllocator>(m_camera);

    int ret = m_allocator->allocate(config.stream());
    if (ret < 0) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Can't allocate buffers");
        m_camera.reset();
        return;
    }

    m_stream = config.stream();
    const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers = m_allocator->buffers(m_stream);
    if (buffers.size() != config.bufferCount) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Wrong buffers number");
    }

    for (unsigned int i = 0; i < config.bufferCount; i++) {
        m_requests.emplace_back(m_camera->createRequest());
        if (!m_requests[i]) {
            RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Can't create request");
            m_camera.reset();
            return;
        }

        auto ret = m_requests[i]->addBuffer(m_stream, buffers[i].get());
        if (ret < 0) {
            RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Can't set buffer for request");
            m_camera.reset();
            return;
        }
    }

    SetDefaultControls();

    m_camera->requestCompleted.connect(this, &PreciseCameraController::OnRequestComplete);
    ret = m_camera->start();
    if (ret < 0) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Can't start camera");
        return;
    }

    m_status = PreciseCameraController::Status::READY;
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Camera initialization is done");
}

PreciseCameraController::~PreciseCameraController() {
    m_autoRestartFlag = false;
    if (m_camera) {
        m_camera->stop();
        m_camera->release();
        m_allocator->free(m_stream);
        m_allocator.reset();
        m_camera.reset();
    }

    m_cm.stop();
}

void PreciseCameraController::Start(bool autoRestartFlag) {
    std::lock_guard<std::mutex> fence{m_startRequestFence};
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Starting. N requsts=%lu", m_requests.size());
    m_autoRestartFlag = autoRestartFlag;
    for (const auto &req : m_requests) {
        m_camera->queueRequest(req.get());
    }
}

void PreciseCameraController::SetDefaultControls() {
    for (const auto &req : m_requests) {
        libcamera::ControlList &controls = req->controls();
        controls.set(libcamera::controls::AeEnable, true);
        controls.set(libcamera::controls::AwbEnable, true);
    }
}

void PreciseCameraController::SetViewPort(uint32_t center_X, uint32_t center_Y, uint32_t width, uint32_t height)
{
    libcamera::Rectangle scalerCrop(center_X - width/2, center_Y - height/2, width, height);
    if (CheckViewPort(scalerCrop))
    {
        m_scalerCrop = scalerCrop;
        m_scalerCropChanged = true;
    }
}

void PreciseCameraController::SetViewPortOrigin(uint32_t center_X, uint32_t center_Y) {
    auto newScalerCrop = m_scalerCrop;
    newScalerCrop.x = center_X - m_scalerCrop.width / 2;
    newScalerCrop.y = center_Y - m_scalerCrop.height / 2;
    if (CheckViewPort(newScalerCrop))
    {
        m_scalerCrop = newScalerCrop;
        m_scalerCropChanged = true;
    }
}

void PreciseCameraController::SetViewPortSize(uint32_t width, uint32_t height) {
    auto newScalerCrop = m_scalerCrop;
    newScalerCrop.width = width;
    newScalerCrop.height = height;
    if (CheckViewPort(newScalerCrop))
    {
        m_scalerCrop = newScalerCrop;
        m_scalerCropChanged = true;
    }
}

bool PreciseCameraController::CheckViewPort(const libcamera::Rectangle &rect) {
    return rect.topLeft().x >= 0 && rect.topLeft().x + rect.size().width <= k_matrixWidth &&
           rect.topLeft().y >= 0 && rect.topLeft().y + rect.size().height <= k_matrixHeight;
}

void PreciseCameraController::ValidateScalerCrop(libcamera::Request *request) {
    if (m_scalerCropChanged)
    {
        libcamera::ControlList &controls = request->controls();
        controls.set(libcamera::controls::ScalerCrop, m_scalerCrop);
        m_scalerCropChanged = false;
    }
}

void PreciseCameraController::AddProcessor(std::shared_ptr<FrameProcessingInterface> proc) {
    std::lock_guard<std::mutex> lck{m_procMutex};
    m_processors.emplace_back(proc);
    proc->SetFrameSize(m_width, m_height);
}

void PreciseCameraController::OnRequestComplete(libcamera::Request *request) {
    if (request->status() == libcamera::Request::RequestCancelled) {
        return;
    }

    m_receivedRequestsNumber++;

    std::lock_guard<std::mutex> fence{m_startRequestFence};
    const libcamera::Request::BufferMap &buffers = request->buffers();
    for (auto &[stream, buffer] : buffers) {
        const auto &cfg = stream->configuration();
        const auto stride = cfg.stride;

        if (buffer->planes().size() > 0) {
            const libcamera::FrameBuffer::Plane &plane = buffer->planes()[0];
            const int fd = plane.fd.get();
            // const size_t length = ::lseek(fd, 0, SEEK_END);
            size_t mapLength = static_cast<size_t>(plane.offset + plane.length);
            auto mem_it = m_memoryRequestMaping.find(request);
            void *address;
            if (mem_it == m_memoryRequestMaping.end()) {
                address = ::mmap(nullptr, mapLength, PROT_READ, MAP_SHARED, fd, 0);
                m_memoryRequestMaping[request] = address;
            } else {
                address = mem_it->second;
            }
            if (address != MAP_FAILED) {
                std::lock_guard<std::mutex> lck{m_procMutex};
                for (auto proc : m_processors) {
                    proc->ProcessFrame(reinterpret_cast<uint8_t *>(address), stride);
                }
            }
        }
    }
            
    request->reuse(libcamera::Request::ReuseBuffers);

    if (m_autoRestartFlag) {
        ValidateScalerCrop(request);
        m_camera->queueRequest(request);
    }
}

void PreciseCameraController::SetActive(bool active) {
    m_autoRestartFlag = active;
    if (active) {
        Start();
    }
}

} // camera_utility
