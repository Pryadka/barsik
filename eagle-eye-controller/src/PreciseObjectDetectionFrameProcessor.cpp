#include "PreciseObjectDetectionFrameProcessor.h"

namespace camera_utility
{
    
PreciseObjectDetectionFrameProcessor::PreciseObjectDetectionFrameProcessor(rclcpp::Publisher<barsik_interfaces::msg::PreciseDetections>::SharedPtr publisher) :
    m_detector(1, "/home/sasa-r/src/barsik/yolov8n_full_integer_quant_edgetpu.tflite")
  , m_publisher(publisher)
  , m_waitForFrameSemaphore(0)
{
    m_detectionThread = std::thread([this](){
        while(m_run) {
            m_waitForFrameSemaphore.acquire();
            decltype(m_detector.ExecuteInference(m_frame.data)) detections;
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                detections = m_detector.ExecuteInference(m_frame.data);
            }
            std::vector<barsik_interfaces::msg::Detection> result;
            for (const auto &det : detections) {
                barsik_interfaces::msg::Detection res_det;
                res_det.center_x = det.center.x;
                res_det.center_y = det.center.y;
                res_det.width = det.size.x;
                res_det.height = det.size.y;
                res_det.object_class = det.object_class;
                result.emplace_back(res_det);
            }
            barsik_interfaces::msg::PreciseDetections msg;
            msg.objects = result;
            m_publisher->publish(msg);
        }
    });
}

PreciseObjectDetectionFrameProcessor::~PreciseObjectDetectionFrameProcessor() {
    m_run = false;
    m_detectionThread.join();
}

void PreciseObjectDetectionFrameProcessor::ProcessFrame(uint8_t *address, uint32_t stride) {
    cv::Mat image(m_width, m_height, CV_8UC3, address, stride);
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_frame = image.clone();
    }
    m_waitForFrameSemaphore.release();
}

bool PreciseObjectDetectionFrameProcessor::HasFrame() const {
    std::lock_guard<std::mutex> lock(m_mtx);

    return !m_frame.empty();
}

const cv::Mat PreciseObjectDetectionFrameProcessor::GetLatestFrame() {
    std::lock_guard<std::mutex> lock(m_mtx);
    if (!m_frame.empty()) {
        return m_frame;
    }
    return cv::Mat();
}

} // namespace camera_utility