/*
 * pca9548a.c
 *
 *  Created on: Jul 25, 2025
 *      Author: Ali Rezaei
 */

#include "main.h"

/*
 * @brief : read the joints velocity from pca9548a
 */
void pca9548a_GetStates(PCA9548a_s *pca9548a, LowPassFilter_s *lpf, uint8_t sensor_id)
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

	// pass filter
	pca9548a->position[sensor_id] += delta;
	pca9548a->position[sensor_id] = lpf_Operator(lpf, pca9548a->position[sensor_id]);

	// update velocity
	pca9548a->velocity[sensor_id] = lpf_FilteredDerivative(lpf,
			pca9548a->velocity[sensor_id],
			pca9548a->position[sensor_id]);

	// update previous position and velocity
	pca9548a->prevPosition[sensor_id] = pca9548a->position[sensor_id];
	pca9548a->prevVelocity[sensor_id] = pca9548a->velocity[sensor_id];
}

