# PUMA560 ROS2

Симуляція поведінки робота PUMA560 в Gazebo та зв'язок з реальним роботом через ROS2. Включає URDF-модель у xacro, ROS2 Control з кастомним hardware interface для керування фізичним роботом.

## Структура

Робочий простір поділений на чотири пакети за призначенням:

| Пакет | Призначення |
|---|---|
| `puma560_description` | URDF/xacro модель робота, STL меші, конфігурація контролерів |
| `puma560_hardware` | C++ hardware interface плагін і TCP клієнт |
| `puma560_camera` | SDF модель overhead-камери та Gazebo bridge для зображення |
| `puma560_launch` | Launch-файли симуляції та реального робота, конфігурація RViz |

```
puma560_ws/
├── src/
│   ├── puma560_description/
│   │   ├── urdf/              # URDF/xacro модель робота
│   │   ├── meshes/            # STL геометрія ланок (7 ланок)
│   │   └── config/            # Налаштування контролерів (ros2_control)
│   ├── puma560_hardware/
│   │   ├── include/           # Заголовки плагіна та TCP клієнта
│   │   ├── src/               # Реалізація RobotHardwareInterface
│   │   ├── frame_codec/       # submodule: код протоколу обміну (C)
│   │   └── robot_hardware_interface.xml  # опис плагіна для pluginlib
│   ├── puma560_camera/
│   │   ├── world/             # SDF модель overhead-камери
│   │   └── launch/            # Спуск камери в Gazebo та image bridge
│   └── puma560_launch/
│       ├── launch/            # robot_connection.xml, simulation_puma560.xml
│       └── config/            # Конфігурація RViz
```

### ros2_control стек

```
joint_trajectory_controller   — приймає траєкторії, видає команди позицій
joint_state_broadcaster       — публікує /joint_states для RViz та інших нод
RobotHardwareInterface        — кастомний TCP-плагін для фізичного робота
```

Контролери працюють на частоті **100 Hz** (задається в `puma560_description/config/controllers.yaml`).

Плагін вибирається через xacro-аргумент `use_sim_time`: під час симуляції URDF
підставляє `gz_ros2_control/GazeboSimSystem`, при підключенні реального робота —
`puma560_hardware/RobotHardwareInterface`.

---

## Встановлення

### Вимоги

- Ubuntu 24.04
- ROS2 Jazzy

### 1. Налаштування середовищя

```bash
source /opt/ros/jazzy/setup.bash
```

### 2. Клонування репозиторію

```bash
git clone https://github.com/finch16/puma560_ros2.git
cd puma560_ros2
```

### 3. Ініціалізація submodule

У `puma560_hardware` використовується submodule `frame_codec`.

```bash
git submodule update --init --recursive
```

### 4. Збірка

```bash
colcon build
```

### 5. Підключення workspace

```bash
source install/setup.bash
```

---

## Запуск

### Підключення реального робота

Запускає RViz, rqt-контролер траєкторій і ros2_control з TCP-драйвером
`RobotHardwareInterface`. Параметри підключення (IP, порт) задаються в
`puma560_hardware/src/robot_hardware_interface.cpp`.

```bash
ros2 launch puma560_launch robot_connection.xml
```

### Симуляція в Gazebo

Запускає Gazebo з фізичною симуляцією, overhead-камеру, ros2_control, RViz та
GUI контролером траєкторій.

```bash
ros2 launch puma560_launch simulation_puma560.xml
```

---

## Надсилання команд

Після запуску можна надіслати команду траєкторії:

```bash
ros2 action send_goal /joint_trajectory_controller/follow_joint_trajectory \
  control_msgs/action/FollowJointTrajectory \
  "{
    trajectory: {
      joint_names: [joint1, joint2, joint3, joint4, joint5, joint6],
      points: [{
        positions: [0.5, 0.5, 0.5, 0.5, 0.5, 0.5],
        time_from_start: {sec: 3, nanosec: 0}
      }]
    }
  }"
```

Моніторинг поточних кутів суглобів:

```bash
ros2 topic echo /joint_states
```

---

## Рух у суглобах

Межі задані у `puma560_description/urdf/puma560_robot.urdf.xacro`:

| Суглоб | Діапазон, рад |
|---|---|
| joint1 | ±3.1416 |
| joint2 | −0.8552 … 3.9968 |
| joint3 | −0.8552 … 3.9968 |
| joint4 | −3.3161 … 2.1817 |
| joint5 | ±1.7453 |
| joint6 | ±3.1416 |
