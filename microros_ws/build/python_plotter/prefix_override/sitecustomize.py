import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/ali/STM32CubeIDE/stm32_uros_ws/Panik/microros_ws/install/python_plotter'
