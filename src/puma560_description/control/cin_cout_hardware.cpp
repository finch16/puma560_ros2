#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <iostream>

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "rclcpp/rclcpp.hpp"

namespace your_hardware_package
{

class CinCoutHardware : public hardware_interface::SystemInterface
{
public:
    CinCoutHardware();
    ~CinCoutHardware();
    
    hardware_interface::CallbackReturn on_init(
        const hardware_interface::HardwareInfo & info) override;
    
    hardware_interface::CallbackReturn on_deactivate(
        const rclcpp::Time & time,
        const rclcpp::Duration & period) override;
    
    hardware_interface::CallbackReturn on_cleanup(
        const rclcpp::Time & time,
        const rclcpp::Duration & period) override;
    
    std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
    std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;
    
    hardware_interface::return_type read(
        const rclcpp::Time & time,
        const rclcpp::Duration & period) override;
    
    hardware_interface::return_type write(
        const rclcpp::Time & time,
        const rclcpp::Duration & period) override;
    
private:
    std::vector<std::string> joint_names_;
    std::vector<double> hw_commands_;
    std::vector<double> hw_positions_;
    std::vector<double> hw_velocities_;
    std::vector<double> prev_positions_;  // для вычисления скорости
    
    std::unique_ptr<std::thread> input_thread_;
    std::atomic<bool> running_{false};
    
    void inputLoop();
    void printStatus();
    void stopInputThread();
};

} // namespace

#include <iomanip>
#include <sstream>

namespace your_hardware_package
{

CinCoutHardware::CinCoutHardware()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "  CinCoutHardware для PUMA 560" << std::endl;
    std::cout << "========================================" << std::endl;
}

CinCoutHardware::~CinCoutHardware()
{
    stopInputThread();
}

void CinCoutHardware::stopInputThread()
{
    if (input_thread_ && input_thread_->joinable()) {
        running_ = false;
        input_thread_->join();
    }
}

hardware_interface::CallbackReturn CinCoutHardware::on_init(
    const hardware_interface::HardwareInfo & info)
{
    if (hardware_interface::SystemInterface::on_init(info) != 
        hardware_interface::CallbackReturn::SUCCESS)
    {
        return hardware_interface::CallbackReturn::ERROR;
    }
    
    std::cout << "\n[on_init] Загружаем конфигурацию..." << std::endl;
    
    size_t num_joints = info_.joints.size();
    if (num_joints == 0) {
        RCLCPP_ERROR(rclcpp::get_logger("CinCoutHardware"), "Нет суставов!");
        return hardware_interface::CallbackReturn::ERROR;
    }
    
    joint_names_.resize(num_joints);
    hw_commands_.resize(num_joints, 0.0);
    hw_positions_.resize(num_joints, 0.0);
    hw_velocities_.resize(num_joints, 0.0);
    prev_positions_.resize(num_joints, 0.0);  // ИНИЦИАЛИЗАЦИЯ prev_pos
    
    for (size_t i = 0; i < num_joints; ++i) {
        joint_names_[i] = info_.joints[i].name;
        std::cout << "  Сустав " << i << ": " << joint_names_[i] << std::endl;
    }
    
    std::cout << "[on_init] Загружено " << num_joints << " суставов" << std::endl;
    
    running_ = true;
    input_thread_ = std::make_unique<std::thread>(&CinCoutHardware::inputLoop, this);
    
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn CinCoutHardware::on_deactivate(
    const rclcpp::Time & /*time*/,
    const rclcpp::Duration & /*period*/)
{
    std::cout << "\n[on_deactivate] Деактивация..." << std::endl;
    stopInputThread();
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn CinCoutHardware::on_cleanup(
    const rclcpp::Time & /*time*/,
    const rclcpp::Duration & /*period*/)
{
    std::cout << "\n[on_cleanup] Очистка..." << std::endl;
    stopInputThread();
    return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> CinCoutHardware::export_state_interfaces()
{
    std::vector<hardware_interface::StateInterface> state_interfaces;
    
    for (size_t i = 0; i < joint_names_.size(); ++i) {
        state_interfaces.emplace_back(joint_names_[i], "position", &hw_positions_[i]);
        state_interfaces.emplace_back(joint_names_[i], "velocity", &hw_velocities_[i]);
    }
    
    std::cout << "[export_state_interfaces] Зарегистрировано " 
              << state_interfaces.size() << " state interfaces" << std::endl;
    
    return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> CinCoutHardware::export_command_interfaces()
{
    std::vector<hardware_interface::CommandInterface> command_interfaces;
    
    for (size_t i = 0; i < joint_names_.size(); ++i) {
        command_interfaces.emplace_back(joint_names_[i], "position", &hw_commands_[i]);
    }
    
    std::cout << "[export_command_interfaces] Зарегистрировано " 
              << command_interfaces.size() << " command interfaces" << std::endl;
    
    return command_interfaces;
}

hardware_interface::return_type CinCoutHardware::read(
    const rclcpp::Time & /*time*/,
    const rclcpp::Duration & /*period*/)
{
    return hardware_interface::return_type::OK;
}

hardware_interface::return_type CinCoutHardware::write(
    const rclcpp::Time & /*time*/,
    const rclcpp::Duration & /*period*/)
{
    std::cout << "\n[write] КОМАНДЫ ОТ КОНТРОЛЛЕРА:" << std::endl;
    for (size_t i = 0; i < joint_names_.size(); ++i) {
        std::cout << "  " << joint_names_[i] << " = " 
                  << std::fixed << std::setprecision(3) << hw_commands_[i] 
                  << " rad" << std::endl;
    }
    
    return hardware_interface::return_type::OK;
}

void CinCoutHardware::inputLoop()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "  ВВОД СОСТОЯНИЯ РОБОТА" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Формат: <индекс сустава> <позиция в радианах>" << std::endl;
    std::cout << "Пример: 0 1.57   (установить joint1 в 1.57 рад)" << std::endl;
    std::cout << "Команды: status - показать состояние" << std::endl;
    std::cout << "         quit   - выход" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::string line;
    
    while (running_ && std::getline(std::cin, line)) {
        if (line == "quit" || line == "exit") {
            std::cout << "Завершение..." << std::endl;
            break;
        }
        
        if (line == "status") {
            printStatus();
            continue;
        }
        
        std::istringstream iss(line);
        size_t idx;
        double pos;
        
        if (iss >> idx >> pos) {
            if (idx < joint_names_.size()) {
                // ИСПРАВЛЕНО: вычисляем скорость ДО обновления позиции
                hw_velocities_[idx] = (pos - prev_positions_[idx]) * 100.0;
                hw_positions_[idx] = pos;
                prev_positions_[idx] = pos;
                
                std::cout << "✓ " << joint_names_[idx] << " -> " 
                          << pos << " rad, скорость: " << hw_velocities_[idx] << std::endl;
            } else {
                std::cout << "Ошибка! Индекс должен быть 0-" << joint_names_.size()-1 << std::endl;
            }
        } else {
            std::cout << "Неверный формат! Используйте: <индекс> <позиция>" << std::endl;
        }
    }
    
    running_ = false;
}

void CinCoutHardware::printStatus()
{
    std::cout << "\n╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    СОСТОЯНИЕ РОБОТА                             ║" << std::endl;
    std::cout << "╠══════╦══════════════════╦══════════════╦════════════════════════╣" << std::endl;
    std::cout << "║ Инд. ║ Сустав           ║ Позиция (rad)║ Скорость (rad/s)       ║" << std::endl;
    std::cout << "╠══════╬══════════════════╬══════════════╬════════════════════════╣" << std::endl;
    
    for (size_t i = 0; i < joint_names_.size(); ++i) {
        std::cout << "║ " << std::setw(4) << i << " ║ "
                  << std::setw(16) << joint_names_[i] << " ║ "
                  << std::setw(12) << std::fixed << std::setprecision(3) << hw_positions_[i] << " ║ "
                  << std::setw(22) << std::fixed << std::setprecision(3) << hw_velocities_[i] << " ║"
                  << std::endl;
    }
    
    std::cout << "╚══════╩══════════════════╩══════════════╩════════════════════════╝" << std::endl;
    
    std::cout << "\nТекущие команды от контроллера:" << std::endl;
    for (size_t i = 0; i < joint_names_.size(); ++i) {
        std::cout << "  " << joint_names_[i] << " → " << hw_commands_[i] << " rad" << std::endl;
    }
    std::cout << std::endl;
}

} // namespace

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(your_hardware_package::CinCoutHardware, hardware_interface::SystemInterface)
