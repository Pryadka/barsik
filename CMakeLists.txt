cmake_minimum_required(VERSION 3.22)

project(barsik VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(proto)
add_subdirectory(eigen)
add_subdirectory(turret-controller)
add_subdirectory(barsik_interfaces)

add_subdirectory(turret_test_app)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

ament_package()
