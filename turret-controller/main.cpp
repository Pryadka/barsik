#include <iostream>
#include "orbital-controller.h"
#include "gun-controller.h"
#include "mpu6050-controller.h"
#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/fire.hpp"
#include "barsik_interfaces/srv/get_angle.hpp"
#include "barsik_interfaces/msg/set_angle_diff.hpp"

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

class SetAngleDiffSubscriber : public rclcpp::Node
{
public:
    SetAngleDiffSubscriber(turret::OrbitalController &orbi) :
        rclcpp::Node{"set_angle_sibscriber"}
      , m_orbi{orbi}
    {
        auto angle_diff_callback =
            [this](barsik_interfaces::msg::SetAngleDiff::UniquePtr msg) -> void {
                RCLCPP_INFO(this->get_logger(), "Angle diff: horizontal->%f, vertical->%f", msg->horizontal_diff, msg->vertical_diff);
                m_orbi.SetAngelsMissmatch(msg->horizontal_diff, msg->vertical_diff);
            };
            
        m_angle_diffSubscription =
            create_subscription<barsik_interfaces::msg::SetAngleDiff>("angle_diff", 10, angle_diff_callback);
    }

private:
    rclcpp::Subscription<barsik_interfaces::msg::SetAngleDiff>::SharedPtr m_angle_diffSubscription;
    turret::OrbitalController &m_orbi;
};

class GetAngleService : public rclcpp::Node
{
public:
    GetAngleService(turret::OrbitalController &orbi) :
        rclcpp::Node{"get_angle"}
      , m_orbi{orbi}
    {
        auto get_angle_callback = 
            [this](const std::shared_ptr<barsik_interfaces::srv::GetAngle::Request> request,
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

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    gpio::GPIO_Controller gpio_controller;

    turret::OrbitalController orbi{gpio_controller};
    turret::GunController gun{gpio_controller};

    orbi.GoToBasePosition();

    auto gun_node = std::make_shared<FireSubscriber>(gun);
    auto get_angle_node = std::make_shared<GetAngleService>(orbi);
    auto set_angle_diff_node = std::make_shared<SetAngleDiffSubscriber>(orbi);

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(gun_node);
    executor.add_node(get_angle_node);
    executor.add_node(set_angle_diff_node);
    executor.spin();
  
    return 0;
}

// int main()
// {

//     gpio::GPIO_Controller gpio_controller;
//     turret::OrbitalController orbi(gpio_controller);
//     imu::MPU_6050_Controller imu_controller;

//     turret::GunController gun{gpio_controller};

//     //orbi.Calibrate(); //return 0;

//     orbi.GoToBasePosition(); 

//     std::this_thread::sleep_for(1s);
//     float h = 90., v = 40.;
//     for (int i = 0; i < 5; i++) {
//         orbi.SetPosition({h, v});
//         h *= -1;
//         v *= -1.;
//         std::this_thread::sleep_for(3s);
//         gun.Shut();
//         std::this_thread::sleep_for(1s);
//         gun.Stop();
//     }
//     orbi.SetPosition({0.f, 0.f});
//     std::this_thread::sleep_for(5s);
//     // for (int i = 0; i < 1000; i++) {
//     //     auto t = imu.GetAccelData();
//     //     std::cout<< std::endl << "T=" << t << std::endl;
//     // }

//     return 0;
// }