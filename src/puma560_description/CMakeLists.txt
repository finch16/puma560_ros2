cmake_minimum_required(VERSION 3.8)
project(puma560_description)

find_package(ament_cmake REQUIRED)
find_package(ros_gz REQUIRED)
find_package(gz_ros2_control REQUIRED)

install(
  DIRECTORY urdf meshes config launch
  DESTINATION share/${PROJECT_NAME}/
)

install(
  FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
