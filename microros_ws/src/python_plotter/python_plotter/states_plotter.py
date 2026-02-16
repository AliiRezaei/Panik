#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy
from sensor_msgs.msg import JointState
import numpy as np
import pyqtgraph as pg
from pyqtgraph.Qt import QtCore, QtWidgets
import threading
from collections import deque
import time

class JointPlotter(Node):
    def __init__(self):
        super().__init__('joint_state_plotter')
        
        # Set compatible QoS profile
        qos_profile = QoSProfile(
            depth=10,
            reliability=QoSReliabilityPolicy.BEST_EFFORT
        )
        
        self.subscription = self.create_subscription(
            JointState,
            '/joint_states',
            self.joint_state_callback,
            qos_profile
        )
        
        # Graphics configuration
        pg.setConfigOptions(
            useOpenGL=True,         # GPU acceleration
            antialias=False,        # Disable anti-aliasing for performance
            background='w',         # White background
            foreground='k'          # Black foreground
        )
        
        self.win = pg.GraphicsLayoutWidget(title="Real-time Joint Positions")
        self.win.resize(1200, 800)
        self.plot = self.win.addPlot()
        
        # Plot styling
        self.plot.showGrid(x=True, y=True, alpha=0.3)
        self.plot.setLabel('left', 'Position (rad)', color='#333')
        self.plot.setLabel('bottom', 'Time (s)', color='#333')
        self.plot.setTitle("Joint Position Tracking", color='#333', size='12pt')
        self.plot.addLegend(offset=(-10, 10))
        self.plot.setMouseEnabled(x=False, y=False)  # Disable pan/zoom for performance
        
        # Data storage
        self.joint_data = {}
        self.timestamps = deque(maxlen=500)  # Reduced buffer size
        self.start_time = time.time()
        self.last_update_time = 0
        self.new_data_flag = False
        self.lock = threading.Lock()
        self.curves = {}
        
        # Color palette for joints
        self.colors = [
            '#FF0000',  # Red
            '#00AA00',  # Green
            '#0000FF',  # Blue
            '#FF00FF',  # Magenta
            '#00FFFF',  # Cyan
            '#FFA500'   # Orange
        ]
        
        # Update timer (20Hz)
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(50)  # 50ms = 20Hz

    def joint_state_callback(self, msg):
        with self.lock:
            current_time = time.time() - self.start_time
            
            # Update timestamps only if we have new data
            if not self.timestamps or current_time > self.timestamps[-1]:
                self.timestamps.append(current_time)
            
            # Process joints
            for i, name in enumerate(msg.name[:6]):  # Limit to first 6 joints
                if name not in self.joint_data:
                    self.joint_data[name] = deque(maxlen=500)
                    self.joint_data[name].extend([0.0] * 500)
                    
                    # Create curve with unique color
                    color = self.colors[len(self.curves) % len(self.colors)]
                    self.curves[name] = self.plot.plot(
                        pen=pg.mkPen(color, width=2.5),
                        name=name,
                        antialias=False
                    )
                
                # Store position data
                if i < len(msg.position):
                    self.joint_data[name].append(msg.position[i])
            
            self.new_data_flag = True

    def update_plot(self):
        if not self.new_data_flag:
            return
            
        with self.lock:
            # Only update if we have new data
            current_time = self.timestamps[-1] if self.timestamps else 0
            
            # Update each curve
            for name, curve in self.curves.items():
                if len(self.timestamps) == len(self.joint_data[name]):
                    # Convert to numpy arrays for efficiency
                    x = np.array(self.timestamps)
                    y = np.array(self.joint_data[name])
                    curve.setData(x, y)
            
            # Update view range (show last 5 seconds)
            if self.timestamps:
                self.plot.setXRange(max(0, current_time - 5), current_time + 0.2)
                self.plot.enableAutoRange('y', 0.95)  # Auto Y with 5% margin
            
            self.new_data_flag = False

def main(args=None):
    rclpy.init(args=args)
    
    # Create Qt application
    app = QtWidgets.QApplication([])
    
    # Create ROS2 node
    plotter = JointPlotter()
    
    # Start ROS node in separate thread
    ros_thread = threading.Thread(target=rclpy.spin, args=(plotter,))
    ros_thread.daemon = True
    ros_thread.start()
    
    # Start Qt application
    plotter.win.show()
    app.exec_()
    
    # Cleanup
    plotter.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()