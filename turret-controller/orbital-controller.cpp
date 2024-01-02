#include "orbital-controller.h"

namespace turret
{

OrbitalController::OrbitalController(/* args */)
{
}

OrbitalController::~OrbitalController()
{
}

void OrbitalController::SetAngelsMissmatch(float horizontal, float vertical)
{
    m_horizontalMotor.SetSpeed(200.);
    m_verticalMotor.SetSpeed(100.);
    m_horizontalMotor.Start();
    m_verticalMotor.Start();
}

void OrbitalController::GoToVerticalPosition()
{
}

} // namespace turret