#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "imu_l3gd20_lsm303d/imu_kalman_node.hpp"

// --- KalmanFilter ---

double KalmanFilter::update(double new_angle, double new_rate, double dt) {
    double rate = new_rate - bias_;
    angle_ += dt * rate;

    P_[0][0] += dt * (dt * P_[1][1] - P_[0][1] - P_[1][0] + Q_angle_);
    P_[0][1] -= dt * P_[1][1];
    P_[1][0] -= dt * P_[1][1];
    P_[1][1] += Q_bias_ * dt;

    double S = P_[0][0] + R_angle_;
    double K[2];
    K[0] = P_[0][0] / S;
    K[1] = P_[1][0] / S;

    double y = new_angle - angle_;
    angle_ += K[0] * y;
    bias_  += K[1] * y;

    double P00_temp = P_[0][0];
    double P01_temp = P_[0][1];

    P_[0][0] -= K[0] * P00_temp;
    P_[0][1] -= K[0] * P01_temp;
    P_[1][0] -= K[1] * P00_temp;
    P_[1][1] -= K[1] * P01_temp;

    return angle_;
}

// --- ImuKalmanNode ---

ImuKalmanNode::ImuKalmanNode() : Node("imu_kalman_node"), i2c_file_(-1) {
    imu_pub_ = create_publisher<sensor_msgs::msg::Imu>("imu/data", 10);
    yaw_pub_ = create_publisher<std_msgs::msg::Float64>("imu/yaw", 10);
    temp_pub_ = create_publisher<sensor_msgs::msg::Temperature>("bmp180/temperature", 10);
    press_pub_ = create_publisher<sensor_msgs::msg::FluidPressure>("bmp180/pressure", 10);

    i2c_file_ = open("/dev/i2c-1", O_RDWR);
    if (i2c_file_ < 0) {
        RCLCPP_ERROR(get_logger(), "I2C Bus /dev/i2c-1 can't be opened!");
        throw std::runtime_error("I2C Failed");
    }

    writeRegister(LSM303D_ADDR, 0x20, 0x57);
    writeRegister(LSM303D_ADDR, 0x24, 0x64);
    writeRegister(LSM303D_ADDR, 0x26, 0x00);
    writeRegister(L3GD20_ADDR,  0x20, 0x0F);

    if (!readBMP180Calibration()) {
        RCLCPP_ERROR(get_logger(), "BMP180 calibration failed!");
        throw std::runtime_error("BMP180 Error");
    }

    last_time_ = now();

    timer_ = create_wall_timer(
        std::chrono::milliseconds(10),
        [this]() {return update_callback();}
    );

    RCLCPP_INFO(get_logger(), "IMU + BMP180 Kalman Node successfully started.");
}

ImuKalmanNode::~ImuKalmanNode() {
    if (i2c_file_ >= 0) close(i2c_file_);
}

bool ImuKalmanNode::writeRegister(int addr, uint8_t reg, uint8_t val) {
    if (ioctl(i2c_file_, I2C_SLAVE, addr) < 0) return false;
    uint8_t buf[2] = {reg, val};
    return write(i2c_file_, buf, 2) == 2;
}

bool ImuKalmanNode::readWord3D(int addr, uint8_t reg, int16_t &x, int16_t &y, int16_t &z) {
    if (ioctl(i2c_file_, I2C_SLAVE, addr) < 0) {
        return false;
    }

    uint8_t reg_buf[1] = { static_cast<uint8_t>(reg | 0x80) };
    if (write(i2c_file_, reg_buf, 1) != 1) {
        RCLCPP_ERROR(get_logger(), (std::string("i2c failed! reg=") + std::to_string(reg)).c_str());
        return false;
    }

    uint8_t data[6];
    if (read(i2c_file_, data, 6) == 6) {
        x = static_cast<int16_t>(data[0] | (data[1] << 8));
        y = static_cast<int16_t>(data[2] | (data[3] << 8));
        z = static_cast<int16_t>(data[4] | (data[5] << 8));
        return true;
    }

    return false;
}

bool ImuKalmanNode::readBMP180Short(uint8_t reg, int16_t &value) {
    if (ioctl(i2c_file_, I2C_SLAVE, BMP180_ADDR) < 0) return false;
    if (write(i2c_file_, &reg, 1) != 1) return false;
    uint8_t data[2];
    if (read(i2c_file_, data, 2) != 2) return false;
    value = static_cast<int16_t>((data[0] << 8) | data[1]);
    return true;
}

bool ImuKalmanNode::readBMP180UShort(uint8_t reg, uint16_t &value) {
    int16_t val;
    if (readBMP180Short(reg, val)) {
        value = static_cast<uint16_t>(val);
        return true;
    }
    return false;
}

bool ImuKalmanNode::readBMP180Calibration() {
    bool success = true;
    success &= readBMP180Short(0xAA, ac1_);
    success &= readBMP180Short(0xAC, ac2_);
    success &= readBMP180Short(0xAE, ac3_);
    success &= readBMP180UShort(0xB0, ac4_);
    success &= readBMP180UShort(0xB2, ac5_);
    success &= readBMP180UShort(0xB4, ac6_);
    success &= readBMP180Short(0xB6, b1_);
    success &= readBMP180Short(0xB8, b2_);
    success &= readBMP180Short(0xBA, mb_);
    success &= readBMP180Short(0xBC, mc_);
    success &= readBMP180Short(0xBE, md_);
    return success;
}

void ImuKalmanNode::readBMP180Data(double &temperature, double &pressure) {
    writeRegister(BMP180_ADDR, 0xF4, 0x2E);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    int16_t ut;
    readBMP180Short(0xF6, ut);

    writeRegister(BMP180_ADDR, 0xF4, 0x34);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    if (ioctl(i2c_file_, I2C_SLAVE, BMP180_ADDR) < 0) return;
    uint8_t reg = 0xF6;
    write(i2c_file_, &reg, 1);
    uint8_t data[3];
    read(i2c_file_, data, 3);
    int32_t up = ((data[0] << 16) | (data[1] << 8) | data[2]) >> 8;

    int32_t x1 = (ut - ac6_) * ac5_ >> 15;
    int32_t x2 = (mc_ << 11) / (x1 + md_);
    int32_t b5 = x1 + x2;
    temperature = ((b5 + 8) >> 4) / 10.0;

    int32_t b6 = b5 - 4000;
    x1 = (b2_ * (b6 * b6 >> 12)) >> 11;
    x2 = ac2_ * b6 >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = (((ac1_ * 4 + x3) << 0) + 2) >> 2;

    x1 = ac3_ * b6 >> 13;
    x2 = (b1_ * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    uint32_t b4 = ac4_ * static_cast<uint32_t>(x3 + 32768) >> 15;
    uint32_t b7 = (static_cast<uint32_t>(up - b3)) * (50000 >> 0);

    int32_t p;
    if (b7 < 0x80000000) {
        p = (b7 * 2) / b4;
    } else {
        p = (b7 / b4) * 2;
    }

    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4);

    pressure = static_cast<double>(p);
}

void ImuKalmanNode::update_callback() {
    rclcpp::Time current_time = now();
    double dt = (current_time - last_time_).seconds();
    last_time_ = current_time;

    if (dt <= 0) dt = 0.01;

    int16_t acc_x, acc_y, acc_z;
    int16_t mag_x, mag_y, mag_z;
    int16_t gyro_x, gyro_y, gyro_z;

    if (!readWord3D(LSM303D_ADDR, 0x28, acc_x, acc_y, acc_z) ||
        !readWord3D(MAG_ADDR, 0x03, mag_x, mag_y, mag_z) ||
        !readWord3D(L3GD20_ADDR,  0x28, gyro_x, gyro_y, gyro_z)) {
        RCLCPP_ERROR(get_logger(), "IMU data read failed!");
        return;
    }

    double ax = (acc_x / ACCEL_SCALE) * G_TO_MS2;
    double ay = (acc_y / ACCEL_SCALE) * G_TO_MS2;
    double az = (acc_z / ACCEL_SCALE) * G_TO_MS2;
    double gx_deg = gyro_x / GYRO_SCALE;
    double gy_deg = gyro_y / GYRO_SCALE;
    double gz_rad = (gyro_z / GYRO_SCALE) * DEG_TO_RAD;

    double acc_roll = std::atan2(ay, az) * 180.0 / M_PI;
    double acc_pitch = std::atan2(-ax, std::sqrt(ay * ay + az * az)) * 180.0 / M_PI;

    double roll_deg = kalmanX_.update(acc_roll, gx_deg, dt);
    double pitch_deg = kalmanY_.update(acc_pitch, gy_deg, dt);
    double roll_rad = roll_deg * DEG_TO_RAD;
    double pitch_rad = pitch_deg * DEG_TO_RAD;

    double mx_comp = mag_x * std::cos(pitch_rad) + mag_z * std::sin(pitch_rad);
    double my_comp = mag_x * std::sin(roll_rad) * std::sin(pitch_rad)
                   + mag_y * std::cos(roll_rad)
                   - mag_z * std::sin(roll_rad) * std::cos(pitch_rad);
    double yaw_deg = std::atan2(-my_comp, mx_comp) * 180.0 / M_PI;
    if (yaw_deg < 0) yaw_deg += 360.0;
    double yaw_rad = yaw_deg * DEG_TO_RAD;

    double cy = std::cos(yaw_rad * 0.5); double sy = std::sin(yaw_rad * 0.5);
    double cp = std::cos(pitch_rad * 0.5); double sp = std::sin(pitch_rad * 0.5);
    double cr = std::cos(roll_rad * 0.5); double sr = std::sin(roll_rad * 0.5);

    sensor_msgs::msg::Imu imu_msg;
    imu_msg.header.stamp = current_time;
    imu_msg.header.frame_id = "imu_link";
    imu_msg.orientation.w = cr * cp * cy + sr * sp * sy;
    imu_msg.orientation.x = sr * cp * cy - cr * sp * sy;
    imu_msg.orientation.y = cr * sp * cy + sr * cp * sy;
    imu_msg.orientation.z = cr * cp * sy - sr * sp * cy;
    imu_msg.linear_acceleration.x = ax;
    imu_msg.linear_acceleration.y = ay;
    imu_msg.linear_acceleration.z = az;
    imu_msg.angular_velocity.x = gx_deg * DEG_TO_RAD;
    imu_msg.angular_velocity.y = gy_deg * DEG_TO_RAD;
    imu_msg.angular_velocity.z = gz_rad;
    imu_pub_->publish(imu_msg);

    std_msgs::msg::Float64 yaw_msg;
    yaw_msg.data = yaw_deg;
    yaw_pub_->publish(yaw_msg);

    bmp_counter_++;
    if (bmp_counter_ >= 10) {
        bmp_counter_ = 0;
        double temperature = 0.0;
        double pressure = 0.0;
        readBMP180Data(temperature, pressure);

        sensor_msgs::msg::Temperature temp_msg;
        temp_msg.header.stamp = current_time;
        temp_msg.header.frame_id = "imu_link";
        temp_msg.temperature = temperature;
        temp_pub_->publish(temp_msg);

        sensor_msgs::msg::FluidPressure press_msg;
        press_msg.header.stamp = current_time;
        press_msg.header.frame_id = "imu_link";
        press_msg.fluid_pressure = pressure;
        press_pub_->publish(press_msg);
    }
}

int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ImuKalmanNode>());
    rclcpp::shutdown();
    return 0;
}
