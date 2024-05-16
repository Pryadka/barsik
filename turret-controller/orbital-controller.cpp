#include "orbital-controller.h"
#include <math.h>  

namespace turret
{

OrbitalController::OrbitalController(gpio::GPIO_Controller &gpio):
    m_horizontalMotor(gpio),
    m_verticalMotor(gpio),
    m_position{0.f, 0.f}
{
    if (!m_imu.UploadOffsetsFromFile("mpu6050.data")) {
        std::cout << "Failed to upload imu data" << std::endl;
    }

    // skip first measurement because they are wrong
    for (int i = 0; i < 10; i++) {
        m_imu.GetAccelData();
    }
}

OrbitalController::~OrbitalController() {
}

Eigen::Vector2f OrbitalController::GetPosition() const {
    return Eigen::Vector2f{m_horizontalMotor.GetAngleInGrad(), m_verticalMotor.GetAngleInGrad()};
}

void OrbitalController::SetPosition(const Eigen::Vector2f &pos) {
    m_position = pos;

    m_horizontalMotor.SetAngleInGrad(m_position[0]);
    m_verticalMotor.SetAngleInGrad(m_position[1]);
}

void OrbitalController::SetAngelsMissmatch(float horizontal, float vertical) {
    m_position[0] = ConstrainAngle(m_position[0] + horizontal);
    m_position[1] = ConstrainAngle(m_position[1] + vertical);

    m_horizontalMotor.SetAngleInGrad(m_position[0]);
    m_verticalMotor.SetAngleInGrad(m_position[1]);
}

void OrbitalController::GoToBasePosition() {
    using namespace std::chrono_literals;
    constexpr double PI = 3.14159265;

    m_verticalMotor.SetSpeed(20.f);
    auto angle = m_verticalMotor.GetAngleInGrad();
    constexpr int averageCycleNumber = 100;
    for (int i = 0; i < averageCycleNumber; i++) {
        auto accelData = m_imu.GetAccelData();
        //std::cout <<  "a=" << accelData[0] <<  ", b=" << accelData[1] <<  ", c=" << accelData[2] << std::endl;

        auto angleDiff = -std::asin(accelData[0]) * 180.0 / PI;

        std::cout << "AngleDiff=" << angleDiff << std::endl;
        auto absAngleDiff = std::abs(angleDiff);
        if (absAngleDiff < 0.5f) break;
        if (absAngleDiff < 10.f) m_verticalMotor.SetSpeed(absAngleDiff);

        m_verticalMotor.SetAngleInGrad(m_verticalMotor.GetAngleInGrad() + angleDiff);
        std::this_thread::sleep_for(100ms);
    }

    m_verticalMotor.SetZeroPosition();
    m_verticalMotor.SetSpeed();
}

void OrbitalController::Calibrate() {
    m_imu.CalibrateAndWriteOffsetsToFile("mpu6050.data");
}

float OrbitalController::ConstrainAngle(float angle)
{
    angle = fmod(angle + 180.f ,360.f);
    if (angle < 0.)
        angle += 360.f;
    return angle - 180.f;
}

} // namespace turret