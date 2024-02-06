cmake_minimum_required(VERSION 3.8)
project(ntrip_client)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(include)
# find dependencies
find_package(ament_cmake_auto REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(mavros_msgs REQUIRED)
find_package(nmea_msgs REQUIRED)

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

set(NTRIP_CLIENT_ROS_SRC
        src/ntrip_client_ros.cpp
        src/ntrip_client_ros_node.cpp)

set(NTRIP_CLIENT_ROS_HEADER
        include/ntrip_client_ros/ntrip_client_ros.hpp)


set(NTRIP_LIB_SRC
        src/ntrip_lib/ntrip_client.cpp
        src/ntrip_lib/ntrip_util.cpp)

set(NTRIP_LIB_HEADER
        include/ntrip/cmake_definition.h
        include/ntrip/ntrip_client.h
        include/ntrip/ntrip_util.h
        include/ntrip/thread_raii.h)

set(ASYNC_SERIAL_SRC
        src/AsyncSerial.cpp)

set(ASYNC_SERIAL_HEADER
        include/AsyncSerial.h)

add_executable(ntrip_client_exe
        ${NTRIP_CLIENT_ROS_SRC}
        ${NTRIP_CLIENT_ROS_HEADER}
        ${NTRIP_LIB_SRC}
        ${NTRIP_LIB_HEADER}
        ${ASYNC_SERIAL_SRC}
        ${ASYNC_SERIAL_HEADER}
        )

ament_target_dependencies(ntrip_client_exe
        rclcpp
        mavros_msgs
        nmea_msgs
        )

install(TARGETS
		ntrip_client_exe
        DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch
        DESTINATION share/${PROJECT_NAME}
        )

install(DIRECTORY config
        DESTINATION share/${PROJECT_NAME}
        )



if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
