cmake_minimum_required(VERSION 3.8)
project(puma560_description)

find_package(ament_cmake REQUIRED)

find_package(ros_gz REQUIRED)
find_package(gz_ros2_control REQUIRED)

find_package(sensor_msgs REQUIRED)
find_package(rviz_default_plugins REQUIRED)
find_package(cv_bridge QUIET)
find_package(image_transport QUIET)

install(
  DIRECTORY urdf meshes world config launch
  DESTINATION share/${PROJECT_NAME}/
)

install(
  FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
