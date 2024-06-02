#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/srv/get_angle.hpp"

using namespace std::chrono_literals;

class GetAngleClient
{
public:
    GetAngleClient()
    {
        m_node = rclcpp::Node::make_shared("angle_subscriber");
        m_angleClient = m_node->create_client<barsik_interfaces::srv::GetAngle>("get_angle");
        while (!m_angleClient->wait_for_service(1s)) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(m_node->get_logger(), "Interrupted while waiting for the service. Exiting.");
            }
            RCLCPP_INFO(m_node->get_logger(), "service not available, waiting again...");
        }
        RCLCPP_INFO(m_node->get_logger(), "connection done");
    }

    void PrintAngle() {
        auto request = std::make_shared<barsik_interfaces::srv::GetAngle::Request>();
        auto future = m_angleClient->async_send_request(request);
        
        RCLCPP_INFO(m_node->get_logger(), "sent request is done. wait for response");
        
        if (rclcpp::spin_until_future_complete(m_node, future) == rclcpp::FutureReturnCode::SUCCESS) {
            if (future.valid()) {
                auto resp = future.get();
                float hor = resp->horizontal;
                float ver = resp->vertical;
                RCLCPP_INFO(m_node->get_logger(), "Angle hor=%f, vert=%f", hor, ver);
            }
        } else {
            RCLCPP_INFO(m_node->get_logger(), "Failed to get angle");
        }
        RCLCPP_INFO(m_node->get_logger(), "Done get angle");
    }
    
private:
    std::shared_ptr<rclcpp::Node> m_node;
    rclcpp::Client<barsik_interfaces::srv::GetAngle>::SharedPtr m_angleClient;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
  
    auto get_angle_client = std::make_shared<GetAngleClient>();

    get_angle_client->PrintAngle();

    rclcpp::shutdown();

    return 0;
}