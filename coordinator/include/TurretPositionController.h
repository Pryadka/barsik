#ifndef __TURRET_POSITION_CONTROLLER__
#define __TURRET_POSITION_CONTROLLER__

#include "rclcpp/rclcpp.hpp"
#include "barsik_interfaces/msg/set_angle_diff.hpp"

class TurretPositionController: public rclcpp::Node {
public:
    TurretPositionController();
    ~TurretPositionController();

    float GetHorizontalAngle() const;
    float GetVerticalAngle() const;

    void SetHorizontalAngleDiff(float angle);
    void SetVerticalAngleDiff(float angle);

    void MoveToBasePosition();
private:
    rclcpp::Publisher<barsik_interfaces::msg::SetAngleDiff>::SharedPtr m_anglePublisher;
};

#endif // __TURRET_POSITION_CONTROLLER__