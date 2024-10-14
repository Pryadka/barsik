#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/fire.hpp"
#include "barsik_interfaces/srv/get_angle.hpp"
#include "barsik_interfaces/msg/set_angle_diff.hpp"
#include "barsik_interfaces/msg/debug_save_current_image.hpp"

#include "ObjectDetectionProcessNode.h"
#include "FlyingObjectProcessNode.h"
#include "DataProviderStatusPublisher.h"

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

class DebugImageRecordPublisher : public rclcpp::Node
{
public:
    DebugImageRecordPublisher() :
        rclcpp::Node{"data_provider_status_publisher"}
    {
        m_publisher = create_publisher<barsik_interfaces::msg::DebugSaveCurrentImage>("video_save_frame", 10);
    }

    void SaveImage(bool fisheye, bool highresolution) {
        barsik_interfaces::msg::DebugSaveCurrentImage message;
        message.precise = highresolution;
        message.fisheye = fisheye;
        RCLCPP_INFO(this->get_logger(), "Save image");
        m_publisher->publish(message);
    }

    
private:
    rclcpp::Publisher<barsik_interfaces::msg::DebugSaveCurrentImage>::SharedPtr m_publisher;
};

/////////////////////////////////////

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    DataProviderStatusPublisher statusSetter;
    statusSetter.SetActiveStatus(true, false);

    auto fisheye_detector_reciever = std::make_shared<FlyingObjectProcessNode>();
    auto precise_detector_reciever = std::make_shared<ObjectDetectionProcessNode>();

    std::thread t{
        []() {
            DebugImageRecordPublisher publisher;
            while(true) {
                std::this_thread::sleep_for(1000ms);
                publisher.SaveImage(false, false);
            }
        }
    };

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(fisheye_detector_reciever);
    executor.add_node(precise_detector_reciever);
    // executor.add_node(set_angle_diff_node);
    executor.spin();
  
    return 0;
}
