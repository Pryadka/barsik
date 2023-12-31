#ifndef __GPIO_CONTROLLER__
#define __GPIO_CONTROLLER__

#include <pigpio.h>

namespace gpio
{

class GPIO_Controller {
public:
    static bool Acquire();
    static void Release();
private:
    static int s_counter;
    static bool s_running;

    static void Stop(int signum);
};
    
} // namespace gpio



#endif // __GPIO_CONTROLLER__