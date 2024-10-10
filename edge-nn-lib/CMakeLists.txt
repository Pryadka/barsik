cmake_minimum_required(VERSION 3.25)

project(edgetpu-nn-lib VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 20)

file(GLOB SRCS
     "src/*.cpp"
)

file(GLOB HDRS
     "include/*.h"
)

add_library(${PROJECT_NAME} SHARED ${SRCS} ${HDRS})

# set your own path
set(TENSORFLOW_SOURCE_DIR "/home/sasa-r/src/tensorflow")

if(NOT TENSORFLOW_SOURCE_DIR)
  get_filename_component(TENSORFLOW_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
endif()

add_subdirectory("${TENSORFLOW_SOURCE_DIR}/tensorflow/lite"  "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} include)
target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
# ament_export_dependencies(tensorflow-lite)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE ${OpenCV_LIBS}
    PRIVATE libedgetpu.so
    PRIVATE tensorflow-lite
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT  export_${PROJECT_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

ament_export_include_directories(include)
ament_package()