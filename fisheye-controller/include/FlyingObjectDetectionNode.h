#ifndef __FLYING_OBJECT_DETECTION_NODE__
#define __FLYING_OBJECT_DETECTION_NODE__

#include "rclcpp/rclcpp.hpp"
#include "CameraController.h"
#include "FlyingObjectDetectionFrameProcessor.h"
#include "barsik_interfaces/msg/activate_data_provider.hpp"
#include "barsik_interfaces/msg/debug_save_current_image.hpp"
#include "barsik_interfaces/msg/fisheye_status.hpp"

#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

class FlyingObjectDetectionNode : public rclcpp::Node
{
public:
    FlyingObjectDetectionNode();
    ~FlyingObjectDetectionNode();

private:
    camera_utility::CameraController m_cameraController;
    std::shared_ptr<camera_utility::FlyingObjectDetectionFrameProcessor> m_frameProcessor;
    rclcpp::Subscription<barsik_interfaces::msg::ActivateDataProvider>::SharedPtr m_activation_subscription;
    rclcpp::Subscription<barsik_interfaces::msg::DebugSaveCurrentImage>::SharedPtr m_save_current_frame_subscription;
    rclcpp::Publisher<barsik_interfaces::msg::FisheyeStatus>::SharedPtr m_fisheyeStatusPublisher;

    fs::path m_dataPath;

    size_t m_imageNumber = 1;
    bool m_saveFrames = false;

    void SaveImage(const cv::Mat &image);
};

#endif //__FLYING_OBJECT_DETECTION_NODE__