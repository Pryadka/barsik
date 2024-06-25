#include "gun-controller.h"

namespace turret {

GunController::GunController(gpio::GPIO_Controller &gpio) :
    m_gpio{gpio}
{}
    
GunController::~GunController() {

}

void GunController::Shut(int /*n*/) {
    m_gpio.Set(15, gpio::GPIO_Controller::LEVEL::HIGH);
}

void GunController::Stop() {
    m_gpio.Set(15, gpio::GPIO_Controller::LEVEL::LOW);
}

} //turret