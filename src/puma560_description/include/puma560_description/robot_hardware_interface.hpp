#pragma once

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"

namespace puma560_description
{

class RobotHardwareInterface : public hardware_interface::SystemInterface
{
public:
    hardware_interface::CallbackReturn on_init(
        const hardware_interface::HardwareComponentInterfaceParams& params) override;

    hardware_interface::CallbackReturn on_configure(
        const rclcpp_lifecycle::State& previous_state) override;

    hardware_interface::CallbackReturn on_activate(
        const rclcpp_lifecycle::State& previous_state) override;

    hardware_interface::return_type read(
        const rclcpp::Time& time,
        const rclcpp::Duration& period) override;

    hardware_interface::return_type write(
        const rclcpp::Time& time,
        const rclcpp::Duration& period) override;
};

} // namespace
