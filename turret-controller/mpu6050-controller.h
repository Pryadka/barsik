#ifndef __MPU_6050__
#define __MPU_6050__

#include "Eigen/Dense"
#include "gpio-controller.h"

namespace imu
{

class MPU_6050_Controller {
public:
    MPU_6050_Controller(unsigned int address = 0x68);
    ~MPU_6050_Controller();

    Eigen::Vector3f GetAccelData();
    Eigen::Vector3f GetGyroData();
    float GetTemp();

    bool CalibrateAndWriteOffsetsToFile(const char *path);
    bool UploadOffsetsFromFile(const char *path);

private :
    void SetOffsets();
    bool AutoCalibration();

    Eigen::Vector3i GetAccelDataRaw();
    Eigen::Vector3i GetGyroDataRaw();

    int16_t ReadInt16FromRegisters(int reg);
    void WriteInt16ToRegisters(int reg, int16_t data);
    
    int m_i2c_handle;

    static constexpr uint32_t k_accel_LSB = 16384;
    static constexpr uint32_t k_gyro_LSB = 131;

    static constexpr float k_gyro_sens = (1000. / k_gyro_LSB) / 1000.;
    static constexpr float k_accel_sens = (1000. / k_accel_LSB) / 1000.;

    static constexpr uint32_t k_auto_calibration_accel_limit = 100;
    static constexpr uint32_t k_auto_calibration_gyro_limit = 20;

    Eigen::Vector3i m_gyroOffset;
    Eigen::Vector3i m_accelOffset;
};
    
} // namespace imu


#endif // __MPU_6050__