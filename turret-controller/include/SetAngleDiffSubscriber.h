#ifndef __SET_ANGLE_DIFF_SUBSCRIBER__
#define __SET_ANGLE_DIFF_SUBSCRIBER__

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/set_angle_diff.hpp"

class SetAngleDiffSubscriber : public rclcpp::Node
{
public:
    SetAngleDiffSubscriber(turret::OrbitalController &orbi) :
        rclcpp::Node{"set_angle_sibscriber"}
      , m_orbi{orbi}
    {
        auto angle_diff_callback =
            [this](barsik_interfaces::msg::SetAngleDiff::UniquePtr msg) -> void {
                RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Angle diff: horizontal->%f, vertical->%f", msg->horizontal_diff, msg->vertical_diff);
                m_orbi.SetAngelsMissmatch(msg->horizontal_diff, msg->vertical_diff);
            };
            
        m_angle_diffSubscription =
            create_subscription<barsik_interfaces::msg::SetAngleDiff>("angle_diff", 10, angle_diff_callback);
    }

private:
    rclcpp::Subscription<barsik_interfaces::msg::SetAngleDiff>::SharedPtr m_angle_diffSubscription;
    turret::OrbitalController &m_orbi;
};

#endif // __SET_ANGLE_DIFF_SUBSCRIBER__