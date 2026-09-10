/*
 * observer.h
 *
 *  Created on: Sep 2, 2026
 *      Author: Ali Rezaei
 */

#ifndef UTILS_INC_OBSERVER_H_
#define UTILS_INC_OBSERVER_H_

typedef struct
{
	float pos;        // first state (position)
	float vel;        // second state (velocity)
} ObsvStates_s;

typedef struct
{
	ObsvStates_s prevStates; // previous states
	ObsvStates_s states;     // current states
	float alpha1;            // obsv characteristic first coeff
	float alpha2;            // obsv characteristic second coeff
	float eps;               // high gain (1/eps)
	TickType_t   prevTick;   // previous operator tick
} HighGainObsv_s;

void observer_Init(HighGainObsv_s *obsv, ObsvStates_s initialStates, float alpha1, float alpha2, float eps);
void observer_Reset(HighGainObsv_s *obsv);
void observer_Operator(HighGainObsv_s *obsv, float ym);

#endif /* UTILS_INC_OBSERVER_H_ */
