#include "mpu6050-controller.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

namespace imu
{
using namespace std::chrono_literals;

bool MPU_6050_Controller::AutoCalibration()
{
    bool gyroStop = false, accelStop = false;

    int i = 0;

    while((i++ < 100) && (!gyroStop || !accelStop)) {
        if (!gyroStop) {
            auto g_d = GetGyroDataRaw();
            std::cout << "GYRO=" << g_d << std::endl;
            if (abs(g_d(0)) < k_auto_calibration_gyro_limit
                && abs(g_d(1)) < k_auto_calibration_gyro_limit
                && abs(g_d(2)) < k_auto_calibration_gyro_limit)
            {
                gyroStop = true;
            }
            else
            {
                g_d /= 8;
                m_gyroOffset -= g_d;
            }
        }

        if (!accelStop) {
            auto a_d = GetAccelDataRaw();
            a_d(2) = a_d(2) - k_accel_LSB;
            if (abs(a_d(0)) < k_auto_calibration_accel_limit
                && abs(a_d(1)) < k_auto_calibration_accel_limit
                && abs(a_d(2)) < k_auto_calibration_accel_limit)
            {
                accelStop = true;
            }
            else
            {
                a_d /= 8;
                m_accelOffset -= a_d;
            }
        }

        SetOffsets();
        std::this_thread::sleep_for(100ms);
    }

    return gyroStop && accelStop;
}

MPU_6050_Controller::MPU_6050_Controller(unsigned int address) :
    m_gyroOffset{Eigen::Vector3i::Zero()},
    m_accelOffset{Eigen::Vector3i::Zero()}
{
    m_i2c_handle = lgI2cOpen(1, address, 0);
    if (m_i2c_handle < 0) {
        std::cout << "Error openning i2c device" << std::endl;
    } else {
        lgI2cWriteByteData(m_i2c_handle, 0x6b, 0x00); // wake up
        lgI2cWriteByteData(m_i2c_handle, 0x1a, 0x03); // digital low pass filter Acc=44Hz(4.9ms), Gyro=42Hz(4.8ms)
        lgI2cWriteByteData(m_i2c_handle, 0x19, 0x04); // sample rate 200Hz
        lgI2cWriteByteData(m_i2c_handle, 0x1b, 0x00); // gyro config. value 0 for +/-250deg/sec
        lgI2cWriteByteData(m_i2c_handle, 0x1c, 0x00); // accel config value 0 for +/-2g

        SetOffsets();
        // self test
        lgI2cWriteByteData(m_i2c_handle, 0x0, 0x81);
        lgI2cWriteByteData(m_i2c_handle, 0x1, 0x01);
        lgI2cWriteByteData(m_i2c_handle, 0x2, 0x81);
    }
}

MPU_6050_Controller::~MPU_6050_Controller() {
    if (m_i2c_handle >= 0) {
        lgI2cClose(m_i2c_handle);
    }
}

void MPU_6050_Controller::SetOffsets()
{
    // offsets
    // accelerometer
    // 1.Reset
    WriteInt16ToRegisters(0x06, 0x00); // x
    WriteInt16ToRegisters(0x08, 0x00); // y
    WriteInt16ToRegisters(0x0a, 0x00); // y
    // 2.Read initial
    int16_t a_x = ReadInt16FromRegisters(0x06);
    int16_t a_y = ReadInt16FromRegisters(0x08);
    int16_t a_z = ReadInt16FromRegisters(0x0a);
    auto x_mask = a_x & 1;
    auto y_mask = a_y & 1;
    auto z_mask = a_z & 1;
    a_x += m_accelOffset[0];
    a_y += m_accelOffset[1];
    a_z += m_accelOffset[2];
    a_x |= x_mask;
    a_y |= y_mask;
    a_z |= z_mask;
    // 3.Write new values
    WriteInt16ToRegisters(0x06, a_x); // x
    WriteInt16ToRegisters(0x08, a_y); // y
    WriteInt16ToRegisters(0x0a, a_z); // y

    // gyro
    WriteInt16ToRegisters(0x13, m_gyroOffset[0]); // x
    WriteInt16ToRegisters(0x15, m_gyroOffset[1]); // y
    WriteInt16ToRegisters(0x17, m_gyroOffset[2]); // z
}

Eigen::Vector3i MPU_6050_Controller::GetAccelDataRaw()
{
    Eigen::Vector3i rawData(
        ReadInt16FromRegisters(0x3b),
        ReadInt16FromRegisters(0x3d),
        ReadInt16FromRegisters(0x3f));
    
    return rawData;
}

Eigen::Vector3f MPU_6050_Controller::GetAccelData()
{
    Eigen::Vector3f res;

    res = GetAccelDataRaw().cast<float>() * k_accel_sens;

    return res;
}

Eigen::Vector3f MPU_6050_Controller::GetGyroData()
{
    Eigen::Vector3f res;

    res = GetGyroDataRaw().cast<float>() * k_gyro_sens;

    return res;
}
Eigen::Vector3i MPU_6050_Controller::GetGyroDataRaw()
{
    Eigen::Vector3i rawData(
        ReadInt16FromRegisters(0x43),
        ReadInt16FromRegisters(0x45),
        ReadInt16FromRegisters(0x47));
    
    return rawData;
}

float MPU_6050_Controller::GetTemp()
{
    const int16_t temp_out = lgI2cReadByteData(m_i2c_handle, 0x41) << 8 | lgI2cReadByteData(m_i2c_handle, 0x42);

    return temp_out / 340. + 36.53;
}

bool MPU_6050_Controller::CalibrateAndWriteOffsetsToFile(const char *path)
{
    auto res = AutoCalibration();
    if (res) {
        res = false;
        std::ofstream output{path, std::ofstream::trunc};
        if (output.good()) {
            for (int i = 0; i < 3; i++) {
                output << m_accelOffset[i] << " ";
            }
            
            for (int i = 0; i < 3; i++) {
                output << m_gyroOffset[i] << " ";
            }
            
            res = output.good();
            output.close();
        }
    }
    return res;
}

bool MPU_6050_Controller::UploadOffsetsFromFile(const char *path)
{
    bool res = false;
    std::ifstream input{path};
    if (input.good()) {
        for (int i = 0; i < 3; i++) {
            input >> m_accelOffset[i];
        }
            
        for (int i = 0; i < 3; i++) {
            input >> m_gyroOffset[i];
        };
        res = input.good();
        input.close();
    }
    return res;
}

int16_t MPU_6050_Controller::ReadInt16FromRegisters(int reg)
{
    int16_t res;
    char b1 = lgI2cReadByteData(m_i2c_handle, reg);
    char b2 = lgI2cReadByteData(m_i2c_handle, reg + 1);
    res = (b1 << 8) | b2;
    return res;
}

void MPU_6050_Controller::WriteInt16ToRegisters(int reg, int16_t data)
{
    lgI2cWriteByteData(m_i2c_handle, reg, 0xff & (data >> 8)); // high
    lgI2cWriteByteData(m_i2c_handle, reg + 1, 0xff & data); // low
}

} // namespace imu