#include "PreciseObjectDetectionNode.h"

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/srv/get_angle.hpp"

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
  
    auto object_detection_service = std::make_shared<PreciseObjectDetectionNode>();

    rclcpp::spin(object_detection_service);

    rclcpp::shutdown();

    return 0;
}