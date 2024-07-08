#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/fire.hpp"
#include "barsik_interfaces/srv/get_angle.hpp"
#include "barsik_interfaces/msg/set_angle_diff.hpp"
#include "barsik_interfaces/msg/activate_data_provider.hpp"

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

class DataProviderStatusPublisher : public rclcpp::Node
{
public:
    DataProviderStatusPublisher() :
        rclcpp::Node{"data_provider_status_publisher"}
    {
        m_statusPublisher = create_publisher<barsik_interfaces::msg::ActivateDataProvider>("video_provider_activation", 10);
    }

    void SetActiveStatus(bool fisheye_active, bool highresolution_active) {
        barsik_interfaces::msg::ActivateDataProvider message;
        message.high_resolution_camera_active = highresolution_active;
        message.fish_eye_camera_active = fisheye_active;
        m_statusPublisher->publish(message);
        RCLCPP_INFO(this->get_logger(), "Set cameras fish=%d, high=%d", fisheye_active, highresolution_active);
    }

    
private:
    rclcpp::Publisher<barsik_interfaces::msg::ActivateDataProvider>::SharedPtr m_statusPublisher;
};


int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    DataProviderStatusPublisher statusSetter;

    bool a = true;
    while(true) {
        std::this_thread::sleep_for(200ms);
        statusSetter.SetActiveStatus(a, false);
        a = !a;
    }

    // gpio::GPIO_Controller gpio_controller;

    // turret::OrbitalController orbi{gpio_controller};
    // turret::GunController gun{gpio_controller};

    // orbi.GoToBasePosition();

    // auto gun_node = std::make_shared<FireSubscriber>(gun);
    // auto get_angle_node = std::make_shared<GetAngleService>(orbi);
    // auto set_angle_diff_node = std::make_shared<SetAngleDiffSubscriber>(orbi);

    // rclcpp::executors::SingleThreadedExecutor executor;
    // executor.add_node(gun_node);
    // executor.add_node(get_angle_node);
    // executor.add_node(set_angle_diff_node);
    // executor.spin();
  
    return 0;
}
