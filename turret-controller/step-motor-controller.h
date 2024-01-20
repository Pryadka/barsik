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
    StepMotorController() :
      m_blocked{true},
      m_run{false},
      m_pos{0},
      m_goInPositiveDirection{true} 
    {
        if (gpio::GPIO_Controller::Acquire())
        {
            if (gpioSetMode(DIR_PIN, PI_OUTPUT) || gpioSetMode(STEP_PIN, PI_OUTPUT))
            {
                std::cout << "set mode error" << std::endl;
                return;
            }

            m_run = true;
            gpioWrite(DIR_PIN, 1);

            m_controlThread = std::thread([this]()
            {
                while(true)
                {
                    while (m_blocked)
                    {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        m_cv.wait(lock);
                    }

                    if (!m_run)
                    {
                        return;
                    }

                    auto diff = m_delayInMks.load() - m_currentDelayInMks;
                    std::clamp(diff, -k_delayDecreseInMks, k_delayDecreseInMks);
                    m_currentDelayInMks += diff;

                    if (m_currentDelayInMks < 0 && m_goInPositiveDirection)
                    {
                        m_goInPositiveDirection = false;
                        gpioWrite(DIR_PIN, 0);
                    }
                    else if (m_currentDelayInMks > 0 && !m_goInPositiveDirection)
                    {
                        m_goInPositiveDirection = true;
                        gpioWrite(DIR_PIN, 1);
                    }

                    std::chrono::microseconds delay(std::abs(m_currentDelayInMks));
                    gpioWrite(STEP_PIN, PI_HIGH);
                    std::this_thread::sleep_for(delay);
                    gpioWrite(STEP_PIN, PI_LOW);
                    std::this_thread::sleep_for(delay);
                    m_pos += m_goInPositiveDirection ? 1 : -1;
                }
            });
        }
    }

    ~StepMotorController() {
        if (m_run)
        {
            m_run = false;
            if (m_blocked) Start();
            if (m_controlThread.joinable())
            {
                m_controlThread.join();
            }
        }
        gpio::GPIO_Controller::Release();
    }

    void Start() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_blocked = false;
        m_cv.notify_one();
    }

    void Stop() {
        m_blocked = true;
    }

    void SetSpeed(float speedGradPerSec) {
        speedGradPerSec *= REDUCTION;
        std::clamp(speedGradPerSec, -k_maxSpeed, k_maxSpeed);

        // half of the period in mks
        double delay = ((1000000. * 360.) / (2 * IMP_PER_ROTATION)) / speedGradPerSec;
        m_delayInMks.store(static_cast<int32_t>(delay));
    }

    float GetAngleInGrad() const {
        return (360. * (m_pos % (IMP_PER_ROTATION * REDUCTION))) / (IMP_PER_ROTATION * REDUCTION);
    }

private:
    std::thread m_controlThread;
    std::atomic<int32_t> m_delayInMks;
    int32_t m_currentDelayInMks = 0;
    std::condition_variable m_cv;
    std::mutex m_mutex;
    bool m_blocked;
    bool m_run;
    
    int m_pos;
    bool m_goInPositiveDirection = true;

    static constexpr float k_maxSpeed = 1000.;
    static constexpr int32_t k_delayDecreseInMks = 1;  // should be adjusted and may be templete argument
    static constexpr float k_StartSpeedGradPerSec = 2.; // should be adjusted for real device
    static constexpr uint32_t k_initialDelay = ((1000000. * 360.) / (2 * IMP_PER_ROTATION)) / k_StartSpeedGradPerSec;
};

} // namespace step