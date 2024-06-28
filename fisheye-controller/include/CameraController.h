#ifndef __camera_utility__
#define __camera_utility__

#include <libcamera/libcamera.h>
#include <memory>
#include <vector>
#include <mutex>
#include <iostream>
#include "FrameProcessingInterface.h"
#include "rclcpp/logger.hpp"

namespace camera_utility
{

class CameraController {
public:
    enum class Status : uint8_t {
        NOT_READY = 0,
        READY,
    };
    
    CameraController(const libcamera::PixelFormat &format, uint32_t width, uint32_t height);
    ~CameraController();

    void Start(bool autoRestartFlag = true);

    void AddProcessor(std::shared_ptr<FrameProcessingInterface> proc);

    int GetReqNumberAndReset() {return std::exchange(m_receivedRequestsNumber, 0);}

    void SetActive(bool active);

    Status GetStatus() const {return m_status;}

private:
    libcamera::CameraManager m_cm;
    std::shared_ptr<libcamera::Camera> m_camera;
    std::unique_ptr<libcamera::FrameBufferAllocator> m_allocator;
    libcamera::Stream *m_stream = nullptr;
    std::vector<std::unique_ptr<libcamera::Request>> m_requests;
    bool m_autoRestartFlag;

    uint32_t m_width;
    uint32_t m_height;
    
    Status m_status;

    int m_receivedRequestsNumber = 0;

    std::vector<std::shared_ptr<FrameProcessingInterface>> m_processors;
    std::mutex m_procMutex;
    std::mutex m_startRequestFence;

    std::map<libcamera::Request *, void *> m_memoryRequestMaping;

    void OnRequestComplete(libcamera::Request *request);
    void SetDefaultControls();

    static const char s_cameraSensorName[];
};

} // camera_utility

#endif