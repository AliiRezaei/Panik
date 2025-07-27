# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/ali/STM32CubeIDE/stm32_uros_ws/Panik/microros_ws/src/uros/micro-ROS-Agent/micro_ros_agent"
  "/home/ali/STM32CubeIDE/stm32_uros_ws/Panik/microros_ws/build/micro_ros_agent"
  "/home/ali/STM32CubeIDE/stm32_uros_ws/Panik/microros_ws/build/micro_ros_agent/micro_ros_agent-prefix"
  "/home/ali/STM32CubeIDE/stm32_uros_ws/Panik/microros_ws/build/micro_ros_agent/micro_ros_agent-prefix/tmp"
  "/home/ali/STM32CubeIDE/stm32_uros_ws/Panik/microros_ws/build/micro_ros_agent/micro_ros_agent-prefix/src/micro_ros_agent-stamp"
  "/home/ali/STM32CubeIDE/stm32_uros_ws/Panik/microros_ws/build/micro_ros_agent/micro_ros_agent-prefix/src"
  "/home/ali/STM32CubeIDE/stm32_uros_ws/Panik/microros_ws/build/micro_ros_agent/micro_ros_agent-prefix/src/micro_ros_agent-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ali/STM32CubeIDE/stm32_uros_ws/Panik/microros_ws/build/micro_ros_agent/micro_ros_agent-prefix/src/micro_ros_agent-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ali/STM32CubeIDE/stm32_uros_ws/Panik/microros_ws/build/micro_ros_agent/micro_ros_agent-prefix/src/micro_ros_agent-stamp${cfgdir}") # cfgdir has leading slash
endif()
