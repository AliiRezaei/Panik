/*
 * lpf.h
 *
 *  Created on: Jul 29, 2025
 *      Author: Ali Rezaei
 */

#ifndef UTILS_INC_LPF_H_
#define UTILS_INC_LPF_H_

typedef struct
{
	float Tff;              // filter time constant
	float Tfd;              // filtered derivative time constant
	float prevYOpr;         // previous operator output
	float prevYDrv;         // previous derivative output
	float prevYd;           // previous data to derivative
	TickType_t prevTickOpr; // previous operator tick
	TickType_t prevTickDrv; // previous derivative tick
} LowPassFilter_s;

void  lpf_Init(LowPassFilter_s *lpf, float Tff_, float Tfd_d);
void  lpf_Reset(LowPassFilter_s *lpf);
float lpf_Operator(LowPassFilter_s *lpf, float yf);
float lpf_FilteredDerivative(LowPassFilter_s *lpf, float prevVel, float yd);

#endif /* UTILS_INC_LPF_H_ */
