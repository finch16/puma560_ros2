#pragma once

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tcp_client/client.hpp"

//#include <tuple>
//#include <string>
//#include <unordered_map>

namespace puma560_description
{

class RobotHardwareInterface : public hardware_interface::SystemInterface
{
    /*std::unordered_map<std::string, std::pair<float, float>> convert =
    {
        {"joint1", {-a, a}},
        {"joint2", {-b, c}},
        {"joint3", {-b, c}},
        {"joint4", {-d, e}},
        {"joint5", {-f, f}},
        {"joint6", {-a, a}}
    };*/

    std::array<std::pair<float, float>, 6> convert =
    {
        std::pair{-3.14159265f, 3.14159265f},
        std::pair{-0.85521133f, 3.99680399f},
        std::pair{-0.85521133f, 3.99680399f},
        std::pair{-3.31612558f, 2.18166157f},
        std::pair{-1.74532925f, 1.74532925f},
        std::pair{-3.14159265f, 3.14159265f}
    };

    net::tcp_client client;

public:
    hardware_interface::CallbackReturn on_init(
        const hardware_interface::HardwareComponentInterfaceParams& params) override;

    hardware_interface::CallbackReturn on_configure(
        const rclcpp_lifecycle::State& previous_state) override;

    hardware_interface::CallbackReturn on_activate(
        const rclcpp_lifecycle::State& previous_state) override;
    
    hardware_interface::CallbackReturn on_shutdown(
        const rclcpp_lifecycle::State& previous_state) override;

    hardware_interface::return_type read(
        const rclcpp::Time& time,
        const rclcpp::Duration& period) override;

    hardware_interface::return_type write(
        const rclcpp::Time& time,
        const rclcpp::Duration& period) override;
};

} // namespace
