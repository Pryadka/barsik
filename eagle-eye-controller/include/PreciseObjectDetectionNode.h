#include "rclcpp/rclcpp.hpp"
#include "PreciseCameraController.h"
#include "PreciseObjectDetectionFrameProcessor.h"
#include "barsik_interfaces/msg/activate_data_provider.hpp"
#include "barsik_interfaces/msg/debug_save_current_image.hpp"

#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

class PreciseObjectDetectionNode : public rclcpp::Node
{
public:
    PreciseObjectDetectionNode();
    ~PreciseObjectDetectionNode();

private:
    camera_utility::PreciseCameraController m_cameraController;
    std::shared_ptr<camera_utility::PreciseObjectDetectionFrameProcessor> m_frameProcessor;
    rclcpp::Subscription<barsik_interfaces::msg::ActivateDataProvider>::SharedPtr m_activation_subscription;
    rclcpp::Subscription<barsik_interfaces::msg::DebugSaveCurrentImage>::SharedPtr m_save_current_frame_subscription;
    rclcpp::Publisher<barsik_interfaces::msg::PreciseDetections>::SharedPtr m_objectDetectionPublisher;

    fs::path m_dataPath;

    size_t m_imageNumber = 1;

    void SaveImage(const cv::Mat &image);
};