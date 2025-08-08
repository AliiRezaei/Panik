# Panik


TODO

1. Make Clean Workspace
    - Review Tasks Stack Size
    - Proper Partitioning
    - Some Docs and Comments
2. Add Control Necessary Files (Done)
    - PID Controller Source and Header
3. Add Low-Pass Filter Necessary Files
    - Low-Pass Filter Source and Header
    - Apply to the Sensors Readded Data
4. Timers Initialization (Done)
5. Control Task Creation (Done)
6. Set Subscriber Task Received Data as Desireds (Done)
7. Implement Closed-Loop Command Following
    - Single Motor Test (Done)
    - Multi Motor Test (Done)
    - Notice Publishing the Torque Commands on Efforts


Install Joy Package
```
sudo apt install ros-jazzy-joy
```

Device Info
```
cat /proc/bus/input/devices | grep -A6 -i sidewinder
```

Run joy_node
```
ros2 run joy joy_node --ros-args -p dev:=/dev/input/js0
```

Echo joy Data
```
ros2 topic echo /joy
```

Run micro_ros_agent
```
ros2 run micro_ros_agent micro_ros_agent serial -b 921600 --dev /dev/ttyUSB0
```

Publish Desired States
```
ros2 topic pub --once /joint_desireds sensor_msgs/msg/JointState "{name: ['joint1, joint2, joint3'], position: [0.0, 0.0, 0.0], velocity: [0.0, 0.0, 0.0], effort: [0.0, 0.0, 0.0]}"
```

Echo joint_states Data
```
ros2 topic echo /joint_states
```


