#ifndef __GUN_CONTROLLER__
#define __GUN_CONTROLLER__

#include "gpio-controller.h"

namespace turret
{

class GunController
{
public:
    GunController(gpio::GPIO_Controller &gpio);
    ~GunController();

    void Shut(int n = 0);

    void Stop();

private:
    gpio::GPIO_Controller &m_gpio;
};

} // namespace turret

#endif // __GUN_CONTROLLER__