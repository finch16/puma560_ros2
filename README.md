# PUMA560 ROS2

ROS2 реалізація 6-DOF маніпулятора PUMA 560 з підтримкою симуляції в Gazebo та кастомним hardware interface для підключення реального робота.

## Архітектура

```
puma560_description/
├── urdf/          # URDF/xacro модель робота
├── meshes/        # STL геометрія ланок (7 ланок)
├── config/        # Налаштування контролерів (ros2_control)
├── launch/        # Launch файли
├── src/           # C++ hardware interface плагін
└── world/         # SDF моделі для Gazebo (камера)
```

### ros2_control стек

```
joint_trajectory_controller   — приймає траєкторії, видає команди позицій
joint_state_broadcaster       — публікує /joint_states для RViz та інших нод
RobotHardwareInterface        — кастомний плагін: симулює рух або читає реальний робот
```

Контролери працюють на частоті **100 Hz** (задається в `config/controllers.yaml`).

---

## Встановлення

### Вимоги
- Ubuntu 24.04
- ROS2 Jazzy

### 1. Встановлення ROS2 Jazzy

Якщо ROS2 ще не встановлено:

```bash
chmod +x scripts/install_ros2.sh
./scripts/install_ros2.sh
```

Після завершення підключи ROS2 до поточної сесії та до `~/.bashrc`:

```bash
echo 'source /opt/ros/jazzy/setup.bash' >> ~/.bashrc
source ~/.bashrc
```

### 2. Клонування репозиторію

```bash
git clone https://github.com/finch16/puma560_ros2.git
cd puma560_ros2
```

### 3. Збірка

```bash
chmod +x scripts/build.sh
./scripts/build.sh
```

### 4. Підключення workspace

> Виконувати в кожному новому терміналі, або додати до `~/.bashrc`

```bash
source install/setup.bash
```

---

## Запуск

### Перегляд моделі в RViz

Завантажує URDF модель у RViz з GUI повзунками для ручного керування суглобами.

```bash
ros2 launch puma560_description display_puma560.xml
```

### Симуляція в Gazebo

Запускає Gazebo з фізичною симуляцією, ros2_control, RViz та GUI контролером траєкторій.

```bash
ros2 launch puma560_description simulation_puma560.xml
```

### Тест hardware interface

Запускає ros2_control з кастомним hardware interface без Gazebo. Використовується для розробки та відлагодження підключення до реального робота.

```bash
ros2 launch puma560_description test_robot_hardware_interface.xml
```

---

## Надсилання команд

Після запуску `test_robot_hardware_interface.xml` або `simulation_puma560.xml` можна надіслати команду траєкторії:

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
