/*
 * lpf.c
 *
 *  Created on: Jul 29, 2025
 *      Author: Ali Rezaei
 */

#include "main.h"

void  lpf_Init(LowPassFilter_s *lpf, float Tff_, float Tfd_)
{
	lpf_Reset(lpf);
	lpf->Tff = Tff_;
	lpf->Tfd = Tfd_;
	lpf->prevTickOpr = xTaskGetTickCount();
	lpf->prevTickDrv = xTaskGetTickCount();
}

void  lpf_Reset(LowPassFilter_s *lpf)
{
	lpf->prevYOpr    = 0.0;
	lpf->prevYDrv    = 0.0;
	lpf->prevYd      = 0.0;
	lpf->prevTickOpr = 0.0;
	lpf->prevTickDrv = 0.0;
}

float lpf_Operator(LowPassFilter_s *lpf, float yf)
{
	// compute timestamp
	TickType_t now = xTaskGetTickCount();
	float Ts = (float)(now - lpf->prevTickOpr) / 1000.0f;
	if(Ts < 0.0 || Ts > 0.5) {Ts = 0.001f;} // when xTaskGetTickCount overflow

	// pass filter
	float alpha     = lpf->Tff / (lpf->Tff + Ts);
	float filteredY = alpha * lpf->prevYOpr + (1.0f - alpha) * yf;

	// update tick and output
	lpf->prevTickOpr = now;
	lpf->prevYOpr    = filteredY;

	return filteredY;
}

float lpf_FilteredDerivative(LowPassFilter_s *lpf, float prevVel, float yd)
{
	// compute timestamp
	TickType_t now = xTaskGetTickCount();
	float Ts = (float)(now - lpf->prevTickDrv) / 1000.0f;
	if(Ts < 0.0 || Ts > 0.5) {Ts = 0.001f;} // when xTaskGetTickCount overflow

	// take filtered derivative (s / (Tf s + 1)) from position
	float a1 = (lpf->Tfd - Ts / 2.0f) / (lpf->Tfd + Ts / 2.0f);
	float b0 = 1.0f / (lpf->Tfd + Ts / 2.0f);
	float filteredDrv = a1 * prevVel + b0 * (yd - lpf->prevYd);

	float alpha = lpf->Tff / (lpf->Tff + Ts);
	filteredDrv = alpha * lpf->prevYDrv + (1.0f - alpha) * filteredDrv;

	// update tick and output
	lpf->prevTickDrv = now;
	lpf->prevYDrv    = filteredDrv;
	lpf->prevYd      = yd;

	return filteredDrv;
}
