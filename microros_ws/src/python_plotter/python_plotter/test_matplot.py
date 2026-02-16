import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import datetime

# Data containers
x_data = []
y_data = []

# Create the figure and line object
fig, ax = plt.subplots()
line, = ax.plot([], [], 'b-')
ax.set_xlim(0, 10)
ax.set_ylim(-1.5, 1.5)
ax.set_title("Live Sine Wave")
ax.set_xlabel("Time (s)")
ax.set_ylabel("Amplitude")

# Capture start time
start_time = datetime.datetime.now().timestamp()

def update(frame):
    current_time = datetime.datetime.now().timestamp() - start_time
    x_data.append(current_time)
    y_data.append(np.sin(2 * np.pi * 0.5 * current_time))  # 0.5 Hz sine wave

    # Keep last 10 seconds of data
    while x_data and (x_data[-1] - x_data[0]) > 10:
        x_data.pop(0)
        y_data.pop(0)

    line.set_data(x_data, y_data)
    ax.set_xlim(max(0, x_data[0]), x_data[-1] + 0.1)
    return line,

# Start animation
ani = FuncAnimation(fig, update, interval=100)
plt.tight_layout()
plt.show()
