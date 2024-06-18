#include "rclcpp/rclcpp.hpp"
#include "CameraController.h"
#include "FlyingObjectDetectionFrameProcessor.h"

class FlyingObjectDetectionNode : public rclcpp::Node
{
public:
    FlyingObjectDetectionNode();

private:
    camera_utility::CameraController m_cameraController;
};