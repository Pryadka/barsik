#include <iostream>
#include "orbital-controller.h"
#include "gun-controller.h"
#include "mpu6050-controller.h"
#include <chrono>

using namespace std::chrono_literals;

int main()
{

    gpio::GPIO_Controller gpio_controller;
    turret::OrbitalController orbi(gpio_controller);
    imu::MPU_6050_Controller imu_controller;

    turret::GunController gun{gpio_controller};

    //orbi.Calibrate(); return 0;

    orbi.GoToBasePosition(); std::this_thread::sleep_for(5s); return 0;

    std::this_thread::sleep_for(1s);
    float h = 90., v = 40.;
    for (int i = 0; i < 5; i++) {
        orbi.SetPosition({h, v});
        h *= -1;
        v *= -1.;
        std::this_thread::sleep_for(3s);
        gun.Shut();
        std::this_thread::sleep_for(1s);
        gun.Stop();
    }
    orbi.SetPosition({0.f, 0.f});
    std::this_thread::sleep_for(5s);
    // for (int i = 0; i < 1000; i++) {
    //     auto t = imu.GetAccelData();
    //     std::cout<< std::endl << "T=" << t << std::endl;
    // }

    return 0;
}