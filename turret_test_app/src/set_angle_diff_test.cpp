#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/set_angle_diff.hpp"

using namespace std::chrono_literals;

class SetAngleDiffPublisher : public rclcpp::Node
{
public:
    SetAngleDiffPublisher() :
        rclcpp::Node{"angle_subscriber"}
    {
        m_anglePublisher = create_publisher<barsik_interfaces::msg::SetAngleDiff>("angle_diff", 10);
        while(true) {
            float h_diff, v_diff;
            std::cout << "Enter angles\n  Horizontal>";
            std::cin >> h_diff;
            std::cout << "  Vertical>";
            std::cin >> v_diff;
            if (h_diff == 0. && v_diff == 0.) break;
            SetAngle(h_diff, v_diff);
        }
    }
    
private:
    rclcpp::Publisher<barsik_interfaces::msg::SetAngleDiff>::SharedPtr m_anglePublisher;
    void SetAngle(float horizontal, float vertical) {
        barsik_interfaces::msg::SetAngleDiff message;
        message.horizontal_diff = horizontal;
        message.vertical_diff = vertical;
        RCLCPP_INFO(this->get_logger(), "Set angle diff hor=%f, vert=%f", message.horizontal_diff, message.vertical_diff);
        m_anglePublisher->publish(message);
    }
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SetAngleDiffPublisher>());
  rclcpp::shutdown();
  return 0;
}