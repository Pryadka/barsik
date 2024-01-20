#include "orbital-controller.h"

namespace turret
{

OrbitalController::OrbitalController(/* args */)
{
    m_horizontalMotor.Start();
    m_verticalMotor.Start();
    m_printThread = std::thread([this] {
        using namespace std::chrono_literals;
        while(m_print) {
        std::cout << "Angle=" << m_verticalMotor.GetAngleInGrad() << std::endl;
        std::this_thread::sleep_for(200ms);
        }
    });
}

OrbitalController::~OrbitalController()
{
    m_print = false;
    m_printThread.join();
}

void OrbitalController::SetAngelsMissmatch(float horizontal, float vertical)
{
    m_horizontalMotor.SetSpeed(horizontal * 200.);
    m_verticalMotor.SetSpeed(vertical * 100.);
}

void OrbitalController::GoToVerticalPosition()
{
}

} // namespace turret