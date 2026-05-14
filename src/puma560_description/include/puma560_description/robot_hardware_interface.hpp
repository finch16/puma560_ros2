#pragma once

#include <string>
#include <vector>
#include <memory>
#include <array>

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "rclcpp/rclcpp.hpp"

namespace puma560_description
{

class RobotHardwareInterface : public hardware_interface::SystemInterface
{
public:
    hardware_interface::CallbackReturn on_init(
        const hardware_interface::HardwareComponentInterfaceParams& params) override;
    
    hardware_interface::CallbackReturn on_activate(
        const rclcpp_lifecycle::State& previous_state) override;
    
    hardware_interface::return_type read(
        const rclcpp::Time& time,
        const rclcpp::Duration& period) override;
    
    hardware_interface::return_type write(
        const rclcpp::Time& time,
        const rclcpp::Duration& period) override;
    
    std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
    std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

private:
    static constexpr int num_joints = 6;
    
    std::array<double, num_joints> command_positions{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    std::array<double, num_joints> state_positions{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    std::array<double, num_joints> state_velocity{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    std::array<std::string, num_joints> joint_names;
};

} // namespace
