#!/bin/bash
# Скрипт встановлення ROS2 Jazzy на чистій Ubuntu 24.04
# Виконується один раз перед збіркою проекту
set -e

echo "=== Встановлення ROS2 Jazzy ==="

# ---------------------------------------------------------------------------
# Локаль
# ROS2 вимагає UTF-8 локаль для коректної роботи
# ---------------------------------------------------------------------------
sudo apt update
sudo apt install -y locales
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8

# ---------------------------------------------------------------------------
# Universe репозиторій
# Потрібен для деяких залежностей ROS2
# ---------------------------------------------------------------------------
sudo apt install -y software-properties-common
sudo add-apt-repository -y universe

# ---------------------------------------------------------------------------
# ROS2 apt джерело
# Офіційний репозиторій пакетів ROS2 від Open Robotics
# Версія визначається автоматично через GitHub API
# ---------------------------------------------------------------------------
sudo apt install -y curl git
ROS_APT_SOURCE_VERSION=$(curl -s https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest \
    | grep -F "tag_name" | awk -F'"' '{print $4}')

# Завантаження та встановлення .deb пакету з джерелом репозиторію
curl -L -o /tmp/ros2-apt-source.deb \
    "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.$(. /etc/os-release && echo ${UBUNTU_CODENAME:-${VERSION_CODENAME}})_all.deb"
sudo dpkg -i /tmp/ros2-apt-source.deb

# ---------------------------------------------------------------------------
# Встановлення ROS2 Jazzy
# ros-jazzy-desktop — повна версія з RViz2, rqt та іншими GUI інструментами
# ros-dev-tools    — colcon, rosdep та інші інструменти розробки
# ---------------------------------------------------------------------------
sudo apt update && sudo apt upgrade -y
sudo apt install -y ros-jazzy-desktop ros-dev-tools

# ---------------------------------------------------------------------------
# Ініціалізація rosdep
# rosdep автоматично встановлює системні залежності пакетів ROS2
# ---------------------------------------------------------------------------
sudo rosdep init || true   # || true — не падати якщо вже ініціалізовано
rosdep update

echo ""
echo "=== ROS2 Jazzy встановлено ==="
echo ""
echo "Додай source до ~/.bashrc щоб не вводити щоразу вручну:"
echo "  echo 'source /opt/ros/jazzy/setup.bash' >> ~/.bashrc"
echo "  source ~/.bashrc"
echo ""
echo "Далі: ./scripts/build.sh"
