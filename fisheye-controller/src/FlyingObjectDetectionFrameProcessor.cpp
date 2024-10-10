#include "FlyingObjectDetectionFrameProcessor.h"

namespace camera_utility
{
    
FlyingObjectDetectionFrameProcessor::FlyingObjectDetectionFrameProcessor(
    rclcpp::Publisher<barsik_interfaces::msg::FisheyeStatus>::SharedPtr publisher) :
    m_detector(0, "/home/sasa-r/src/barsik/fisheye_detector_quant_edgetpu.tflite")
  , m_publisher(publisher)
{
}

FlyingObjectDetectionFrameProcessor::~FlyingObjectDetectionFrameProcessor() {
}

void FlyingObjectDetectionFrameProcessor::SetFrameSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
}

void FlyingObjectDetectionFrameProcessor::ProcessFrame(uint8_t *address, uint32_t stride) {
    cv::Mat image(m_height, m_width, CV_8UC3, address, stride);
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_frame = image.clone();
    }
}

bool FlyingObjectDetectionFrameProcessor::HasFrame() const {
    return !m_frame.empty();
}

const cv::Mat FlyingObjectDetectionFrameProcessor::GetLatestFrame() {
    std::lock_guard<std::mutex> lock(m_mtx);
    if (!m_frame.empty()) {
        return m_frame;
    }
    return cv::Mat();
}

} // namespace camera_utility