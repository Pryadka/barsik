#include "gpio-controller.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <algorithm>

#include <iostream>

namespace turret {

template <int DIR_PIN, int STEP_PIN, unsigned int REDUCTION = 1, unsigned int IMP_PER_ROTATION = 400>
class StepMotorController
{
public:
    StepMotorController(gpio::GPIO_Controller &gpio_controller) :
      m_gpio_controller{gpio_controller},
      m_run{false},
      m_pos{0},
      m_currentPos{0}
    {
        m_gpio_controller.SetMode({
            {DIR_PIN, gpio::GPIO_Controller::IOMODE::OUT},
            {STEP_PIN, gpio::GPIO_Controller::IOMODE::OUT}}
        );
        m_run = true;
        m_gpio_controller.Set(DIR_PIN, gpio::GPIO_Controller::LEVEL::HIGH);

        SetSpeed(k_maxSpeed);

        m_controlThread = std::thread([this]() {
            while(true) {
                if (!m_run) {
                    return;
                }

                std::chrono::microseconds delay(std::abs(m_delayInMks));
                auto diff = m_pos - m_currentPos;
                if (diff == 0) {
                    std::this_thread::sleep_for(delay);
                    continue;
                }
                bool goInPositiveDirection = diff > 0;

                if (goInPositiveDirection) {
                    m_gpio_controller.Set(DIR_PIN, gpio::GPIO_Controller::LEVEL::HIGH);
                } else {
                    m_gpio_controller.Set(DIR_PIN, gpio::GPIO_Controller::LEVEL::LOW);
                }

                m_gpio_controller.Set(STEP_PIN, gpio::GPIO_Controller::LEVEL::HIGH);
                std::this_thread::sleep_for(delay);
                m_gpio_controller.Set(STEP_PIN, gpio::GPIO_Controller::LEVEL::LOW);
                std::this_thread::sleep_for(delay);
                m_currentPos += goInPositiveDirection ? 1 : -1;
            }
        });
    }

    ~StepMotorController() {
        if (m_run) {
            m_run = false;
            if (m_controlThread.joinable())
            {
                m_controlThread.join();
            }
        }
    }

    void SetSpeed(float speedGradPerSec = k_maxSpeed) {
        speedGradPerSec = std::clamp(speedGradPerSec * REDUCTION, -k_maxSpeed, k_maxSpeed);

        // half of the period in mks
        double delay = ((1000000. * 360.) / (2 * IMP_PER_ROTATION)) / speedGradPerSec;
        m_delayInMks.store(static_cast<int32_t>(delay));
    }

    float GetAngleInGrad() const {
        return (360. * m_currentPos) / (IMP_PER_ROTATION * REDUCTION);
    }

    void SetAngleInGrad(float angle = 0.f) {
        m_pos = static_cast<int>((angle * (IMP_PER_ROTATION * REDUCTION)) / 360.f);
    }

    void SetZeroPosition() {
        m_pos = m_currentPos = 0;
    }

private:
    gpio::GPIO_Controller &m_gpio_controller;
    std::thread m_controlThread;
    std::atomic<int32_t> m_delayInMks;
    bool m_run;
    
    int m_pos;
    int m_currentPos;

    static constexpr float k_maxSpeed = 1000.;
};

} // namespace step