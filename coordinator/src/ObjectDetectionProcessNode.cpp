#include "ObjectDetectionProcessNode.h"

ObjectDetectionProcessNode::ObjectDetectionProcessNode(Coordinator &coordinator) :
    rclcpp::Node{"detection_processor"}
  , m_coordinator{coordinator}
{
    auto process_message_callback =
        [this](barsik_interfaces::msg::PreciseDetections::UniquePtr msg) -> void {
            std::vector<Coordinator::PreciseDetection> detections;
            for (const auto &d:msg->objects) {
                RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Detected object of class=%d", d.object_class);
                Coordinator::PreciseDetection detection;
                detection.x = d.center_x;
                detection.y = d.center_y;
                detection.size = std::max(d.width, d.height);
                detection.focusFactor = d.scale_type;
                detections.emplace_back(detection);
            }
            m_coordinator.NewPreciseDetection(detections);
        };

    m_message_subscription =
        create_subscription<barsik_interfaces::msg::PreciseDetections>("object_detection", 10, process_message_callback);
}

ObjectDetectionProcessNode::~ObjectDetectionProcessNode() {

}
