#include "FlyingObjectDetectionNode.h"
#include "barsik_interfaces/msg/ActivateDataProvider.hpp"

FlyingObjectDetectionNode::FlyingObjectDetectionNode() :
  m_cameraController(libcamera::formats::RGB888, 640, 480, rclcpp::get_logger("rclcpp")) {
    auto activation_callback =
        [this](barsik_interfaces::msg::ActivateDataProvider::UniquePtr msg) -> void {
            RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Fish eye camera active", msg->fish_eye_camera_active);
            m_cameraController.SetActive(msg->fish_eye_camera_active);
        };
            
    m_activation_subscription =
        create_subscription<barsik_interfaces::msg::ActivateDataProvider>("video_provider_activation", 10, activation_callback);
}