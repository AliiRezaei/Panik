/*
 * observer.c
 *
 *  Created on: Sep 2, 2026
 *      Author: Ali Rezaei
 */

#include "main.h"


void observer_Init(HighGainObsv_s *obsv, ObsvStates_s initialStates, float alpha1, float alpha2, float eps)
{
	observer_Reset(obsv);
	obsv->prevStates.pos = initialStates.pos;
	obsv->prevStates.vel = initialStates.vel;
	obsv->states.pos     = initialStates.pos;
	obsv->states.vel     = initialStates.vel;
	obsv->alpha1         = alpha1;
	obsv->alpha2         = alpha2;
	obsv->eps            = eps;
	obsv->prevTick       = xTaskGetTickCount();
}
void  observer_Reset(HighGainObsv_s *obsv)
{
	obsv->prevStates.pos = 0.0;
	obsv->prevStates.vel = 0.0;
	obsv->states.pos     = 0.0;
	obsv->states.vel     = 0.0;
	obsv->alpha1         = 0.0;
	obsv->alpha2         = 0.0;
	obsv->eps            = 0.0;
	obsv->prevTick       = 0.0;
}


void observer_Operator(HighGainObsv_s *obsv, float ym)
{
	// compute timestamp
	TickType_t now = xTaskGetTickCount();
	float Ts = (float)(now - obsv->prevTick) / 1000.0f;
	if(Ts < 0.0 || Ts > 0.5) {Ts = 0.001f;} // when xTaskGetTickCount overflow

	// observation error
	float y_tilde = ym - obsv->prevStates.pos;

	// design params
	float alpha1 = obsv->alpha1;
	float alpha2 = obsv->alpha2;
	float eps    = obsv->eps;

	// correction terms
	float e_pos = y_tilde * ((Ts*Ts*alpha2) / (2*eps*eps) + (Ts*alpha1) / eps);
	float e_vel = (Ts*alpha2*y_tilde) / (eps*eps);

	// observer model
	obsv->states.pos = obsv->prevStates.pos + Ts * obsv->prevStates.vel + e_pos;
	obsv->states.vel = obsv->prevStates.vel + e_vel;

	// update tick and store last states
	obsv->prevTick = now;
	obsv->prevStates.pos = obsv->states.pos;
	obsv->prevStates.vel = obsv->states.vel;

}
