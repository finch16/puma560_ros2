#include "puma560_description/robot_hardware_interface.hpp"
#include "tcp_client/client.hpp"
#include <algorithm>
#include <sstream>
#include <array>

namespace puma560_description
{

hardware_interface::CallbackReturn RobotHardwareInterface::on_init(
    const hardware_interface::HardwareComponentInterfaceParams& params)
{
    if (hardware_interface::SystemInterface::on_init(params) != CallbackReturn::SUCCESS)
    {
        return CallbackReturn::ERROR;
    }

    for (const auto& joint : info_.joints)
    {
        if (joint.command_interfaces.size() != 1 ||
            joint.command_interfaces[0].name != hardware_interface::HW_IF_POSITION)
        {
            RCLCPP_FATAL(get_logger(),
                "Joint '%s': expected 1 position command interface, got %zu",
                joint.name.c_str(), joint.command_interfaces.size());
            return CallbackReturn::ERROR;
        }

        if (joint.state_interfaces.size() != 2)
        {
            RCLCPP_FATAL(get_logger(),
                "Joint '%s': expected 2 state interfaces (position + velocity), got %zu",
                joint.name.c_str(), joint.state_interfaces.size());
            return CallbackReturn::ERROR;
        }
    }

    return CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn RobotHardwareInterface::on_configure(
    const rclcpp_lifecycle::State& /*previous_state*/)
{
    for (const auto& [name, descr] : joint_state_interfaces_)
    {
        set_state(name, 0.0);
    }
    for (const auto& [name, descr] : joint_command_interfaces_)
    {
        set_command(name, 0.0);
    }
    RCLCPP_INFO(get_logger(), "Configured successfully");
    return CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn RobotHardwareInterface::on_activate(
    const rclcpp_lifecycle::State& /*previous_state*/)
{
    for (const auto& [name, descr] : joint_command_interfaces_)
    {
        set_command(name, get_state(name));
    }
    
    client.start("192.168.1.10", "1000");
    
    RCLCPP_INFO(get_logger(), "Activated successfully");
    return CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn RobotHardwareInterface::on_shutdown(
    const rclcpp_lifecycle::State& /*previous_state*/)
{
    client.stop();
    
    RCLCPP_INFO(get_logger(), "Shutdowned successfully");
    return CallbackReturn::SUCCESS;
}

hardware_interface::return_type RobotHardwareInterface::read(
    const rclcpp::Time& /*time*/,
    const rclcpp::Duration& period)
{
    double dt = period.seconds();
    if (dt <= 0.0) dt = 0.01;

    auto data = client.get();

    for(int it = 0; it < 6; it++)
    {
        const std::string pos_state = std::format("joint{}/{}", it + 1, hardware_interface::HW_IF_POSITION);
        const std::string vel_state = std::format("joint{}/{}", it + 1, hardware_interface::HW_IF_VELOCITY);

        double current_pos = get_state(pos_state);
        double pos = data[it] * (convert[it].second - convert[it].first) / 99.99 + convert[it].first;
        
        set_state(pos_state, pos);
        set_state(vel_state, (pos - current_pos) / dt);
    }

    return hardware_interface::return_type::OK;
}

hardware_interface::return_type RobotHardwareInterface::write(
    const rclcpp::Time& /*time*/,
    const rclcpp::Duration& /*period*/)
{
    std::array<float, 6> data = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    
    for(int it = 0; it < 6; it++)
    {
        const std::string pos_cmd = std::format("joint{}/{}", it + 1, hardware_interface::HW_IF_POSITION);

        data[it] = get_command(pos_cmd);
        data[it] = (get_command(pos_cmd) - convert[it].first) * 99.99 / (convert[it].second - convert[it].first);
    }

    client.send(data);
    return hardware_interface::return_type::OK;
}

} // namespace puma560_description

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(puma560_description::RobotHardwareInterface, hardware_interface::SystemInterface)
