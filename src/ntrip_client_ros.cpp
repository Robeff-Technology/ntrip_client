//
// Created by farukbaykara on 04.05.2023.
//

#include "ntrip_client_ros/ntrip_client_ros.hpp"

using namespace std;
using namespace chrono_literals;
using rcl_interfaces::msg::ParameterDescriptor;
using rclcpp::ParameterValue;
using namespace std::placeholders;
using PubAllocT = rclcpp::PublisherOptionsWithAllocator<std::allocator<void>>;
using SubAllocT = rclcpp::SubscriptionOptionsWithAllocator<std::allocator<void>>;


NtripClientRos::NtripClientRos():Node("ntrip_client_ros"),
  m_serial_port_{this->declare_parameter(
                       "serial_port",
                       ParameterValue{"/dev/ttyUSB0"},
                       ParameterDescriptor{})
                   .get<std::string>()},
  m_serial_baud_rate_{this->declare_parameter(
                       "serial_port_baud_rate",
                       ParameterValue{460800},
                       ParameterDescriptor{})
                   .get<long>()},
  m_ntrip_ip_{this->declare_parameter(
                       "ntrip_ip",
                       ParameterValue{"212.156.70.42"},
                       ParameterDescriptor{})
                   .get<std::string>()},
  m_ntrip_password_{this->declare_parameter(
                    "ntrip_password",
                    ParameterValue{"GzMSQg"},
                    ParameterDescriptor{})
                .get<std::string>()},
  m_ntrip_username_{this->declare_parameter(
                    "ntrip_user_name",
                    ParameterValue{"K0734151301"},
                    ParameterDescriptor{})
                .get<std::string>()},
  m_ntrip_mountpoint_{this->declare_parameter(
                    "ntrip_mount_point",
                    ParameterValue{"VRSRTCM31"},
                    ParameterDescriptor{})
                .get<std::string>()},
  m_ntrip_port_{static_cast<int>(this->declare_parameter(
                    "ntrip_port",
                    ParameterValue{2101},
                    ParameterDescriptor{})
                .get<int>())},
  m_publish_ros_rtcm_active_{this->declare_parameter(
                    "publish_ros_rtcm_active",
                    ParameterValue{true},
                    ParameterDescriptor{})
                .get<bool>()},
  m_ntrip_location_lat{this->declare_parameter(
                    "ntrip_location_lat",
                    ParameterValue{0.0},
                    ParameterDescriptor{})
                    .get<double>()},
  m_ntrip_location_lon{this->declare_parameter(
                    "ntrip_location_lon",
                    ParameterValue{0.0},
                    ParameterDescriptor{})
                    .get<double>()},
  m_nav_sat_fix_topic{this->declare_parameter(
                    "nav_sat_fix_topic",
                    ParameterValue{"/fix"},
                    ParameterDescriptor{})
                    .get<std::string>()},
  m_debug_{this->declare_parameter(
                    "debug",
                    ParameterValue{true},
                    ParameterDescriptor{})
                .get<bool>()},
  m_rtcm_topic_{this->declare_parameter(
                            "rtcm_topic",
                            ParameterValue{"/ntrip/rtcm"},
                            ParameterDescriptor{})
                        .get<std::string>()},
  m_publish_port_rtcm_active_{this->declare_parameter(
                                   "publish_port_rtcm_active",
                                   ParameterValue{false},
                                   ParameterDescriptor{})
                               .get<bool>()},
  m_set_ntrip_location_from_yaml_{this->declare_parameter(
                                   "set_ntrip_location_from_yaml",
                                   ParameterValue{false},
                                   ParameterDescriptor{})
                               .get<bool>()}

{
  RCLCPP_INFO(this->get_logger(),"NTRIP Client ROS created");
  ReadParameters();

  pub_rtcm_ = create_publisher<mavros_msgs::msg::RTCM>(m_rtcm_topic_,rclcpp::QoS{1},PubAllocT{});

  if(m_set_ntrip_location_from_yaml_ && !is_connected){
    ConnectNtripClient();
  }
  else{
    sub_nav_sat_fix_ = create_subscription<sensor_msgs::msg::NavSatFix>(
      m_nav_sat_fix_topic, rclcpp::QoS{1}, std::bind(&NtripClientRos::NavSatCB, this, _1));
    RCLCPP_INFO(this->get_logger(), "Waiting for %s to be Published",m_nav_sat_fix_topic.c_str());
  }

  if(m_publish_port_rtcm_active_) {
    try {
      m_serial_boost_.open(m_serial_port_, m_serial_baud_rate_);
    } catch (boost::system::system_error & e) {
      RCLCPP_ERROR(
        this->get_logger(), "\033[1;31m RTK Serial port could not be opened: \033[0m  %s",
        e.what());
    }

    if (m_serial_boost_.isOpen() == false) {
      RCLCPP_INFO(
        rclcpp::get_logger("rclcpp"),
        "\033[1;31m RTK Serial port %s could not be opened, plug and relaunch the package\033[0m\n",
        m_serial_port_.c_str());
    } else if (m_serial_boost_.isOpen() == true) {
      RCLCPP_INFO(
        rclcpp::get_logger("rclcpp"), "\033[1;32m  RTK Serial port %s opened!!!\033[0m\n",
        m_serial_port_.c_str());
      // setting the serial receive callback function
      // m_serial_boost_.setCallback(bind(&NtripClientRos::serial_receive_callback, this, _1, _2));
    }
  }
}

void NtripClientRos::ConnectNtripClient(){
  int i = 2000;
  while(i>0 && rclcpp::ok()){
    if(NtripClientStart()){
      RCLCPP_INFO(this->get_logger(), "\033[1;32m NTRIP client connected \033[0m");
      is_connected=true;
      break;
    }
    else{
      RCLCPP_INFO(this->get_logger(), "\033[1;31m NTRIP client cannot connected \033[0m");
    }
    i--;
    //rclcpp::sleep_for(std::chrono::nanoseconds(std::pow(10,9)));
    sleep(1);
  }
}

void NtripClientRos::NavSatCB(const sensor_msgs::msg::NavSatFix::SharedPtr msg){
  m_ntrip_location_lat = msg->latitude;
  m_ntrip_location_lon = msg->longitude;
  if(!is_connected){
    RCLCPP_INFO(this->get_logger(), "\033[1;34m NavSatFix Topic:\033[0m %s",m_nav_sat_fix_topic);
    RCLCPP_INFO(this->get_logger(), "\033[1;34m NTRIP Location Lat:\033[0m %f",m_ntrip_location_lat);
    RCLCPP_INFO(this->get_logger(), "\033[1;34m NTRIP Location Lon:\033[0m %f",m_ntrip_location_lon);
    ConnectNtripClient();
  }
  if(is_connected)
    sub_nav_sat_fix_.reset();
}

bool NtripClientRos::NtripClientStart()
{
  m_ntripClient_.Init(m_ntrip_ip_,m_ntrip_port_,m_ntrip_username_,m_ntrip_password_,m_ntrip_mountpoint_);
  
  m_ntripClient_.set_location(m_ntrip_location_lat,m_ntrip_location_lon);
  m_ntripClient_.set_report_interval(0.001);

  m_ntripClient_.OnReceived([this](const char *buffer, int size)
                         {
                              if(size>0) {
                                if (m_publish_ros_rtcm_active_){
                                  std::vector<uint8_t> data;
                                  std::string s = buffer;
                                  //RCLCPP_INFO(this->get_logger(),"%s",s.c_str());
                                  for (int i = 0; i < size; i++)
                                    data.push_back(static_cast<uint8_t>(buffer[i]));

                                  m_msg_rtcm_.header.frame_id = "rtk";
                                  m_msg_rtcm_.header.stamp.set__sec(
                                    static_cast<int32_t>(this->get_clock()->now().seconds()));
                                  m_msg_rtcm_.header.stamp.set__nanosec(
                                    static_cast<uint32_t>(this->get_clock()->now().nanoseconds()));
                                  m_msg_rtcm_.set__data(data);

                                  pub_rtcm_->publish(m_msg_rtcm_);
                                  if (m_debug_) {
                                    RCLCPP_INFO(this->get_logger(), "NTRIP Topic Send Data size: %lu", data.size());
                                    RCLCPP_INFO(this->get_logger(), "NTRIP Topic Send Status: %d", m_ntripClient_.service_is_running());
                                  }

                                }
                                if(m_publish_port_rtcm_active_ && m_serial_boost_.isOpen()){
                                  try{
                                    m_serial_boost_.write(buffer,size);
                                    if (m_debug_) {
                                      RCLCPP_INFO(this->get_logger(), "NTRIP Port Send Data size: %d", size);
                                      RCLCPP_INFO(this->get_logger(), "NTRIP Port Send Status: %d", m_ntripClient_.service_is_running());
                                    }
                                  }catch(boost::system::system_error & e){
                                    RCLCPP_ERROR(this->get_logger(),"Cannot write to port: %s",e.what());
                                  }
                                }


                              }
                         });


  return m_ntripClient_.Run();

}

void NtripClientRos::ReadParameters()
{
  RCLCPP_INFO(this->get_logger(), "\033[1;34m Parameters\033[0m");
  RCLCPP_INFO(this->get_logger(), "\033[1;34m Serial Port Name:\033[0m %s",m_serial_port_.c_str());
  RCLCPP_INFO(this->get_logger(), "\033[1;34m NTRIP Server ID:\033[0m %s",m_ntrip_ip_.c_str());
  RCLCPP_INFO(this->get_logger(), "\033[1;34m NTRIP UserName:\033[0m %s",m_ntrip_username_.c_str());
  RCLCPP_INFO(this->get_logger(), "\033[1;34m NTRIP Password:\033[0m %s",m_ntrip_password_.c_str());
  RCLCPP_INFO(this->get_logger(), "\033[1;34m NTRIP MountPoint:\033[0m %s",m_ntrip_mountpoint_.c_str());
  RCLCPP_INFO(this->get_logger(), "\033[1;34m NTRIP Port:\033[0m %d",m_ntrip_port_);
  RCLCPP_INFO(this->get_logger(), "\033[1;34m Set NTRIP Location Manually:\033[0m %B",m_set_ntrip_location_from_yaml_);
  if(m_set_ntrip_location_from_yaml_){
    RCLCPP_INFO(this->get_logger(), "\033[1;34m NavSatFix Topic:\033[0m %s",m_nav_sat_fix_topic);
    RCLCPP_INFO(this->get_logger(), "\033[1;34m NTRIP Location Lat:\033[0m %f",m_ntrip_location_lat);
    RCLCPP_INFO(this->get_logger(), "\033[1;34m NTRIP Location Lon:\033[0m %f",m_ntrip_location_lon);
  }
  RCLCPP_INFO(this->get_logger(), "\033[1;34m Publish RTCM Msg via ROS:\033[0m %B",m_publish_ros_rtcm_active_);
  RCLCPP_INFO(this->get_logger(), "\033[1;34m Debug:\033[0m %B",m_debug_);
  RCLCPP_INFO(this->get_logger(), "\033[1;34m -----------------------------------------------------");
}
