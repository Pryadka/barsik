cmake_minimum_required(VERSION 3.22)

project(barsik VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_subdirectory(eigen)
add_subdirectory(barsik_interfaces)

add_subdirectory(turret-controller)
add_subdirectory(fisheye_controller)

add_subdirectory(turret_test_app)

ament_package()
