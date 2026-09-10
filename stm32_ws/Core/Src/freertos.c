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
bool microros_initialized   = 0;
bool controller_initialized = 0;
bool filter_initialized     = 0;
PIDController_s pid[NUM_JOINTS];
LowPassFilter_s lpf[NUM_JOINTS];
LowPassFilter_s lpf_qaxis;
HighGainObsv_s  obsv[NUM_JOINTS];
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
uint32_t jointStatesReadTaskBuffer[ 512 ];
osStaticThreadDef_t jointStatesReadTaskControlBlock;
const osThreadAttr_t jointStatesReadTask_attributes = {
		.name = "jointStatesReadTask",
		.cb_mem = &jointStatesReadTaskControlBlock,
		.cb_size = sizeof(jointStatesReadTaskControlBlock),
		.stack_mem = &jointStatesReadTaskBuffer[0],
		.stack_size = sizeof(jointStatesReadTaskBuffer),
		.priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for jointStatesControlTask */
osThreadId_t jointStatesControlTaskHandle;
uint32_t jointStatesControlTaskBuffer[ 512 ];
osStaticThreadDef_t jointStatesControlTaskControlBlock;
const osThreadAttr_t jointStatesControlTask_attributes = {
	.name = "jointStatesControlTask",
	.cb_mem = &jointStatesControlTaskControlBlock,
	.cb_size = sizeof(jointStatesControlTaskControlBlock),
	.stack_mem = &jointStatesControlTaskBuffer[0],
	.stack_size = sizeof(jointStatesControlTaskBuffer),
	.priority = (osPriority_t) osPriorityNormal,
};

/* Definitions for initTask */
osThreadId_t initTaskHandle;
uint32_t initTaskBuffer[ 1024 * 1 ];
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
void JointStatesControlTask(void *argument);
void InitTask(void *argument);
void InitMicroROS(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

void ControlLoop(float e, float tau, size_t joint_id);

void ControlLoopNew(float e, float tau, size_t joint_id);

void SetPWM(float dc_phase_a, float dc_phase_b, float dc_phase_c, size_t joint_id);
float TorqueEstimation(uint8_t motor_id);
float TorqueEstimationNew(uint8_t motor_id, float Vq);
float GetDGZ(float theta, float Ua, float Ub, float Uc);
float WrapAngle(float angle);
void InitControllers(void);
void InitFilters(void);
void InitObserversANDqAxisFilter(void);
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

	/* creation of jointStatesControlTask */
	jointStatesControlTaskHandle = osThreadNew(JointStatesControlTask, NULL, &jointStatesControlTask_attributes);

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
//			joint_state_msg.effort.data[i]   = TorqueEstimation(i);
//			joint_state_msg.effort.data[i]   = 0.0f;
			if (i == 0) {
				joint_state_msg.position.data[i] += 0.30; // sensor 0 offset quick fix
			}
			if (i == 2) {
				joint_state_msg.position.data[i] -= 0.37; // sensor 2 offset quick fix
			}
		}

		// Publish the message
		rcl_ret_t ret = rcl_publish(&publisher, &joint_state_msg, NULL);
		if (ret != RCL_RET_OK) {
			printf("Failed to publish JointState message (line %d)\n", __LINE__);
		}

		GPIOB->ODR ^= GPIO_ODR_OD4;
		HAL_IWDG_Refresh(&hiwdg);
//		osDelay(pdMS_TO_TICKS(1));
		vTaskDelay(pdMS_TO_TICKS(10));
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
		vTaskDelay(pdMS_TO_TICKS(10));
//		osDelay(pdMS_TO_TICKS(1));
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
	while (!filter_initialized) {
		osDelay(10);
	}
	TickType_t lastWake = xTaskGetTickCount();
	const TickType_t period = pdMS_TO_TICKS(10);

	for (size_t i = 0; i < NUM_JOINTS; i++) {
		PCA9548a.position[i]     = 0.0;
		PCA9548a.prevPosition[i] = 0.0;
		PCA9548a.velocity[i]     = 0.0;
		PCA9548a.prevTick[i]     = 0;
	}

	for (;;)
	{
		for (uint8_t i = 0; i < NUM_JOINTS; i++) {
//			pca9548a_GetStates(&PCA9548a, &lpf[i], i);
			pca9548a_GetStates_HighGain(&PCA9548a, &obsv[i], i);
//			GPIOB->ODR ^= GPIO_ODR_OD1;
		}

//		osDelay(pdMS_TO_TICKS(10));
		vTaskDelayUntil(&lastWake, period);
	}
}

void JointStatesControlTask(void *argument)
{
	while (!controller_initialized) {
		osDelay(10);
	}

//	float gear_coeffs[] = {1.0, 40.0/16.0, 70.0/16.0};
	TickType_t lastWake = xTaskGetTickCount();
	const TickType_t period = pdMS_TO_TICKS(1);

	for (;;)
	{
		for (size_t joint_id = 0; joint_id < NUM_JOINTS; joint_id++) {
			// error calculation
//			float e = joint_desired_msg.position.data[joint_id] - joint_state_msg.position.data[joint_id];
			float e = joint_desired_msg.position.data[joint_id];

			// torque command
			float tau = joint_desired_msg.effort.data[joint_id];

			// run control loop
//			ControlLoop(e * gear_coeffs[joint_id], tau, joint_id);
			ControlLoop(e, tau, joint_id);
//			ControlLoopNew(e, tau, joint_id);
//			osDelay(pdMS_TO_TICKS(1));
			vTaskDelayUntil(&lastWake, period);
		}
//		GPIOB->ODR ^= GPIO_ODR_OD1;
//		vTaskDelayUntil(&lastWake, period);
//		osDelay(pdMS_TO_TICKS(10));
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

void InitMicroROS(void)
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
	rclc_node_init_default(&node, "stm32_node", "", &support);

	// Initialize subscriber
	rclc_subscription_init_best_effort(
			&subscriber,
			&node,
			ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
			"joint_desireds");

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

void ControlLoop(float e, float tau, size_t joint_id)
{
//	float elec_angle = pid_Operator(&pid[joint_id], e);
	float elec_angle = e;
//	float elec_angle = 14.0 * joint_state_msg.position.data[joint_id];
//	float elec_angle = WrapAngle(pid_Operator(&pid[joint_id], e));
//	float elec_angle = pid_Operator(&pid[joint_id], tau);
//	float elec_angle = tau;
	float Uq = tau; // slow start-up
//	float Uq = pid_Operator(&pid[joint_id], tau);
//	float Uq = tau;
//	float Uq = lpf_Operator(&lpf_qaxis, tau); // slow start-up
	float Ud = 0.0;
//	float theta = joint_state_msg.position.data[joint_id];
//	float w     = joint_state_msg.velocity.data[joint_id];
//	float theta_d = joint_desired_msg.position.data[joint_id];
//	float w_d     = 0.0;
//	float P = 15.0, D = 1.02;
//	float elec_angle = WrapAngle(14 * theta);
////	float Uq = -(P * (theta_d - theta) + D * (w_d - w));
////	float Uq = -(P * (theta_d - 0.0) + D * (0.0 - w));
//	float Uq = -(P * (0.0 - theta) + D * (0.0 - w));
////	Uq = lpf_Operator(&lpf_qaxis, _constrain(-5.0*Uq, -15.0f, 15.0f)); // slow start-up
//	Uq = lpf_Operator(&lpf_qaxis, Uq); // slow start-up
//	float Ud = 0.0;

	// sin cos of elec_angle
	float s_elec_angle = sin(elec_angle);
	float c_elec_angle = cos(elec_angle);

	// inverse park transform
	float Ualpha = c_elec_angle * Ud - s_elec_angle * Uq;
	float Ubeta  = s_elec_angle * Ud + c_elec_angle * Uq;

	// clarke transform
	float Ua = Ualpha;
	float Ub = - 0.5f * Ualpha + 0.5f * sqrt(3.0f) * Ubeta;
	float Uc = - 0.5f * Ualpha - 0.5f * sqrt(3.0f) * Ubeta;

	// center
	float center = 24.0f / 2.0f; // changed. old value = 15 / 2

	// midpoint clamp
	float Umin = fmin(Ua, fmin(Ub, Uc));
	float Umax = fmax(Ua, fmax(Ub, Uc));
	center = center - 0.5f * (Umax + Umin);

//	// centered modulation
//	Ua = _constrain(Ua - Umin, 0.0, 24.0);
//	Ub = _constrain(Ub - Umin, 0.0, 24.0);
//	Uc = _constrain(Uc - Umin, 0.0, 24.0);

	// centered modulation
	Ua = _constrain(Ua + center, 0.0f, 24.0f); // changed
	Ub = _constrain(Ub + center, 0.0f, 24.0f);
	Uc = _constrain(Uc + center, 0.0f, 24.0f);


//	float gear_coeffs[] = {1.0, 40.0/16.0, 70.0/16.0};
//	// pole pairs
//	float pp = 14.0;
//
//	//
//	float theta = joint_state_msg.position.data[joint_id] * gear_coeffs[joint_id] * pp;
//	float Uq_tmp = GetDGZ(theta, Ua, Ub, Uc);
//	joint_state_msg.effort.data[joint_id] = Uq_tmp;

//	joint_state_msg.effort.data[joint_id] = TorqueEstimationNew(joint_id, Uq_tmp);


	float dc_a = _constrain(Ua / 24.0, 0.0, 1.0);
	float dc_b = _constrain(Ub / 24.0, 0.0, 1.0);
	float dc_c = _constrain(Uc / 24.0, 0.0, 1.0);

	SetPWM(dc_a, dc_b, dc_c, joint_id);
}

void ControlLoopNew(float e, float tau, size_t joint_id)
{
	// electrical angle
	float elec_angle = 14.0 * joint_state_msg.position.data[joint_id];

	// dq voltages
 	float Uq = tau;
 	float Ud = 0.0;

	// dc-link voltage
	const float Vdc = 24.0f;

	// linear SVPWM voltage limit
	const float Vmax = Vdc * 0.5773502692f;   // Vdc/sqrt(3)

    // limit dq voltage vector
    float U_mag = sqrtf(Ud * Ud + Uq * Uq);
    if (U_mag > Vmax)
    {
        float scale = Vmax / U_mag;

        Ud *= scale;
        Uq *= scale;
    }

	// sin cos of elec_angle
	float s_elec_angle = sin(elec_angle);
	float c_elec_angle = cos(elec_angle);

	// inverse park transform
	float Ualpha = c_elec_angle * Ud - s_elec_angle * Uq;
	float Ubeta  = s_elec_angle * Ud + c_elec_angle * Uq;

	// inverse clarke transform
    const float SQRT3_BY_2 = 0.8660254038f;
    float Ua = Ualpha;
    float Ub = -0.5f * Ualpha + SQRT3_BY_2 * Ubeta;
    float Uc = -0.5f * Ualpha - SQRT3_BY_2 * Ubeta;

	// centered SVPWM
    float Umin = fminf(Ua, fminf(Ub, Uc));
    float Umax = fmaxf(Ua, fmaxf(Ub, Uc));
    float Vcm  = -0.5f * (Umax + Umin);
    Ua += Vcm;
    Ub += Vcm;
    Uc += Vcm;

    // convert phase voltages to duty cycles
    float dc_a = 0.5f + Ua / Vdc;
    float dc_b = 0.5f + Ub / Vdc;
    float dc_c = 0.5f + Uc / Vdc;
    dc_a = _constrain(dc_a, 0.0f, 1.0f);
    dc_b = _constrain(dc_b, 0.0f, 1.0f);
    dc_c = _constrain(dc_c, 0.0f, 1.0f);

    // PWM generation
    SetPWM(dc_a, dc_b, dc_c, joint_id);
}

void SetPWM(float dc_phase_a, float dc_phase_b, float dc_phase_c, size_t joint_id)
{
	uint32_t ticks = 0;
	switch (joint_id) {
	case 0:
		// PWM counts
		ticks = TIM1->ARR + 1;

		// write duty cycle in timer 1
		TIM1->CCR1 = (ticks * dc_phase_a);
		TIM1->CCR2 = (ticks * dc_phase_b);
		TIM1->CCR3 = (ticks * dc_phase_c);
		break;
	case 1:
		// PWM counts
		ticks = TIM2->ARR + 1;

		// write duty cycle in timer 2
		TIM2->CCR1 = (ticks * dc_phase_a);
		TIM2->CCR2 = (ticks * dc_phase_b);
		TIM2->CCR3 = (ticks * dc_phase_c);
		break;
	case 2:
		// PWM counts
		ticks = TIM3->ARR + 1;

		// write duty cycle in timer 3
		TIM3->CCR1 = (ticks * dc_phase_a);
		TIM3->CCR2 = (ticks * dc_phase_b);
		TIM3->CCR3 = (ticks * dc_phase_c);
		break;
	default:
		break;
	}
}

void InitControllers(void)
{
	for (size_t i = 0; i < NUM_JOINTS; i++) {
		pid_Init(&pid[i]);
		pid_SetGains(&pid[i], 0.01, 20.0, 0.0, 50.0, 1000.0);
	}

	controller_initialized = 1;
}

void InitFilters(void)
{
	float Tfd = 0.1, Tff = 0.05;
//	float Tfd = 0.05, Tff = 0.005;
	for (size_t i = 0; i < NUM_JOINTS; i++) {
		lpf_Init(&lpf[i], Tff, Tfd);
	}

	lpf_Init(&lpf_qaxis, 0.5, Tfd);

	filter_initialized = 1;
}

void InitObserversANDqAxisFilter(void)
{
	// observer initial states
	ObsvStates_s x_hat0;
	x_hat0.pos = 0.0;
	x_hat0.vel = 0.0;

	// design params
	float alpha1 = 6.0;
	float alpha2 = 5.0;
	float eps    = 0.1;

	// initialization
	for (size_t i = 0; i < NUM_JOINTS; i++) {
		observer_Init(&obsv[i], x_hat0, alpha1, alpha2, eps);
	}

	// init q-axis low-pass filter
	float Tfd = 0.1, Tff = 0.5;
	lpf_Init(&lpf_qaxis, Tff, Tfd);

	// flag
	filter_initialized = 1;
}

float TorqueEstimation(uint8_t motor_id)
{
	float Kt = 1.9137; // torque constant for XM9025GB-SR
	uint32_t ticks = 0;
	float    dc    = 0;
	switch (motor_id) {
	case 0:
		// PWM counts
		ticks = TIM1->ARR + 1;

		// duty cycle average
		dc = (float)(TIM1->CCR1 + TIM1->CCR2 + TIM1->CCR3) / (3.0 * ticks);
		break;
	case 1:
		// PWM counts
		ticks = TIM2->ARR + 1;

		// duty cycle average
		dc = (float)(TIM2->CCR1 + TIM2->CCR2 + TIM2->CCR3) / (3.0 * ticks);
		break;
	case 2:
		// PWM counts
		ticks = TIM3->ARR + 1;

		// duty cycle average
		dc = (float)(TIM3->CCR1 + TIM3->CCR2 + TIM3->CCR3) / (3.0 * ticks);
		break;
	default:
		break;
	}

	float R = 34.3; // phase resistance
	float V = 24.0; // phase max voltage
	float torque = Kt * dc * V / R;
	return torque;
}

float TorqueEstimationNew(uint8_t motor_id, float Vq)
{
	float gear_coeffs[] = {1.0, 40.0/16.0, 70.0/16.0};
	float Ke = 1.9137 / 5.0;   // back-emf constant for XM9025GB-SR
	float Kt = 0.6 * Ke;       // torque constant for XM9025GB-SR
	float pp = 14; // pole-pairs
	float lambda = Ke / pp;

	float R = 34.3; // phase resistance
	float w  = joint_state_msg.velocity.data[motor_id];
	float we = w * gear_coeffs[motor_id] * pp;
	float torque = Kt * (Vq - we * lambda) / R;
	return torque;
}

float GetDGZ(float theta, float Ua, float Ub, float Uc)
{

//	// pole pairs
//	float p = 14.0;

	// wrap elec angle
	theta = WrapAngle(theta);

	// calculate q axis value
	float M_PI_2_3 = 2.0 * M_PI / 3.0;
	float Uq =   sqrt(2)/3 * (cos(theta) * Ua + cos(theta - M_PI_2_3) * Ub + cos(theta + M_PI_2_3) * Uc);
//	float Uq = - sqrt(2)/3 * (sin(theta) * Ua + sin(theta - M_PI_2_3) * Ub + sin(theta + M_PI_2_3) * Uc);

	return Uq;
}

float WrapAngle(float angle)
{
	double two_pi = 2.0 * M_PI;

	double wrapped_angle = fmod(angle, two_pi) - M_PI;

	if (wrapped_angle < 0)
		wrapped_angle += two_pi;

	if (wrapped_angle == 0.0 && angle > 0.0)
		wrapped_angle = two_pi;

	float wrapped_angle_float = (float)wrapped_angle; // This line added

	return wrapped_angle_float;
}

void InitTask(void *argument)
{
	// Init Micro-ROS
    InitMicroROS();

    // Init joints controller
    InitControllers();

    // Init sensor filters
//    InitFilters();
    InitObserversANDqAxisFilter();

    // Once done, delete this task
    vTaskDelete(NULL);
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

