#include "ObjectDetectionProcessNode.h"

ObjectDetectionProcessNode::ObjectDetectionProcessNode(Coordinator &coordinator) :
    rclcpp::Node{"detection_processor"}
  , m_coordinator{coordinator}
{
    auto process_message_callback =
        [this](barsik_interfaces::msg::PreciseDetections::UniquePtr msg) -> void {
            for (const auto &d:msg->objects) {
                RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Detected object of class=%d", d.object_class);
            }
        };

    m_message_subscription =
        create_subscription<barsik_interfaces::msg::PreciseDetections>("object_detection", 10, process_message_callback);
}

ObjectDetectionProcessNode::~ObjectDetectionProcessNode() {

}
