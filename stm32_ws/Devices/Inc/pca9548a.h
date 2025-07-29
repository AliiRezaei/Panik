/*
 * pca9548a.h
 *
 *  Created on: Jul 25, 2025
 *      Author: Ali Rezaei
 */

#ifndef DEVICES_INC_PCA9548A_H_
#define DEVICES_INC_PCA9548A_H_

#define PCA9548A_ADDR (uint8_t)(0x70) // PCA9548A I2C address (7-bit)

/*
 * @brief : PCA9548a_s structure contains joints states
 */
typedef struct
{
	float      position[3];
	float      prevPosition[3];
	float      velocity[3];
	float      prevVelocity[3];
	TickType_t prevTick[3];
} PCA9548a_s;

/*
 * @brief : read the joints states
 */
void pca9548a_GetStates(PCA9548a_s *pca9548a, LowPassFilter_s *lpf, uint8_t sensor_id);

#endif /* DEVICES_INC_PCA9548A_H_ */
