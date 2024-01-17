#include <iostream>
#include "orbital-controller.h"
#include "mpu6050-controller.h"
#include <chrono>

using namespace std::chrono_literals;

int main()
{
    imu::MPU_6050_Controller::gyroOffset = {0,0,0}; //{48, 27, 18};
    imu::MPU_6050_Controller::accelOffset = {0, 0, 0};

    imu::MPU_6050_Controller::AutoCalibration();

    turret::OrbitalController orbi;
    imu::MPU_6050_Controller &imu = imu::MPU_6050_Controller::GetInstance();

    std::this_thread::sleep_for(1s);
    
    orbi.SetAngelsMissmatch(1., 1.);

    //std::this_thread::sleep_for(10s);
    for (int i = 0; i < 1000; i++) {
        auto t = imu.GetAccelData();
        std::cout<< std::endl << "T=" << t << std::endl;
    }

    return 0;
}