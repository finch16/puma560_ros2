#include "puma560_description/robot_hardware_interface.hpp"

#include <algorithm>
#include <sstream>

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
    // Sync commands with current states so robot doesn't jump on activation
    for (const auto& [name, descr] : joint_command_interfaces_)
    {
        set_command(name, get_state(name));
    }
    RCLCPP_INFO(get_logger(), "Activated successfully");
    return CallbackReturn::SUCCESS;
}

hardware_interface::return_type RobotHardwareInterface::read(
    const rclcpp::Time& /*time*/,
    const rclcpp::Duration& period)
{
    double dt = period.seconds();
    if (dt <= 0.0) dt = 0.01;

    std::ostringstream ss;
    ss << std::fixed;
    ss.precision(3);

    for (const auto& joint : info_.joints)
    {
        const std::string pos_state = joint.name + "/" + hardware_interface::HW_IF_POSITION;
        const std::string vel_state = joint.name + "/" + hardware_interface::HW_IF_VELOCITY;
        const std::string pos_cmd   = joint.name + "/" + hardware_interface::HW_IF_POSITION;

        double current_pos = get_state(pos_state);
        double cmd_pos     = get_command(pos_cmd);

        double error = cmd_pos - current_pos;
        double delta = std::clamp(error, -dt, dt);

        set_state(pos_state, current_pos + delta);
        set_state(vel_state, delta / dt);

        ss << "\n  " << joint.name
           << "  state=" << (current_pos + delta)
           << "  cmd=" << cmd_pos
           << "  err=" << error;
    }

    RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 500, "[read]%s", ss.str().c_str());

    return hardware_interface::return_type::OK;
}

hardware_interface::return_type RobotHardwareInterface::write(
    const rclcpp::Time& /*time*/,
    const rclcpp::Duration& /*period*/)
{
    // TODO: send commands to real hardware (TCP/serial/etc.)
    return hardware_interface::return_type::OK;
}

} // namespace puma560_description

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(puma560_description::RobotHardwareInterface, hardware_interface::SystemInterface)
