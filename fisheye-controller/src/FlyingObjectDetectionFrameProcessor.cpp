#include "FlyingObjectDetectionFrameProcessor.h"
#include <iostream>

namespace camera_utility
{
    
FlyingObjectDetectionFrameProcessor::FlyingObjectDetectionFrameProcessor(int width, int height,
    rclcpp::Publisher<barsik_interfaces::msg::FisheyeStatus>::SharedPtr publisher) :
    m_width(width)
  , m_height(height)
  , m_detector(0, "/home/sasa-r/src/barsik/fisheye_detector_quant_edgetpu.tflite")
  , m_publisher(publisher)
  , m_waitForFrameSemaphore(0)
{
    m_mask = cv::Mat::zeros(cv::Size(m_width, m_height), CV_8UC3);
    cv::circle(m_mask, cv::Point(m_width/2, m_height/2), 590, cv::Scalar(255,255,255), -1, 8, 0);

    m_detectionThread = std::thread([this](){
        while(true) {
            m_waitForFrameSemaphore.acquire();
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                const auto res = m_detector.ExecuteInference(m_frame.data);
                if (res.first != -1) {
                    barsik_interfaces::msg::FisheyeStatus msg;
                    msg.set__detection(static_cast<uint8_t>(res.first));
                    msg.set__probability(res.second);
                    static int n = 0;
                    if (++n == 10) {
                        m_publisher->publish(msg);
                        n = 0;
                    }
                }
            }
        }
    });
}

FlyingObjectDetectionFrameProcessor::~FlyingObjectDetectionFrameProcessor() {
    m_run = false;
    m_detectionThread.join();
}

void FlyingObjectDetectionFrameProcessor::SetFrameSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
}

void FlyingObjectDetectionFrameProcessor::ProcessFrame(uint8_t *address, uint32_t stride) {
    cv::Mat image(m_height, m_width, CV_8UC3, address, stride);
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        image.copyTo(m_frame, m_mask);
    }
    m_waitForFrameSemaphore.release();
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