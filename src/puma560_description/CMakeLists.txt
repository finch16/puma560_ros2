cmake_minimum_required(VERSION 3.8)
project(puma560_description)

find_package(ament_cmake REQUIRED)
#find_package(rclcpp REQUIRED)
#find_package(robot_state_publisher REQUIRED)
#find_package(joint_state_publisher REQUIRED)
#find_package(joint_state_publisher_gui REQUIRED)
#find_package(rviz2 REQUIRED)
#find_package(xacro REQUIRED)

install(
  DIRECTORY urdf meshes config launch
  DESTINATION share/${PROJECT_NAME}/
)

install(
  FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
