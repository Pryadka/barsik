#ifndef __FIRE_SUBSCRIBER__
#define __FIRE_SUBSCRIBER__

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/fire.hpp"
#include <chrono>


using namespace std::chrono_literals;

class FireSubscriber : public rclcpp::Node
{
public:
    FireSubscriber(turret::GunController &gun) :
        rclcpp::Node{"fire_subscriber"}
      , m_gun{gun}
    {
        auto gun_callback =
            [this](barsik_interfaces::msg::Fire::UniquePtr msg) -> void {
                RCLCPP_INFO(this->get_logger(), "Gun number: '%d'", msg->gun_number);
                m_gun.Shut(msg->gun_number);
                std::this_thread::sleep_for(1s);
                m_gun.Stop();
            };
            
        m_fireSubscription =
            create_subscription<barsik_interfaces::msg::Fire>("gun", 10, gun_callback);
    }
private:
    rclcpp::Subscription<barsik_interfaces::msg::Fire>::SharedPtr m_fireSubscription;
    turret::GunController &m_gun;
};

#endif