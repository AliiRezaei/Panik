import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Joy, JointState
import time


class JoyToJoint(Node):
    def __init__(self):
        super().__init__('joy_to_joint')
        self.sub = self.create_subscription(Joy, 'joy', self.cb, 10)
        self.pub = self.create_publisher(JointState, '/joint_desireds', 10)
        self.joint_names = ['joint1',
                            'joint2',
                            'joint3']

        # motion range
        self.scales = [0.25, -0.5, -1.0]

        # High-gain observer parameters
        self.alpha1 = 6.0
        self.alpha2 = 5.0
        self.eps    = 0.1

        # Observer states
        # These are persistent and are NOT reset in cb()
        self.obs_pos = [0.0, 0.0, 0.0]
        self.obs_vel = [0.0, 0.0, 0.0]

        # Previous timestamp
        self.prev_time = time.monotonic()

    def cb(self, msg: Joy):
        # compute timestamp
        now = time.monotonic()
        Ts = now - self.prev_time

        if Ts < 0.0 or Ts > 0.5:
            Ts = 0.001

        # map joystick axes
        axes = msg.axes

        # scaled joystick positions
        ym = [
            axes[0] * self.scales[0],
            axes[1] * self.scales[1],
            axes[2] * self.scales[2]
        ]

        # High-gain observer
        for i in range(3):

            # observation error
            y_tilde = ym[i] - self.obs_pos[i]

            # correction terms
            e_pos = y_tilde * (
                (Ts * Ts * self.alpha2)
                / (2.0 * self.eps * self.eps)
                + (Ts * self.alpha1)
                / self.eps
            )

            e_vel = (
                Ts * self.alpha2 * y_tilde
                / (self.eps * self.eps)
            )

            # Store the new observer states directly
            new_pos = (
                self.obs_pos[i]
                + Ts * self.obs_vel[i]
                + e_pos
            )

            new_vel = (
                self.obs_vel[i]
                + e_vel
            )

            self.obs_pos[i] = new_pos
            self.obs_vel[i] = new_vel

        # update timestamp
        self.prev_time = now

        j = JointState()
        j.header.stamp = self.get_clock().now().to_msg()
        j.name = self.joint_names

        # Publish observer outputs
        j.position = self.obs_pos
        j.velocity = self.obs_vel

        j.effort = [
            0.0,
            0.0,
            0.0
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




