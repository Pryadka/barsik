cmake_minimum_required(VERSION 3.22)

project(barsik VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(proto)
add_subdirectory(eigen)
add_subdirectory(turret-controller)