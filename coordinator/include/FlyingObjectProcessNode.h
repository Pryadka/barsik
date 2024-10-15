#ifndef __FLYING_OBJECT_PROCESS_NODE__
#define __FLYING_OBJECT_PROCESS_NODE__

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/fisheye_status.hpp"
#include "Coordinator.h"


class FlyingObjectProcessNode : public rclcpp::Node
{
public:
    FlyingObjectProcessNode(Coordinator &coordinator);
    ~FlyingObjectProcessNode();

private:
    rclcpp::Subscription<barsik_interfaces::msg::FisheyeStatus>::SharedPtr m_message_subscription;
    Coordinator &m_coordinator;
};

#endif // __FLYING_OBJECT_PROCESS_NODE__