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

    m_verticalMotor.SetSpeed(50.f);
    auto angle = m_verticalMotor.GetAngleInGrad();
    auto accelData = m_imu.GetAccelData();
    std::cout <<  "c=" << accelData[2] << std::endl;
    auto g1 = accelData[2];
    m_verticalMotor.SetAngleInGrad(angle + 10.);
    std::this_thread::sleep_for(100ms);
    accelData = m_imu.GetAccelData();
    auto g2 = accelData[2];
    auto angleDiff = std::acos(g2) * 180.0 / PI;
    if (g2 < g1) {
        angleDiff = -angleDiff;
    }

    std::cout << "AngleDiff=" << angleDiff << std::endl;

    m_verticalMotor.SetAngleInGrad(m_verticalMotor.GetAngleInGrad() + angleDiff);

    std::this_thread::sleep_for(5s);

    m_verticalMotor.SetZeroPosition();
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