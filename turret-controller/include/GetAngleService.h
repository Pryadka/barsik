#ifndef __GET_ANGLE_SERVICE__
#define __GET_ANGLE_SERVICE__

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/srv/get_angle.hpp"

class GetAngleService : public rclcpp::Node
{
public:
    GetAngleService(turret::OrbitalController &orbi) :
        rclcpp::Node{"get_angle"}
      , m_orbi{orbi}
    {
        auto get_angle_callback = 
            [this](const std::shared_ptr<barsik_interfaces::srv::GetAngle::Request> /*request*/,
                         std::shared_ptr<barsik_interfaces::srv::GetAngle::Response> response) {
                auto position = m_orbi.GetPosition();
                response->horizontal = position[0];
                response->vertical = position[1];
                RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Get angle requested. H=%f, V=%f", 
                    response->horizontal, response->vertical);
            };

        m_getAngleService = create_service<barsik_interfaces::srv::GetAngle>("get_angle", get_angle_callback);
    }

private:
    rclcpp::Service<barsik_interfaces::srv::GetAngle>::SharedPtr m_getAngleService;
    turret::OrbitalController &m_orbi;
};

#endif