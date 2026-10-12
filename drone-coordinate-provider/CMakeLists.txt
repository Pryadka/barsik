cmake_minimum_required(VERSION 3.8)
project(drone_coordinate_provider)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(barsik_interfaces REQUIRED)

add_executable(drone_coordinate_provider_node src/drone_coordinate_provider_node.cpp)
target_include_directories(drone_coordinate_provider_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(drone_coordinate_provider_node PUBLIC c_std_99 cxx_std_17)
ament_target_dependencies(
  drone_coordinate_provider_node
  "rclcpp"
  "sensor_msgs"
  "std_msgs"
  "barsik_interfaces"
)

install(TARGETS drone_coordinate_provider_node
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
