#!/bin/bash
# Скрипт збірки пакету puma560_ros2
# Виконується після клонування репозиторію та встановлення ROS2
set -e

# Визначаємо кореневу директорію проекту (де лежить цей скрипт)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WS_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=== Збірка puma560_ros2 ==="
echo "Директорія workspace: $WS_ROOT"
cd "$WS_ROOT"

# ---------------------------------------------------------------------------
# Перевірка що ROS2 sourced
# Без цього colcon та rosdep не знайдуть ROS2 пакети
# ---------------------------------------------------------------------------
if [ -z "$ROS_DISTRO" ]; then
    echo ""
    echo "ПОМИЛКА: ROS2 не підключено."
    echo "Виконай: source /opt/ros/jazzy/setup.bash"
    echo ""
    exit 1
fi
echo "ROS2 дистрибутив: $ROS_DISTRO"

# ---------------------------------------------------------------------------
# rosdep install
# Автоматично встановлює всі системні залежності описані в package.xml
# --from-paths src  — шукати пакети у папці src/
# --ignore-src      — не встановлювати залежності що вже є у src/
# -y                — відповідати "так" на всі питання
# ---------------------------------------------------------------------------
echo ""
echo "--- Встановлення залежностей через rosdep ---"
rosdep install --from-paths src --ignore-src -y

# ---------------------------------------------------------------------------
# colcon build
# Система збірки для ROS2 (аналог catkin для ROS1)
# Компілює C++ код та встановлює Python скрипти у папку install/
# ---------------------------------------------------------------------------
echo ""
echo "--- Збірка пакетів ---"
colcon build

echo ""
echo "=== Збірка завершена ==="
echo ""
echo "Для запуску необхідно підключити workspace:"
echo "  source install/setup.bash"
echo ""
echo "Варіанти запуску:"
echo "  ros2 launch puma560_description display_puma560.xml          # перегляд URDF в RViz"
echo "  ros2 launch puma560_description simulation_puma560.xml       # симуляція в Gazebo"
echo "  ros2 launch puma560_description test_robot_hardware_interface.xml  # тест hardware interface"
