//
// Created by farukbaykara on 04.05.2023.
//
#include "ntrip_client_ros/ntrip_client_ros.hpp"


int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<NtripClientRos>());
  rclcpp::shutdown();
  return 0;
}

