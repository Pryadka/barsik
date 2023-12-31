#include "mpu6050-controller.h"
#include "gpio-controller.h"

namespace imu
{

MPU_6050_Controller::MPU_6050_Controller(unsigned int address) {
    if (gpio::GPIO_Controller::Acquire()) {
        m_i2c_handle = i2cOpen(1, address, 0);
    }
}

MPU_6050_Controller::~MPU_6050_Controller() {
    if (m_i2c_handle >= 0) {
        i2cClose(m_i2c_handle);
    }
    gpio::GPIO_Controller::Release();
}

} // namespace imu