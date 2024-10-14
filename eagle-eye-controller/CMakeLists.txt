cmake_minimum_required(VERSION 3.25)
project(eagle-eye-controller)

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

install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})

ament_package()
