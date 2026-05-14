#include "puma560_description/robot_hardware_interface.hpp"

namespace puma560_description
{

hardware_interface::CallbackReturn RobotHardwareInterface::on_init(
    const hardware_interface::HardwareComponentInterfaceParams& params)
{
    if(hardware_interface::SystemInterface::on_init(params) != CallbackReturn::SUCCESS)
    {
        RCLCPP_ERROR(get_node()->get_logger(), "Not init RobotHardwareInterface");
        return CallbackReturn::ERROR;
    }

    const auto& joint_info = get_hardware_info().joints;

    if(joint_info.size() < num_joints)
    {
        RCLCPP_ERROR(get_node()->get_logger(), "There are fewer (6) joints in the configuration than necessary.");
        return CallbackReturn::ERROR;
    }

    for(size_t i = 0; i < num_joints; ++i)
    {
        joint_names[i] = joint_info[i].name;
    }
    
    return CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn RobotHardwareInterface::on_activate(
    const rclcpp_lifecycle::State& /*previous_state*/)
{
    return CallbackReturn::SUCCESS;
}

hardware_interface::return_type RobotHardwareInterface::read(
    const rclcpp::Time& /*time*/,
    const rclcpp::Duration& period)
{
    double dt = period.seconds();
    if(dt <= 0.0) dt = 0.01;
    for(size_t i = 0; i < num_joints; ++i)
    {
        double error = command_positions[i] - state_positions[i];
        double delta = std::clamp(error, -dt, dt);
        state_positions[i] += delta;
        state_velocity[i] = delta / dt;
    }
    return hardware_interface::return_type::OK;;
}

hardware_interface::return_type RobotHardwareInterface::write(
    const rclcpp::Time& /*time*/,
    const rclcpp::Duration& /*period*/)
{
    return hardware_interface::return_type::OK;
}

std::vector<hardware_interface::StateInterface> RobotHardwareInterface::export_state_interfaces()
{
    std::vector<hardware_interface::StateInterface> state_interfaces;
    for (size_t i = 0; i < num_joints; ++i)
    {
        state_interfaces.emplace_back(joint_names[i], hardware_interface::HW_IF_POSITION, &state_positions[i]);
        state_interfaces.emplace_back(joint_names[i], hardware_interface::HW_IF_VELOCITY, &state_velocity[i]);
    }
    return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> RobotHardwareInterface::export_command_interfaces()
{
    
    std::vector<hardware_interface::CommandInterface> command_interfaces;
    for (size_t i = 0; i < num_joints; ++i)
    {
        command_interfaces.emplace_back(joint_names[i], hardware_interface::HW_IF_POSITION, &command_positions[i]);
    }
    return command_interfaces;

}

} // namespace

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(puma560_description::RobotHardwareInterface, hardware_interface::SystemInterface)
