/*
 * pca9548a.c
 *
 *  Created on: Jul 25, 2025
 *      Author: Ali Rezaei
 */

#include "main.h"

/*
 * @brief : read the joints position from pca9548a
 */
void pca9548a_GetPosition(PCA9548a_s *pca9548a, uint8_t sensor_id)
{

}

/*
 * @brief : read the joints velocity from pca9548a
 */
void pca9548a_GetStates(PCA9548a_s *pca9548a, uint8_t sensor_id, float Tfd, float Tff)
{
	// select the channel on the PCA9548A
	i2c_I2C1_Write(PCA9548A_ADDR, (1 << sensor_id));

	// read raw angle [0, 4095]
	uint16_t rawAngle = as5600_ReadRawAngle();
	float angle = rawAngle * AS5600_Gain_Radian; // [0, 2PI]

	// previous raw angle
	float prev = pca9548a->prevPosition[sensor_id];

	// compute difference
	float delta = angle - prev;

	// normalize the delta to [-PI, +PI] to detect wrapping
	if (delta > M_PI)
		delta -= 2.0 * M_PI;
	else if (delta < -M_PI)
		delta += 2.0 * M_PI;

	// compute timestamp
	TickType_t now = xTaskGetTickCount();
	float Ts = (float)(now - pca9548a->prevTick[sensor_id]) / 1000.0f;
	if(Ts < 0.0 || Ts > 0.5) {return;} // when xTaskGetTickCount overflow
	pca9548a->prevTick[sensor_id] = now;

	// accumulate total angle
	pca9548a->position[sensor_id] += delta;

	float alpha = Tff / (Tff + Ts);
	float positionFilterd = alpha * pca9548a->prevPosition[sensor_id] + (1.0f - alpha) * pca9548a->position[sensor_id];
	pca9548a->position[sensor_id]  = positionFilterd;

	// take filtered derivative (s / (Tf s + 1)) from position
	float a1 = (Tfd - Ts / 2.0f) / (Tfd + Ts / 2.0f);
	float b0 = 1.0f / (Tfd + Ts / 2.0f);
	float velocityPrev = pca9548a->velocity[sensor_id];
	float velocity = a1 * velocityPrev + b0 * (pca9548a->position[sensor_id] - pca9548a->prevPosition[sensor_id]);

	// update velocity
	float velocityFilterd = alpha * pca9548a->prevVelocity[sensor_id] + (1.0f - alpha) * velocity;
	pca9548a->velocity[sensor_id]  = velocityFilterd;

	// update previous position and velocity
	pca9548a->prevPosition[sensor_id] = pca9548a->position[sensor_id];
	pca9548a->prevVelocity[sensor_id] = pca9548a->velocity[sensor_id];
}

