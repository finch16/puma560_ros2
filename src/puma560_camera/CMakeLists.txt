cmake_minimum_required(VERSION 3.8)
project(puma560_camera)

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY launch world
  DESTINATION share/${PROJECT_NAME}/
)

install(
  FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()