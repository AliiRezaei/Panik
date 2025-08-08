# joy_to_joint.py
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy, JointState

class JoyToJoint(Node):
    def __init__(self):
        super().__init__('joy_to_joint')
        self.sub = self.create_subscription(Joy, 'joy', self.cb, 10)
        self.pub = self.create_publisher(JointState, '/desired_states', 10)
        self.joint_names = ['wrist_joint_0','wrist_joint_1','wrist_joint_2']
        # tune these scales for comfortable motion range
        self.scales = [0.25, 0.25, 0.25]

    def cb(self, msg: Joy):
        # map joystick axes 0,1,2 to joints (adjust indices as needed)
        axes = msg.axes
        j = JointState()
        j.header.stamp = self.get_clock().now().to_msg()
        j.name = self.joint_names
        j.position = [
            axes[0] * self.scales[2],
            axes[1] * self.scales[1],
            axes[2] * self.scales[0]
        ]
        self.pub.publish(j)

def main(args=None):
    rclpy.init(args=args)
    n = JoyToJoint()
    rclpy.spin(n)
    n.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
