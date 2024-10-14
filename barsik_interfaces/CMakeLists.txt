cmake_minimum_required(VERSION 3.8)
project(barsik_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(geometry_msgs REQUIRED)

set(msg_files
  "msg/Fire.msg"
  "msg/SetAngleDiff.msg"
  "msg/ActivateDataProvider.msg"
  "msg/FisheyeStatus.msg"
  "msg/DebugSaveCurrentImage.msg"
  "msg/Detection.msg"
  "msg/PreciseDetections.msg"
)

set(srv_files
  "srv/GetAngle.srv"
)

rosidl_generate_interfaces(barsik_interfaces
  ${msg_files}
  ${srv_files}
#  DEPENDENCIES geometry_msgs
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
