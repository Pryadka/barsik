#include "TurretPositionController.h"

TurretPositionController::TurretPositionController() :
    rclcpp::Node{"turret_position_controller"}
{
    m_anglePublisher = create_publisher<barsik_interfaces::msg::SetAngleDiff>("angle_diff", 10);
}
    
TurretPositionController::~TurretPositionController() {
}

float TurretPositionController::GetHorizontalAngle() const {
    return 0.f;
}
    
float TurretPositionController::GetVerticalAngle() const {
    return 0.f;
}

void TurretPositionController::SetHorizontalAngleDiff(float angle) {
    barsik_interfaces::msg::SetAngleDiff msg;
    msg.horizontal_diff = angle;
    msg.vertical_diff = 0.f;
    m_anglePublisher->publish(msg);
}
    
void TurretPositionController::SetVerticalAngleDiff(float angle) {
    barsik_interfaces::msg::SetAngleDiff msg;
    msg.horizontal_diff = 0.f;
    msg.vertical_diff = angle;
    m_anglePublisher->publish(msg);
}

void TurretPositionController::MoveToBasePosition() {

}
