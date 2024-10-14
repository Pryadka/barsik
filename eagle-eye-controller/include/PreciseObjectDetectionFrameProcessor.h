#ifndef __OBJECT_DETECTION_FRAME_PROCESSOR__
#define __OBJECT_DETECTION_FRAME_PROCESSOR__

#include "FrameProcessingInterface.h"
#include <thread>
#include <opencv2/opencv.hpp>
#include "YOLOV8_EdgeTpu_Detector.h"
#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/precise_detections.hpp"

namespace camera_utility
{

class PreciseObjectDetectionFrameProcessor : public FrameProcessingInterface {
public:
    PreciseObjectDetectionFrameProcessor(rclcpp::Publisher<barsik_interfaces::msg::PreciseDetections>::SharedPtr publisher);
    ~PreciseObjectDetectionFrameProcessor() override;

    void ProcessFrame(uint8_t *address, uint32_t stride) override;
    void SetFrameSize(uint32_t width, uint32_t height) override {m_width = width; m_height = height;}

    bool HasFrame() const;
    const cv::Mat GetLatestFrame();

private:
    uint32_t m_width = 640;
    uint32_t m_height = 640;
    cv::Mat m_frame;

    barsik_cv::YOLOV8_EdgeTpu_Detector m_detector;
    rclcpp::Publisher<barsik_interfaces::msg::PreciseDetections>::SharedPtr m_publisher;

    std::thread m_detectionThread;
    bool m_run = true;
    mutable std::mutex m_mtx;
    mutable std::binary_semaphore m_waitForFrameSemaphore;
};

} // namespace camera_utility

#endif // __OBJECT_DETECTION_FRAME_PROCESSOR__