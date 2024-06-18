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

  class CameraController
  {
  public:
    CameraController(const libcamera::PixelFormat &format, uint32_t width, uint32_t height, rclcpp::Logger &logger);
    ~CameraController();

    void Start(bool autoRestartFlag = true);

    void AddProcessor(std::shared_ptr<FrameProcessingInterface> proc);

    int GetReqNumberAndReset() {return std::exchange(m_receivedRequestsNumber, 0);}

    void SetActive(bool active);

  private:
    libcamera::CameraManager m_cm;
    std::shared_ptr<libcamera::Camera> m_camera;
    std::unique_ptr<libcamera::FrameBufferAllocator> m_allocator;
    libcamera::Stream *m_stream = nullptr;
    std::vector<std::unique_ptr<libcamera::Request>> m_requests;
    bool m_autoRestartFlag;

    int m_receivedRequestsNumber = 0;

    libcamera::Rectangle m_scalerCrop;
    bool m_scalerCropChanged;

    std::vector<std::shared_ptr<FrameProcessingInterface>> m_processors;
    std::mutex m_procMutex;
    std::mutex m_startRequestFence;

    std::map<libcamera::Request *, void *> m_memoryRequestMaping;

    rclcpp::Logger &m_logger;

    void OnRequestComplete(libcamera::Request *request);
    void SetDefaultControls();

    std::ostream &Log() const { return std::cout; }
  };

} // camera_utility

#endif