#ifndef __GPIO_CONTROLLER__
#define __GPIO_CONTROLLER__

#include <lgpio.h>
#include <vector>
#include <utility>

namespace gpio
{

class GPIO_Controller {
public:
    GPIO_Controller();
    ~GPIO_Controller();

    enum class IOMODE : int {
        IN = 0,
        OUT,
    };

    enum class LEVEL : int {
        LOW = 0,
        HIGH = 1,
    };

    void SetMode(const std::vector<std::pair<int, IOMODE>> &pins);
    void Set(int pin, LEVEL value);
    int Get(int pin);
private:
    int m_d;
    int m_flags = 0;
};
    
} // namespace gpio



#endif // __GPIO_CONTROLLER__