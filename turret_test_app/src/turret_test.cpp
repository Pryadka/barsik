#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/fire.hpp"

using namespace std::chrono_literals;

class FirelPublisher : public rclcpp::Node
{
public:
  FirelPublisher()
  : Node("fire_publisher"), count_(0)
  {
    publisher_ = this->create_publisher<barsik_interfaces::msg::Fire>("topic", 10);
    auto timer_callback =
      [this]() -> void {
        auto message = barsik_interfaces::msg::Fire{};
        message.gun_number = count_++;
        RCLCPP_INFO(this->get_logger(), "Publishing: '%d'", message.gun_number);
        this->publisher_->publish(message);
      };
    timer_ = this->create_wall_timer(500ms, timer_callback);
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<barsik_interfaces::msg::Fire>::SharedPtr publisher_;
  size_t count_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<FirelPublisher>());
  rclcpp::shutdown();
  return 0;
}