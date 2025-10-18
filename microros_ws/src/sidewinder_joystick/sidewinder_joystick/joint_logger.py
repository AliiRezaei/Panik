#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
from rclpy.qos import QoSProfile, QoSReliabilityPolicy
from datetime import datetime
import os

class JointLogger(Node):
    def __init__(self):
        super().__init__('joint_logger')

        # Create a timestamped log file name
        now = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.log_filename = f"joint_log_{now}.csv"
        self.log_path = os.path.join(os.getcwd(), self.log_filename)

        # Open the log file and write the header
        self.log_file = open(self.log_path, 'w')
        self.log_file.write("timestamp,topic,name,position,velocity,effort\n")

        # Initialize subscribers
        qos_profile = QoSProfile(depth=10)
        qos_profile.reliability = QoSReliabilityPolicy.BEST_EFFORT
        self.states_sub = self.create_subscription(
            JointState, 'joint_states', self.states_callback, qos_profile)
        self.desired_sub = self.create_subscription(
            JointState, 'joint_desireds', self.desired_callback, qos_profile)

        self.get_logger().info(f"Logging joint data to {self.log_path}")

    def log_joint_state(self, topic_name: str, msg: JointState):
        timestamp = self.get_clock().now().to_msg()
        time_str = f"{timestamp.sec}.{timestamp.nanosec:09d}"
        for i, name in enumerate(msg.name):
            pos = msg.position[i] if i < len(msg.position) else ''
            vel = msg.velocity[i] if i < len(msg.velocity) else ''
            eff = msg.effort[i] if i < len(msg.effort) else ''
            self.log_file.write(f"{time_str},{topic_name},{name},{pos},{vel},{eff}\n")
        self.log_file.flush()

    def states_callback(self, msg: JointState):
        self.log_joint_state("states", msg)

    def desired_callback(self, msg: JointState):
        self.log_joint_state("desired", msg)

    def destroy_node(self):
        if self.log_file:
            self.log_file.close()
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = JointLogger()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

