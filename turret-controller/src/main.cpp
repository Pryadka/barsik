#include "orbital-controller.h"
#include "gun-controller.h"
#include "mpu6050-controller.h"

#include "rclcpp/rclcpp.hpp"

#include "GetAngleService.h"
#include "FireSubscriber.h"
#include "SetAngleDiffSubscriber.h"

using namespace std::chrono_literals;

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    gpio::GPIO_Controller gpio_controller;

    turret::OrbitalController orbi{gpio_controller};
    turret::GunController gun{gpio_controller};

    orbi.GoToBasePosition();

    auto gun_node = std::make_shared<FireSubscriber>(gun);
    auto get_angle_node = std::make_shared<GetAngleService>(orbi);
    auto set_angle_diff_node = std::make_shared<SetAngleDiffSubscriber>(orbi);

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(gun_node);
    executor.add_node(get_angle_node);
    executor.add_node(set_angle_diff_node);
    executor.spin();
  
    return 0;
}

// int main()
// {

//     gpio::GPIO_Controller gpio_controller;
//     turret::OrbitalController orbi(gpio_controller);
//     imu::MPU_6050_Controller imu_controller;

//     turret::GunController gun{gpio_controller};

//     //orbi.Calibrate(); //return 0;

//     orbi.GoToBasePosition(); 

//     std::this_thread::sleep_for(1s);
//     float h = 90., v = 40.;
//     for (int i = 0; i < 5; i++) {
//         orbi.SetPosition({h, v});
//         h *= -1;
//         v *= -1.;
//         std::this_thread::sleep_for(3s);
//         gun.Shut();
//         std::this_thread::sleep_for(1s);
//         gun.Stop();
//     }
//     orbi.SetPosition({0.f, 0.f});
//     std::this_thread::sleep_for(5s);
//     // for (int i = 0; i < 1000; i++) {
//     //     auto t = imu.GetAccelData();
//     //     std::cout<< std::endl << "T=" << t << std::endl;
//     // }

//     return 0;
// }