//
// Created by farukbaykara on 04.05.2023.
//

#ifndef NTRIP_CLIENT_ROS_NTRIP_CLIENT_ROS_H
#define NTRIP_CLIENT_ROS_NTRIP_CLIENT_ROS_H


#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <vector>

#include <ntrip/ntrip_client.h>

#include <rclcpp/rclcpp.hpp>
#include "nmea_msgs/msg/gpgga.h"
#include "mavros_msgs/msg/rtcm.hpp"
#include "sensor_msgs/msg/nav_sat_fix.hpp"

#include "AsyncSerial.h"

class NtripClientRos: public rclcpp::Node
{
public:

  NtripClientRos();

  ~NtripClientRos() override {
    m_serial_boost_.close();
  }

private:

  bool NtripClientStart();

  void ReadParameters();

  void NavSatCB(const sensor_msgs::msg::NavSatFix::SharedPtr msg);

  void ConnectNtripClient();

  //Parameters
  std::string m_serial_port_;
  long m_serial_baud_rate_;
  std::string m_ntrip_ip_;
  std::string m_ntrip_password_;
  std::string m_ntrip_username_;
  std::string m_ntrip_mountpoint_;
  int m_ntrip_port_;
  bool m_publish_ros_rtcm_active_;
  bool m_debug_;
  double m_ntrip_location_lat;
  double m_ntrip_location_lon;
  std::string m_rtcm_topic_;
  bool m_publish_port_rtcm_active_;
  
  bool m_set_ntrip_location_from_yaml_;
  std::string m_nav_sat_fix_topic;
  bool is_connected{false};


  CallbackAsyncSerial m_serial_boost_;

  libntrip::NtripClient m_ntripClient_;

  mavros_msgs::msg::RTCM m_msg_rtcm_;

  //Publisher
  rclcpp::Publisher<mavros_msgs::msg::RTCM>::SharedPtr pub_rtcm_;
  rclcpp::Subscription<sensor_msgs::msg::NavSatFix>::SharedPtr sub_nav_sat_fix_;



  
  uint8_t m_ntripStatus_;


};



#endif  // NTRIP_CLIENT_ROS_NTRIP_CLIENT_ROS_H
