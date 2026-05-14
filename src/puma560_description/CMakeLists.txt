cmake_minimum_required(VERSION 3.8)
project(puma560_description)

find_package(ament_cmake REQUIRED)

find_package(ros_gz REQUIRED)
find_package(gz_ros2_control REQUIRED)

find_package(sensor_msgs REQUIRED)
find_package(rviz_default_plugins REQUIRED)
find_package(cv_bridge QUIET)
find_package(image_transport QUIET)

find_package(pluginlib REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(rclcpp REQUIRED)

install(
  DIRECTORY urdf meshes world config launch
  DESTINATION share/${PROJECT_NAME}/
)

install(
  FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

add_library(robot_hardware_interface SHARED
  src/robot_hardware_interface.cpp
)

target_include_directories(robot_hardware_interface PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(robot_hardware_interface
  ${pluginlib_LIBRARIES}
  hardware_interface::hardware_interface
  rclcpp::rclcpp
)

pluginlib_export_plugin_description_file(hardware_interface robot_hardware_interface.xml)

install(TARGETS robot_hardware_interface
  EXPORT export_${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY include/ DESTINATION include)

ament_package()
