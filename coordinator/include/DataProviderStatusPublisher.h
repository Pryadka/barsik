#ifndef __DATA_PROVIDER_STATUS_PUBLISHER__
#define __DATA_PROVIDER_STATUS_PUBLISHER__

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/activate_data_provider.hpp"


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

#endif // __DATA_PROVIDER_STATUS_PUBLISHER__