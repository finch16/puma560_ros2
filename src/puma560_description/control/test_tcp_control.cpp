#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "rclcpp/rclcpp.hpp"

// Асинхронный TCP-клиент (например, на основе Boost.Asio)
// или ваш собственный класс
class TcpClient {
public:
    TcpClient(const std::string& host, int port);
    ~TcpClient();
    
    bool connect();
    void disconnect();
    bool isConnected() const;
    
    bool send(const std::vector<double>& data);
    bool receive(std::vector<double>& data);
    
private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

class test_tcp_control : public hardware_interface::SystemInterface
{
public:
    
    hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareInfo & info) override;
    hardware_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State & previous_state) override;
    hardware_interface::CallbackReturn on_cleanup(const rclcpp_lifecycle::State & previous_state) override;
    std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
    std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;
    hardware_interface::return_type read(const rclcpp::Time& time, const rclcpp::Duration& period) override;
    hardware_interface::return_type write(const rclcpp::Time& time, const rclcpp::Duration& period) override;
    
private:
    std::vector<double> hw_commands_;
    std::vector<double> hw_states_;
    
    std::string tcp_server_ip_;
    int tcp_server_port_;
    
    std::unique_ptr<TcpClient> tcp_client_;
    
    std::unique_ptr<std::thread> network_thread_;
    std::atomic<bool> network_thread_running_{false};
    
    std::mutex state_mutex_;
    std::vector<double> received_states_;  // данные от TCP сервера
    
    std::mutex command_mutex_;
    std::vector<double> pending_commands_; // данные для отправки на TCP сервер
    
    void networkLoop();
};

hardware_interface::CallbackReturn test_tcp_control::on_init(const hardware_interface::HardwareInfo& info)
{
    if(hardware_interface::SystemInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS)
    {
        return hardware_interface::CallbackReturn::ERROR;
    }
    
    size_t num_joints = info_.joints.size();
    if(num_joints == 0)
    {
        RCLCPP_ERROR(rclcpp::get_logger("test_tcp_control"), "No joints defined in URDF!");
        return hardware_interface::CallbackReturn::ERROR;
    }
    
    hw_commands_.resize(num_joints, 0.0);
    hw_states_.resize(num_joints, 0.0);
    received_states_.resize(num_joints, 0.0);
    pending_commands_.resize(num_joints, 0.0);
    
    auto it = info_.hardware_parameters.find("tcp_server_ip");
    if(it != info_.hardware_parameters.end())
    {
        tcp_server_ip_ = it->second;
    }
    else
    {
        RCLCPP_ERROR(rclcpp::get_logger("test_tcp_control"), "Missing 'tcp_server_ip' parameter!");
        return hardware_interface::CallbackReturn::ERROR;
    }
    
    it = info_.hardware_parameters.find("tcp_server_port");
    if(it != info_.hardware_parameters.end())
    {
        tcp_server_port_ = std::stoi(it->second);
    }
    else
    {
        RCLCPP_ERROR(rclcpp::get_logger("test_tcp_control"), "Missing 'tcp_server_port' parameter!");
        return hardware_interface::CallbackReturn::ERROR;
    }
    
    RCLCPP_INFO(rclcpp::get_logger("test_tcp_control"), "TCP config: %s:%d, joints: %zu", 
                tcp_server_ip_.c_str(), tcp_server_port_, num_joints);
    
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn test_tcp_control::on_configure(const rclcpp_lifecycle::State& /*previous_state*/)
{
    // Создаем TCP клиент
    tcp_client_ = std::make_unique<TcpClient>(tcp_server_ip_, tcp_server_port_);
    
    if(!tcp_client_->connect())
    {
        RCLCPP_ERROR(rclcpp::get_logger("test_tcp_control"), "Failed to connect to TCP server!");
        return hardware_interface::CallbackReturn::ERROR;
    }
    
    network_thread_running_ = true;
    network_thread_ = std::make_unique<std::thread>(&MyTcpHardware::networkLoop, this);
    
    RCLCPP_INFO(rclcpp::get_logger("test_tcp_control"), "TCP client thread started");
    
    return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type test_tcp_control::read(const rclcpp::Time& /*time*/, const rclcpp::Duration& /*period*/)
{
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        hw_states_ = received_states_;
    }
    
    // В реальном драйвере здесь могли бы быть проверки валидности данных
    
    return hardware_interface::return_type::OK;
}

hardware_interface::return_type test_tcp_control::write(const rclcpp::Time& /*time*/, const rclcpp::Duration& /*period*/)
{
    {
        std::lock_guard<std::mutex> lock(command_mutex_);
        pending_commands_ = hw_commands_;
    }
    
    return hardware_interface::return_type::OK;
}

void test_tcp_control::networkLoop()
{
    RCLCPP_INFO(rclcpp::get_logger("test_tcp_control"), "Network thread started");
    
    while(network_thread_running_ && tcp_client_->isConnected())
    {
        // 1. Отправляем команды на сервер
        std::vector<double> commands_to_send;
        {
            std::lock_guard<std::mutex> lock(command_mutex_);
            commands_to_send = pending_commands_;
        }
        
        if(!tcp_client_->send(commands_to_send))
        {
            RCLCPP_WARN(rclcpp::get_logger("test_tcp_control"), "Failed to send commands via TCP");
        }
        
        // 2. Получаем состояние с сервера
        std::vector<double> new_states;
        if(tcp_client_->receive(new_states))
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            received_states_ = new_states;
        }
        
        // Небольшая задержка для снижения нагрузки на сеть
        // (может быть настроена, но не должна мешать основному циклу)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    RCLCPP_INFO(rclcpp::get_logger("test_tcp_control"), "Network thread stopped");
}
