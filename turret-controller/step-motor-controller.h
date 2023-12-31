#include "gpio-controller.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace turret {

template <int DIR_PIN, int STEP_PIN, unsigned int REDUCTION = 1, unsigned int IMP_PER_ROTATION = 400>
class StepMotorController
{
public:
    StepMotorController() :
      m_blocked{true},
      m_run{true},
      m_pos{0},
      m_goInPositiveDirection{true} 
    {
        if (gpio::GPIO_Controller::Acquire())
        {
            gpioSetMode(DIR_PIN, PI_OUTPUT);
            gpioSetMode(STEP_PIN, PI_OUTPUT);
            m_controlThread = std::thread([this]()
            {
                while (m_blocked)
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_cv.wait(lock);
                }

                if (!m_run) return;

                std::chrono::microseconds delay(m_delayInMks.load());
                gpioWrite(STEP_PIN, 1);
                std::this_thread::sleep_for(delay);
                gpioWrite(STEP_PIN, 0);
                std::this_thread::sleep_for(delay);
                m_pos += m_goInPositiveDirection ? 1 : -1;
            });
        }
    }

    ~StepMotorController() {
        m_run = false;
        if (m_blocked) Start();
        m_controlThread.join();
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
        if (speedGradPerSec < 0) {
            gpioWrite(DIR_PIN, 0);
            speedGradPerSec = -speedGradPerSec;
        } else {
            gpioWrite(DIR_PIN, 1);
        }

        speedGradPerSec *= REDUCTION;
        if (speedGradPerSec > k_maxSpeed) {
            speedGradPerSec = k_maxSpeed;
        }

        // half of the period
        double delay = ((1000000. * 360.) / speedGradPerSec) / (2 * IMP_PER_ROTATION);
        m_delayInMks.store(static_cast<uint32_t>(delay));
    }

    float GetAngleInGrad() const {
        return (360. * (m_pos % (IMP_PER_ROTATION * REDUCTION))) / (IMP_PER_ROTATION * REDUCTION);
    }

private:
    std::thread m_controlThread;
    std::atomic<uint32_t> m_delayInMks;
    std::condition_variable m_cv;
    std::mutex m_mutex;
    bool m_blocked;
    bool m_run;
    
    int m_pos;
    bool m_goInPositiveDirection;

    static constexpr float k_maxSpeed = 1000.;
};

} // namespace step