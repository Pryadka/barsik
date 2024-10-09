cmake_minimum_required(VERSION 3.25)

project(edgetpu-nn-lib VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 20)

file(GLOB SRCS
     "src/*.cpp"
)

file(GLOB HDRS
     "include/*.h"
)

add_library(${PROJECT_NAME} STATIC ${SRCS} ${HDRS})

# set your own path
set(TENSORFLOW_SOURCE_DIR "/home/sasa-r/src/tensorflow")

if(NOT TENSORFLOW_SOURCE_DIR)
  get_filename_component(TENSORFLOW_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
endif()

add_subdirectory("${TENSORFLOW_SOURCE_DIR}/tensorflow/lite"  "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)

find_package(ament_cmake REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} include)

# ament_export_targets(${PROJECT_NAME} HAS_LIBRARY_TARGET)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OpenCV_LIBS}
    libedgetpu.so
    tensorflow-lite
)

install(
    DIRECTORY include
    DESTINATION ${PROJECT_NAME}
)

install(
  TARGETS ${PROJECT_NAME}
  EXPORT  ${PROJECT_NAME}
  LIBRARY DESTINATION lib
)

ament_package()
