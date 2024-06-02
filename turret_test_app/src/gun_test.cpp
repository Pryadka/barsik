#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/fire.hpp"
#include "barsik_interfaces/srv/get_angle.hpp"
#include "barsik_interfaces/msg/set_angle_diff.hpp"

using namespace std::chrono_literals;

class FirePublisher : public rclcpp::Node
{
public:
    FirePublisher() :
        rclcpp::Node{"fire_subscriber"}
    {
        m_gunPublisher = create_publisher<barsik_interfaces::msg::Fire>("gun", 10);
        std::cout << "Enter n-gun>";
        int n;
        std::cin >> n;
        Fire(n);
    }
    
private:
    rclcpp::Publisher<barsik_interfaces::msg::Fire>::SharedPtr m_gunPublisher;
  
    void Fire(int n) {
        barsik_interfaces::msg::Fire message;
        message.gun_number = n;
        RCLCPP_INFO(this->get_logger(), "Set gun '%d'", message.gun_number);
        m_gunPublisher->publish(message);
    }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<FirePublisher>());
  rclcpp::shutdown();
  return 0;
}