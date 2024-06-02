cmake_minimum_required(VERSION 3.8)
project(turret_test_app)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

find_package(rclcpp REQUIRED)
find_package(barsik_interfaces REQUIRED)

add_executable(get_angle_test "src/get_angle_test.cpp")
ament_target_dependencies(get_angle_test rclcpp barsik_interfaces)

add_executable(set_angle_diff_test "src/set_angle_diff_test.cpp")
ament_target_dependencies(set_angle_diff_test rclcpp barsik_interfaces)

add_executable(gun_test "src/gun_test.cpp")
ament_target_dependencies(gun_test rclcpp barsik_interfaces)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS get_angle_test      DESTINATION lib/get_angle_test)
install(TARGETS set_angle_diff_test DESTINATION lib/set_angle_diff_test)
install(TARGETS gun_test            DESTINATION lib/gun_test)

ament_package()
