################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../micro_ros_stm32cubemx_utils/sample_main.c \
../micro_ros_stm32cubemx_utils/sample_main_embeddedrtps.c \
../micro_ros_stm32cubemx_utils/sample_main_udp.c 

C_DEPS += \
./micro_ros_stm32cubemx_utils/sample_main.d \
./micro_ros_stm32cubemx_utils/sample_main_embeddedrtps.d \
./micro_ros_stm32cubemx_utils/sample_main_udp.d 

OBJS += \
./micro_ros_stm32cubemx_utils/sample_main.o \
./micro_ros_stm32cubemx_utils/sample_main_embeddedrtps.o \
./micro_ros_stm32cubemx_utils/sample_main_udp.o 


# Each subdirectory must supply rules for building sources it contributes
micro_ros_stm32cubemx_utils/%.o micro_ros_stm32cubemx_utils/%.su micro_ros_stm32cubemx_utils/%.cyclo: ../micro_ros_stm32cubemx_utils/%.c micro_ros_stm32cubemx_utils/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../../Core/Inc -I../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-micro_ros_stm32cubemx_utils

clean-micro_ros_stm32cubemx_utils:
	-$(RM) ./micro_ros_stm32cubemx_utils/sample_main.cyclo ./micro_ros_stm32cubemx_utils/sample_main.d ./micro_ros_stm32cubemx_utils/sample_main.o ./micro_ros_stm32cubemx_utils/sample_main.su ./micro_ros_stm32cubemx_utils/sample_main_embeddedrtps.cyclo ./micro_ros_stm32cubemx_utils/sample_main_embeddedrtps.d ./micro_ros_stm32cubemx_utils/sample_main_embeddedrtps.o ./micro_ros_stm32cubemx_utils/sample_main_embeddedrtps.su ./micro_ros_stm32cubemx_utils/sample_main_udp.cyclo ./micro_ros_stm32cubemx_utils/sample_main_udp.d ./micro_ros_stm32cubemx_utils/sample_main_udp.o ./micro_ros_stm32cubemx_utils/sample_main_udp.su

.PHONY: clean-micro_ros_stm32cubemx_utils

