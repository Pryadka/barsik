cmake_minimum_required(VERSION 3.18)

project(turret-controller VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lpigpio")

file(GLOB SRCS
     "*.cpp"
)

file(GLOB HDRS
     "*.h"
)

add_executable(${PROJECT_NAME} ${SRCS} ${HDRS})

find_package (Eigen3 3.3 REQUIRED NO_MODULE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    lgpio
    Eigen3::Eigen
)