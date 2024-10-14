cmake_minimum_required(VERSION 3.25)

project(turret_controller VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lpigpio")

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

file(GLOB SRCS
     "src/*.cpp"
)

file(GLOB HDRS
     "include/*.h"
)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(barsik_interfaces REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} include)

add_executable(${PROJECT_NAME} ${SRCS} ${HDRS})
ament_target_dependencies(${PROJECT_NAME} rclcpp barsik_interfaces)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    lgpio
    Eigen3::Eigen
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})

ament_package()
