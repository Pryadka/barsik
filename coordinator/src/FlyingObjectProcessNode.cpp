#include "FlyingObjectProcessNode.h"

FlyingObjectProcessNode::FlyingObjectProcessNode(Coordinator &coordinator) :
    rclcpp::Node{"flying_object_processor"}
  , m_coordinator{coordinator}
{
    auto process_message_callback =
        [this](barsik_interfaces::msg::FisheyeStatus::UniquePtr msg) -> void {
            if (msg->detection == 11) {
                RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Fish eye nothing");
            } else {
                RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Fish eye sector detected=%d, probability=%f", msg->detection, msg->probability);
            }
            m_coordinator.NewFisheyeDetection(msg->detection);
        };

    m_message_subscription =
        create_subscription<barsik_interfaces::msg::FisheyeStatus>("fisheye_status", 10, process_message_callback);
}

FlyingObjectProcessNode::~FlyingObjectProcessNode() {

}
