#include "gpio-controller.h"
#include <signal.h>

namespace gpio
{

int GPIO_Controller::s_counter = 0;
bool GPIO_Controller::s_running = false;

void GPIO_Controller::Stop(int signum)
{
    gpioTerminate();
    s_running = false;
}

bool GPIO_Controller::Acquire()
{
    if (s_running) {
        s_counter++;
        return true;
    }

    if (gpioInitialise() < 0) {
        std::cout << "Initialyse error" << std::endl;
        return false;
    }

    s_running = true;
    s_counter = 1;
    return 0 == gpioSetSignalFunc(SIGINT, GPIO_Controller::Stop) &&
           0 == gpioSetSignalFunc(SIGSEGV, GPIO_Controller::Stop);
}

void GPIO_Controller::Release()
{
    if (--s_counter <= 0)
    {
        Stop(0);
    }
}

} // namespace gpio
