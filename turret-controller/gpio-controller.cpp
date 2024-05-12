#include "gpio-controller.h"
#include <signal.h>
#include <iostream>

namespace gpio
{

GPIO_Controller::GPIO_Controller() {
    m_d = lgGpiochipOpen(4);
}
    
GPIO_Controller::~GPIO_Controller() {
    lgGpiochipClose(m_d);
}

void GPIO_Controller::SetMode(const std::vector<std::pair<int, IOMODE>> &pins) {
    for (const auto & i : pins) {
        switch (i.second)
        {
        case GPIO_Controller::IOMODE::IN:
            lgGpioClaimInput(m_d, m_flags, i.first);
            break;

        case GPIO_Controller::IOMODE::OUT:
            lgGpioClaimOutput(m_d, m_flags, i.first, 0);
            break;
        
        default:
            break;
        }
    }
}

void GPIO_Controller::Set(int pin, GPIO_Controller::LEVEL value) {
    lgGpioWrite(m_d, pin, static_cast<int>(value));    
}

int GPIO_Controller::Get(int pin) {
    return lgGpioRead(m_d, pin);
}

} // namespace gpio
