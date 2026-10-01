cmake_minimum_required(VERSION 3.8)
project(puma560_launch)

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME}/
)

install(
  FILES package.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()