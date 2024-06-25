#ifndef __ORBITAL_CONTROLLER__
#define __ORBITAL_CONTROLLER__

#include "Eigen/Dense"
#include "step-motor-controller.h"
#include "mpu6050-controller.h"

#include <thread>
#include <mutex>

namespace turret
{

class OrbitalController
{
public:
    OrbitalController(gpio::GPIO_Controller &gpio);
    ~OrbitalController();

    Eigen::Vector2f GetPosition() const;
    void SetPosition(const Eigen::Vector2f &pos);

    void SetAngelsMissmatch(float horizontal, float vertical);
    void GoToBasePosition();

    void Calibrate();

private:
    StepMotorController<5, 6, 5> m_horizontalMotor;
    StepMotorController<19, 26, 3> m_verticalMotor;
    imu::MPU_6050_Controller m_imu;

    Eigen::Vector2f m_position;

    static float ConstrainAngle(float angle);
};

} // namespace turret

#endif // __ORBITAL_CONTROLLER__