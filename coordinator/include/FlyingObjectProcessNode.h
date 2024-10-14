#ifndef __FLYING_OBJECT_PROCESS_NODE__
#define __FLYING_OBJECT_PROCESS_NODE__

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/fisheye_status.hpp"


class FlyingObjectProcessNode : public rclcpp::Node
{
public:
    FlyingObjectProcessNode();
    ~FlyingObjectProcessNode();

private:
    rclcpp::Subscription<barsik_interfaces::msg::FisheyeStatus>::SharedPtr m_message_subscription;
};

#endif // __FLYING_OBJECT_PROCESS_NODE__