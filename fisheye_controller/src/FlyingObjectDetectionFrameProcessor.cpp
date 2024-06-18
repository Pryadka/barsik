#include "FlyingObjectDetectionFrameProcessor.h"

namespace camera_utility
{
    
OpenCVFrameProcessor::OpenCVFrameProcessor(uint32_t width, uint32_t height):
m_width(width), m_height(height) 
{
}

OpenCVFrameProcessor::~OpenCVFrameProcessor() {
}

void OpenCVFrameProcessor::ProcessFrame(uint8_t *address, uint32_t stride) {
    cv::Mat image(m_height, m_width, CV_8UC3, address, stride);

    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_frame = image.clone();
    }
}

bool OpenCVFrameProcessor::HasFrame() const {
    return !m_frame.empty();
}

const cv::Mat OpenCVFrameProcessor::GetLatestFrame() {
    std::lock_guard<std::mutex> lock(m_mtx);
    if (!m_frame.empty()) {
        return m_frame;
    }
    return cv::Mat();
}

} // namespace camera_utility