#include "rclcpp/rclcpp.hpp"
#include "CameraController.h"
#include "FlyingObjectDetectionFrameProcessor.h"
#include "barsik_interfaces/msg/activate_data_provider.hpp"

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

    fs::path m_dataPath;

    size_t m_imageNumber = 1;

    void SaveImage(const cv::Mat &image);
};