import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np
from collections import deque

# Parameters
NUM_JOINTS = 3        # Update if your robot has a different number of joints
WINDOW_SIZE = 200     # How many samples to keep in the plot

class JointStatePlotter(Node):
    def __init__(self):
        super().__init__('joint_state_plotter')

        self.subscription = self.create_subscription(
            JointState,
            'joint_states',
            self.listener_callback,
            10)

        self.time_data = deque(maxlen=WINDOW_SIZE)
        self.joint_data = [deque(maxlen=WINDOW_SIZE) for _ in range(NUM_JOINTS)]
        self.start_time = self.get_clock().now().seconds_nanoseconds()[0]

        # Setup plot
        self.fig, self.ax = plt.subplots()
        self.lines = [
            self.ax.plot([], [], label=f'Joint {i}')[0] for i in range(NUM_JOINTS)
        ]

        self.ax.set_title("Joint Positions")
        self.ax.set_xlabel("Time [s]")
        self.ax.set_ylabel("Position [rad or m]")
        self.ax.legend()
        self.ax.grid(True)

        self.ani = FuncAnimation(self.fig, self.update_plot, interval=50)
        plt.show(block=False)

    def listener_callback(self, msg: JointState):
        now = self.get_clock().now().seconds_nanoseconds()[0] - self.start_time
        self.time_data.append(now)

        for i in range(NUM_JOINTS):
            if i < len(msg.position):
                self.joint_data[i].append(msg.position[i])
            else:
                self.joint_data[i].append(0.0)  # default if missing

    def update_plot(self, frame):
        for i in range(NUM_JOINTS):
            self.lines[i].set_data(self.time_data, self.joint_data[i])

        self.ax.relim()
        self.ax.autoscale_view()

        return self.lines


def main(args=None):
    rclpy.init(args=args)
    plotter = JointStatePlotter()

    try:
        while rclpy.ok():
            rclpy.spin_once(plotter, timeout_sec=0.01)
            plt.pause(0.01)
    except KeyboardInterrupt:
        pass

    plotter.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
