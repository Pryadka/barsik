#include "rclcpp/rclcpp.hpp"
#include "CameraController.h"
#include "FlyingObjectDetectionFrameProcessor.h"
#include "barsik_interfaces/msg/activate_data_provider.hpp"


class FlyingObjectDetectionNode : public rclcpp::Node
{
public:
    FlyingObjectDetectionNode();

private:
    camera_utility::CameraController m_cameraController;
    rclcpp::Subscription<barsik_interfaces::msg::ActivateDataProvider>::SharedPtr m_activation_subscription;
};