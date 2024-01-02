#ifndef __ORBITAL_CONTROLLER__
#define __ORBITAL_CONTROLLER__

#include "step-motor-controller.h"
#include "mpu6050-controller.h"

namespace turret
{

class OrbitalController
{
public:
    OrbitalController(/* args */);
    ~OrbitalController();

    void SetAngelsMissmatch(float horizontal, float vertical);
    void GoToVerticalPosition();

private:
    StepMotorController<19, 26, 5> m_horizontalMotor;
    StepMotorController<5, 6, 5> m_verticalMotor;
    imu::MPU_6050_Controller m_imu;
};

} // namespace turret

#endif // __ORBITAL_CONTROLLER__