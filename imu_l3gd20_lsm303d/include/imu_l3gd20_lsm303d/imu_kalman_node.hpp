#ifndef IMU_KALMAN_NODE_HPP_
#define IMU_KALMAN_NODE_HPP_

#include <cmath>
#include <cstdint>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "sensor_msgs/msg/fluid_pressure.hpp"
#include "sensor_msgs/msg/temperature.hpp"
#include "std_msgs/msg/float64.hpp"

#define LSM303D_ADDR 0x19
#define MAG_ADDR     0x1E
#define L3GD20_ADDR  0x6B
#define BMP180_ADDR  0x77

#define ACCEL_SCALE  16384.0
#define GYRO_SCALE   114.28
#define G_TO_MS2     9.80665
#define DEG_TO_RAD   (M_PI / 180.0)

class KalmanFilter {
public:
    double update(double new_angle, double new_rate, double dt);

private:
    double Q_angle_ = 0.001;
    double Q_bias_  = 0.003;
    double R_angle_ = 0.03;
    double angle_ = 0.0;
    double bias_  = 0.0;
    double P_[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
};

class ImuKalmanNode : public rclcpp::Node {
public:
    ImuKalmanNode();
    ~ImuKalmanNode();

private:
    bool writeRegister(int addr, uint8_t reg, uint8_t val);
    bool readWord3D(int addr, uint8_t reg, int16_t &x, int16_t &y, int16_t &z);
    bool readBMP180Short(uint8_t reg, int16_t &value);
    bool readBMP180UShort(uint8_t reg, uint16_t &value);
    bool readBMP180Calibration();
    void readBMP180Data(double &temperature, double &pressure);
    void update_callback();

    int i2c_file_;
    int bmp_counter_ = 0;

    rclcpp::Time last_time_;

    KalmanFilter kalmanX_;
    KalmanFilter kalmanY_;

    int16_t ac1_, ac2_, ac3_, b1_, b2_, mb_, mc_, md_;
    uint16_t ac4_, ac5_, ac6_;

    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr yaw_pub_;
    rclcpp::Publisher<sensor_msgs::msg::Temperature>::SharedPtr temp_pub_;
    rclcpp::Publisher<sensor_msgs::msg::FluidPressure>::SharedPtr press_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

#endif  // IMU_KALMAN_NODE_HPP_
