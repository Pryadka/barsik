#ifndef __OBJECT_DETECTION_FRAME_PROCESSOR__
#define __OBJECT_DETECTION_FRAME_PROCESSOR__

#include "FrameProcessingInterface.h"
#include <thread>
#include <semaphore>
#include <opencv2/opencv.hpp>
#include "Fisheye_EdgeTpu_Detector.h"
#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/fisheye_status.hpp"


namespace camera_utility
{

class FlyingObjectDetectionFrameProcessor : public FrameProcessingInterface {
public:
    FlyingObjectDetectionFrameProcessor(int width, int height, rclcpp::Publisher<barsik_interfaces::msg::FisheyeStatus>::SharedPtr publisher);
    ~FlyingObjectDetectionFrameProcessor() override;

    void ProcessFrame(uint8_t *address, uint32_t stride) override;

    bool HasFrame() const;
    const cv::Mat GetLatestFrame();

    void SetFrameSize(uint32_t width, uint32_t height) override;

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    cv::Mat m_frame;
    cv::Mat m_mask;
    
    barsik_cv::Fisheye_EdgeTpu_Detector m_detector;
    rclcpp::Publisher<barsik_interfaces::msg::FisheyeStatus>::SharedPtr m_publisher;

    std::thread m_detectionThread;
    mutable std::mutex m_mtx;
    mutable std::binary_semaphore m_waitForFrameSemaphore;
};

} // namespace camera_utility

#endif // __OBJECT_DETECTION_FRAME_PROCESSOR__