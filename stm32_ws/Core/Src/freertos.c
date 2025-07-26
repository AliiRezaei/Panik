/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <uxr/client/transport.h>
#include <rmw_microxrcedds_c/config.h>
#include <rmw_microros/rmw_microros.h>

#include <sensor_msgs/msg/joint_state.h>
#include <rosidl_runtime_c/string_functions.h>
#include <rosidl_runtime_c/primitives_sequence_functions.h>
#include <sensor_msgs/msg/detail/joint_state__functions.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_JOINTS (3U)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
rcl_node_t node;
rclc_support_t support;
rcl_allocator_t allocator;
rclc_executor_t executor;
PCA9548a_s PCA9548a;
sensor_msgs__msg__JointState joint_desired_msg;
sensor_msgs__msg__JointState joint_state_msg;
rcl_publisher_t publisher;
rcl_subscription_t subscriber;
bool microros_initialized = 0;
/* USER CODE END Variables */
/* Definitions for jointStatesPublisherTask */
osThreadId_t jointStatesPublisherTaskHandle;
uint32_t jointStatesPublisherTaskBuffer[ 1024 * 3 ];
osStaticThreadDef_t jointStatesPublisherTaskControlBlock;
const osThreadAttr_t jointStatesPublisherTask_attributes = {
		.name = "publisherTask",
		.cb_mem = &jointStatesPublisherTaskControlBlock,
		.cb_size = sizeof(jointStatesPublisherTaskControlBlock),
		.stack_mem = &jointStatesPublisherTaskBuffer[0],
		.stack_size = sizeof(jointStatesPublisherTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for jointDesiredSubscriberTask */
osThreadId_t jointDesiredSubscriberTaskHandle;
uint32_t jointDesiredSubscriberTaskBuffer[ 1024 * 3 ];
osStaticThreadDef_t jointDesiredSubscriberTaskControlBlock;
const osThreadAttr_t jointDesiredSubscriberTask_attributes = {
		.name = "subscriberTask",
		.cb_mem = &jointDesiredSubscriberTaskControlBlock,
		.cb_size = sizeof(jointDesiredSubscriberTaskControlBlock),
		.stack_mem = &jointDesiredSubscriberTaskBuffer[0],
		.stack_size = sizeof(jointDesiredSubscriberTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for jointStatesReadTask */
osThreadId_t jointStatesReadTaskHandle;
uint32_t jointStatesReadTaskBuffer[ 1024 * 1 ];
osStaticThreadDef_t jointStatesReadTaskControlBlock;
const osThreadAttr_t jointStatesReadTask_attributes = {
		.name = "jointStatesReadTask",
		.cb_mem = &jointStatesReadTaskControlBlock,
		.cb_size = sizeof(jointStatesReadTaskControlBlock),
		.stack_mem = &jointStatesReadTaskBuffer[0],
		.stack_size = sizeof(jointStatesReadTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for initTask */
osThreadId_t initTaskHandle;
uint32_t initTaskBuffer[ 1024 * 2 ];
osStaticThreadDef_t initTaskControlBlock;
const osThreadAttr_t initTask_attributes = {
		.name = "initTask",
		.cb_mem = &initTaskControlBlock,
		.cb_size = sizeof(initTaskControlBlock),
		.stack_mem = &initTaskBuffer[0],
		.stack_size = sizeof(initTaskBuffer),
		.priority = (osPriority_t) osPriorityHigh,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
bool cubemx_transport_open(struct uxrCustomTransport * transport);
bool cubemx_transport_close(struct uxrCustomTransport * transport);
size_t cubemx_transport_write(struct uxrCustomTransport* transport, const uint8_t * buf, size_t len, uint8_t * err);
size_t cubemx_transport_read(struct uxrCustomTransport* transport, uint8_t* buf, size_t len, int timeout, uint8_t* err);

void * microros_allocate(size_t size, void * state);
void microros_deallocate(void * pointer, void * state);
void * microros_reallocate(void * pointer, size_t size, void * state);
void * microros_zero_allocate(size_t number_of_elements, size_t size_of_element, void * state);
void JointStateCallback(const void *msgin);
/* USER CODE END FunctionPrototypes */

void JointStatesPublisherTask(void *argument);
void JointDesiredSubscriberTask(void *argument);
void JointStatesReadTask(void *argument);
void InitTask(void *argument);
void MicroROSInit(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */
	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */
	initTaskHandle = osThreadNew(InitTask, NULL, &initTask_attributes);

	/* Create the thread(s) */
	/* creation of jointStatesPublisherTask */
	jointStatesPublisherTaskHandle = osThreadNew(JointStatesPublisherTask, NULL, &jointStatesPublisherTask_attributes);

	/* creation of jointDesiredSubscriberTask */
	jointDesiredSubscriberTaskHandle = osThreadNew(JointDesiredSubscriberTask, NULL, &jointDesiredSubscriberTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* creation of jointStatesReadTask */
	jointStatesReadTaskHandle = osThreadNew(JointStatesReadTask, NULL, &jointStatesReadTask_attributes);

	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */

	/* USER CODE END RTOS_EVENTS */

}

/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
void JointStatesPublisherTask(void *argument)
{
	while (!microros_initialized) {
		osDelay(10);
	}

	// Initialize publisher
	rclc_publisher_init_best_effort(
			&publisher,
			&node,
			ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
			"joint_states");
//	rclc_publisher_init_default(
//			&publisher,
//			&node,
//			ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
//			"joint_states");

	sensor_msgs__msg__JointState__init(&joint_state_msg);

	// Initialize string and double sequences
	rosidl_runtime_c__String__Sequence__init(&joint_state_msg.name, NUM_JOINTS);
	rosidl_runtime_c__double__Sequence__init(&joint_state_msg.position, NUM_JOINTS);
	rosidl_runtime_c__double__Sequence__init(&joint_state_msg.velocity, NUM_JOINTS);
	rosidl_runtime_c__double__Sequence__init(&joint_state_msg.effort, NUM_JOINTS);

	// Assign joint names
	rosidl_runtime_c__String__assign(&joint_state_msg.name.data[0], "joint1");
	rosidl_runtime_c__String__assign(&joint_state_msg.name.data[1], "joint2");
	rosidl_runtime_c__String__assign(&joint_state_msg.name.data[2], "joint3");

	// Initialize joint values
	for (size_t i = 0; i < NUM_JOINTS; i++) {
		joint_state_msg.position.data[i] = 0.0;
		joint_state_msg.velocity.data[i] = 0.0;
		joint_state_msg.effort.data[i]   = 0.0;
	}

	for (;;)
	{
		// Timestamp
		joint_state_msg.header.stamp.sec = (int32_t)(xTaskGetTickCount() / 1000);
		joint_state_msg.header.stamp.nanosec = (xTaskGetTickCount() % 1000) * 1000000;

		// Simulate joint values
		for (size_t i = 0; i < NUM_JOINTS; i++) {
			joint_state_msg.position.data[i] = PCA9548a.position[i];
			joint_state_msg.velocity.data[i] = PCA9548a.velocity[i];
//			joint_state_msg.position.data[i] = joint_desired_msg.position.data[i];
//			joint_state_msg.velocity.data[i] = joint_desired_msg.velocity.data[i];
//			joint_state_msg.effort.data[i]   = joint_desired_msg.effort.data[i];;
		}

		// Publish the message
		rcl_ret_t ret = rcl_publish(&publisher, &joint_state_msg, NULL);
		if (ret != RCL_RET_OK) {
			printf("Failed to publish JointState message (line %d)\n", __LINE__);
		}

		GPIOB->ODR ^= GPIO_ODR_OD4;
		HAL_IWDG_Refresh(&hiwdg);
		osDelay(pdMS_TO_TICKS(1));
	}
}

void JointDesiredSubscriberTask(void *argument)
{
	while (!microros_initialized) {
		osDelay(10);
	}

	// Spin loop
	for (;;)
	{
		rclc_executor_spin_some(&executor, RCL_MS_TO_NS(1));  // check every 100ms
//		rclc_executor_fini(&executor);
		osDelay(pdMS_TO_TICKS(1));
	}
}

/* USER CODE BEGIN Header_PosReadTask */
/**
 * @brief  Function implementing the posReadTask thread.
 * @param  argument: Not used
 * @retval None
 */
void JointStatesReadTask(void *argument)
{
	for (size_t i = 0; i < NUM_JOINTS; i++) {
		PCA9548a.position[i]     = 0.0;
		PCA9548a.prevPosition[i] = 0.0;
		PCA9548a.velocity[i]     = 0.0;
		PCA9548a.prevTick[i]     = 0;
	}
	float derivativeT = 0.1, filterT = 0.01;
	for (;;)
	{
		for (size_t i = 0; i < NUM_JOINTS; i++) {
			pca9548a_GetStates(&PCA9548a, i, derivativeT, filterT);
		}
		osDelay(pdMS_TO_TICKS(10));
	}
}

void JointStateCallback(const void *msgin)
{
	const sensor_msgs__msg__JointState *msg = (const sensor_msgs__msg__JointState *)msgin;

	if (msg->position.size >= NUM_JOINTS) {
		for (size_t i = 0; i < NUM_JOINTS; i++) {
			joint_desired_msg.position.data[i] = msg->position.data[i];
			joint_desired_msg.velocity.data[i] = msg->velocity.data[i];
			joint_desired_msg.effort.data[i]   = msg->effort.data[i];
		}
	}
	GPIOB->ODR ^= GPIO_ODR_OD5;
}

void MicroROSInit(void)
{
	// Set up micro-ROS UART transport
	rmw_uros_set_custom_transport(
			true,
			(void *) &huart1,
			cubemx_transport_open,
			cubemx_transport_close,
			cubemx_transport_write,
			cubemx_transport_read);

	rcl_allocator_t freeRTOS_allocator = rcutils_get_zero_initialized_allocator();
	freeRTOS_allocator.allocate = microros_allocate;
	freeRTOS_allocator.deallocate = microros_deallocate;
	freeRTOS_allocator.reallocate = microros_reallocate;
	freeRTOS_allocator.zero_allocate =  microros_zero_allocate;

	if (!rcutils_set_default_allocator(&freeRTOS_allocator)) {
		printf("Error on default allocators (line %d)\n", __LINE__);
	}

	allocator = rcl_get_default_allocator();

	//create init_options
	rclc_support_init(&support, 0, NULL, &allocator);

	// create node
	rclc_node_init_default(&node, "cubemx_node", "", &support);

	// Initialize subscriber
	rclc_subscription_init_best_effort(
			&subscriber,
			&node,
			ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
			"joint_desireds");
//	rclc_subscription_init_default(
//			&subscriber,
//			&node,
//			ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
//			"joint_desireds");

	// Initialize local storage message
	rosidl_runtime_c__String__Sequence__init(&joint_desired_msg.name, NUM_JOINTS);
	rosidl_runtime_c__double__Sequence__init(&joint_desired_msg.position, NUM_JOINTS);
	rosidl_runtime_c__double__Sequence__init(&joint_desired_msg.velocity, NUM_JOINTS);
	rosidl_runtime_c__double__Sequence__init(&joint_desired_msg.effort, NUM_JOINTS);

	// Assign joint names
	rosidl_runtime_c__String__assign(&joint_desired_msg.name.data[0], "joint1");
	rosidl_runtime_c__String__assign(&joint_desired_msg.name.data[1], "joint2");
	rosidl_runtime_c__String__assign(&joint_desired_msg.name.data[2], "joint3");

	// Initialize joint values
	for (size_t i = 0; i < NUM_JOINTS; i++) {
		joint_desired_msg.position.data[i] = 0.0;
		joint_desired_msg.velocity.data[i] = 0.0;
		joint_desired_msg.effort.data[i]   = 0.0;
	}

	rclc_executor_init(&executor, &support.context, 1, &allocator);

	// Add subscription with callback
	rclc_executor_add_subscription(
			&executor,
			&subscriber,
			&joint_desired_msg,
			&JointStateCallback,
			ON_NEW_DATA);

	microros_initialized = 1;

}

void InitTask(void *argument)
{
    MicroROSInit();

    // Once done, delete this task
    vTaskDelete(NULL);
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

