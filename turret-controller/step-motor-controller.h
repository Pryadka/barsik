#include "gpio-controller.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <functional>

#include <iostream>

namespace turret {

template <int DIR_PIN, int STEP_PIN, unsigned int REDUCTION = 1, unsigned int IMP_PER_ROTATION = 400>
class StepMotorController
{
public:
    StepMotorController(std::function<void(float, float)> &&PID_callback = std::function<void(float, float)>{}) :
      m_blocked{true},
      m_run{false},
      m_pos{0},
      m_goInPositiveDirection{true},
      m_PID_callback{PID_callback}
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

                    std::chrono::microseconds delay;
                    float speed, accelaration;
                    {
                        std::unique_lock<std::mutex>(m_delayMutex);
                        auto delayDiff = m_delayInMks - m_currentDelayInMks;
                        std::clamp(delayDiff, -k_delayDecreseInMks, k_delayDecreseInMks);
                        m_currentDelayInMks += delayDiff;

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

                        delay = std::chrono::microseconds(std::abs(m_currentDelayInMks));
                        if (m_PID_callback) {
                            speed =  k_SpeedToDelayKoeficient / m_currentDelayInMks;
                            const float speedDiff = speed - k_SpeedToDelayKoeficient / (m_currentDelayInMks - delayDiff);
                            accelaration = speedDiff / (2 * m_currentDelayInMks);
                        }
                    }
                    gpioWrite(STEP_PIN, PI_HIGH);
                    std::this_thread::sleep_for(delay);
                    gpioWrite(STEP_PIN, PI_LOW);
                    std::this_thread::sleep_for(delay);
                    m_pos += m_goInPositiveDirection ? 1 : -1;
                    if (m_PID_callback) {
                        m_PID_callback(speed, accelaration);
                    }
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
        if (speedGradPerSec == 0.0f) {
            Stop();
            return;
        }
        speedGradPerSec *= REDUCTION;
        std::clamp(speedGradPerSec, -k_maxSpeed, k_maxSpeed);

        // half of the period in mks
        auto const delay = static_cast<int32_t>(((1000000. * 360.) / (2 * IMP_PER_ROTATION)) / speedGradPerSec);
        {
            std::unique_lock<std::mutex>(m_delayMutex);
            m_delayInMks = delay;
            if (m_blocked) {
                m_currentDelayInMks = (delay > 0) ? k_initialDelay : -k_initialDelay;
                Start();
            }
        }
    }

    float GetAngleInGrad() const {
        return (360. * (m_pos % (IMP_PER_ROTATION * REDUCTION))) / (IMP_PER_ROTATION * REDUCTION);
    }

private:
    std::thread m_controlThread;
    int32_t m_delayInMks;
    int32_t m_currentDelayInMks;
    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::mutex m_delayMutex;
    bool m_blocked;
    bool m_run;
    
    int m_pos;
    bool m_goInPositiveDirection = true;

    std::function<void(float, float)> m_PID_callback;

    static constexpr float k_maxSpeed = 1000.;
    static constexpr int32_t k_delayDecreseInMks = 10;  // should be adjusted and may be templete argument
    static constexpr float k_StartSpeedGradPerSec = 2.; // should be adjusted for real device
    static constexpr float k_SpeedToDelayKoeficient = (1000000. * 360.) / (2 * IMP_PER_ROTATION);
    static constexpr uint32_t k_initialDelay = k_SpeedToDelayKoeficient / k_StartSpeedGradPerSec;
};

} // namespace step