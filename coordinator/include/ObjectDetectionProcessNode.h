#ifndef __OBJECT_DETECTION_PROCESS_NODE__
#define __OBJECT_DETECTION_PROCESS_NODE__

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/precise_detections.hpp"

class ObjectDetectionProcessNode : public rclcpp::Node
{
public:
    ObjectDetectionProcessNode();
    ~ObjectDetectionProcessNode();

private:
    rclcpp::Subscription<barsik_interfaces::msg::PreciseDetections>::SharedPtr m_message_subscription;
};

#endif // __OBJECT_DETECTION_PROCESS_NODE__