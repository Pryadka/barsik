cmake_minimum_required(VERSION 3.18)
project(fisheye-controller)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set (CMAKE_CXX_STANDARD 23)

file(GLOB SRCS
     "src/*.cpp"
)

file(GLOB HDRS
     "include/*.h"
)

find_package(ament_cmake REQUIRED)
find_package(OpenCV REQUIRED)
find_package(rclcpp REQUIRED)
find_package(barsik_interfaces REQUIRED)
find_package(edgetpu-nn-lib REQUIRED)

add_executable(${PROJECT_NAME} ${SRCS} ${HDRS})
ament_target_dependencies(${PROJECT_NAME} rclcpp barsik_interfaces edgetpu-nn-lib)

include_directories(${OpenCV_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    libcamera.so
    libcamera-base.so
)

target_include_directories(${PROJECT_NAME} PUBLIC /usr/local/include/libcamera)
target_include_directories(${PROJECT_NAME} PUBLIC /usr/include/libcamera)

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

install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})

ament_package()
