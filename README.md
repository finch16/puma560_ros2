# PUMA560 ROS2

Симуляція поведінки робота PUMA560 в Gazebo та зв'язок з реальним роботом через ROS2. Включає URDF-модель у xacro, ROS2 Control з кастомним hardware interface для керування фізичним роботом.

## Структура

- `urdf/` — xacro-файли моделі робота
- `meshes/` — 3D-меші ланок робота
- `config/` — конфігурації контролерів та bridge
- `launch/` — файли запуску симуляції або з'єднання робота
- `world/` — додаткові об'єкти для Gazebo
- `src/` — кастомний hardware interface

## Збірка

```bash
colcon build
source install/setup.bash
```

## Запуск

```bash
# Rviz + контролери (реальний робот)
ros2 launch puma560_description robot_connection.xml

# або

# Rviz + Gazebo (симуляція)
ros2 launch puma560_description simulation_puma560.xml
```
