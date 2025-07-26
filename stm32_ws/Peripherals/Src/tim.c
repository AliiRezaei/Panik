/*
 * tim.c
 *
 *  Created on: Jun 8, 2025
 *      Author: Ali Rezaei
 */

#include "main.h"

// TIM1 configuration
void tim_TIM1_3PWM_Config(void)
{
	// enable clock for GPIOA
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	// set PA8, PA9, PA10 to alternate function (TIM1_CH1, TIM1_CH2, TIM1_CH3) AF1
	GPIOA->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
	GPIOA->MODER |=  (GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1); // Alternate Function

	GPIOA->AFR[1] &= ~((0xF << (0 * 4)) | (0xF << (1 * 4)) | (0xF << (2 * 4)));
	GPIOA->AFR[1] |=  ((0x1 << (0 * 4)) | (0x1 << (1 * 4)) | (0x1 << (2 * 4))); // AF1 for TIM1

	// set speed, push-pull
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9 | GPIO_OSPEEDER_OSPEEDR10;

	// enable TIM1 clock
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	// center aligned mode 3
	TIM1->CR1 &= ~TIM_CR1_CMS;
	TIM1->CR1 |=  TIM_CR1_CMS_1 | TIM_CR1_CMS_0; // CMS = 11

	// set auto reload value and prescaler
	TIM1->ARR = 2000 - 1; // because of APB2 clock is 100 MHz and center aligned mode 3
	TIM1->PSC = 0;        // 25 KHz

	// configure CH1, CH2 in CCMR1 (PWM mode 1, preload enable)
	TIM1->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);
	TIM1->CCMR1 |=  (6 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
	TIM1->CCMR1 |=  (6 << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;

	// configure CH3 in CCMR2
	TIM1->CCMR2 &= ~TIM_CCMR2_OC3M;
	TIM1->CCMR2 |= (6 << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;

	// enable outputs
	TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;

	// enable auto reload preload
	TIM1->CR1 |= TIM_CR1_ARPE;

	// force update to load registers
	TIM1->EGR |= TIM_EGR_UG;

	// zero percent duty cycle at the beginning
	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	TIM1->CCR3 = 0;

	// enable main output (TIM1 is an advanced timer)
	TIM1->BDTR |= TIM_BDTR_MOE;

	// start timer
	TIM1->CR1 |= TIM_CR1_CEN;
}

// TIM2 configuration
void tim_TIM2_3PWM_Config(void)
{
	// enable clocks for GPIOA, and GPIOB
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

	// set PA15 to alternate function (TIM2_CH1) AF1
	GPIOA->MODER &= ~GPIO_MODER_MODER15;
	GPIOA->MODER |=  GPIO_MODER_MODER15_1;

	GPIOA->AFR[1] &= ~((0xF << (7 * 4)));
	GPIOA->AFR[1] |=  ((0x1 << (7 * 4))); // AF1 for TIM2

	// set PB3, and PB10 to alternate function (TIM2_CH2, TIM2_CH3) AF1
	GPIOB->MODER &= ~(GPIO_MODER_MODER3 | GPIO_MODER_MODER10);
	GPIOB->MODER |=  (GPIO_MODER_MODER3_1 | GPIO_MODER_MODER10_1);

	GPIOB->AFR[0] &= ~((0xF << (3 * 4)));
	GPIOB->AFR[0] |=  ((0x1 << (3 * 4))); // AF1 for TIM2_CH2

	GPIOB->AFR[1] &= ~((0xF << (2 * 4)));
	GPIOB->AFR[1] |=  ((0x1 << (2 * 4))); // AF1 for TIM2_CH3

	// set speed, push-pull
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR15;
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3 | GPIO_OSPEEDER_OSPEEDR10;

	// enable TIM2 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	// center aligned mode 3
	TIM2->CR1 &= ~TIM_CR1_CMS;
	TIM2->CR1 |=  TIM_CR1_CMS_1 | TIM_CR1_CMS_0; // CMS = 11

	// set auto reload value and prescaler
	TIM2->ARR = 2000 - 1; // because of APB1 clock is 50 MHz and center aligned mode 3
	TIM2->PSC = 0;        // 25 KHz

	// configure CH1, CH2 in CCMR1 (PWM mode 1, preload enable)
	TIM2->CCMR1 &= ~(TIM_CCMR1_OC1M
			| TIM_CCMR1_OC2M);
	TIM2->CCMR1 |=  (6 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
	TIM2->CCMR1 |=  (6 << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;

	// configure CH3 in CCMR2
	TIM2->CCMR2 &= ~TIM_CCMR2_OC3M;
	TIM2->CCMR2 |= (6 << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;

	// enable outputs
	TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;

	// enable auto reload preload
	TIM2->CR1 |= TIM_CR1_ARPE;

	// force update to load registers
	TIM2->EGR |= TIM_EGR_UG;

	// zero percent duty cycle at the beginning
	TIM2->CCR1 = 0;
	TIM2->CCR2 = 0;
	TIM2->CCR3 = 0;

	// enable main output
	TIM2->BDTR |= TIM_BDTR_MOE;

	// start timer
	TIM2->CR1 |= TIM_CR1_CEN;
}

// TIM3 configuration
void tim_TIM3_3PWM_Config(void)
{
	// enable clocks for GPIOA, and GPIOB
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

	// set PA6, and PA7 to alternate function (TIM3_CH1, TIM3_CH2) AF2
	GPIOA->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
	GPIOA->MODER |=  (GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1);

	GPIOA->AFR[0] &= ~((0xF << (6 * 4)) | (0xF << (7 * 4)));
	GPIOA->AFR[0] |=  ((0x2 << (6 * 4)) | (0x2 << (7 * 4))); // AF2 for TIM3

	// set PB0 to alternate function (TIM3_CH3) AF1
	GPIOB->MODER &= ~GPIO_MODER_MODER0;
	GPIOB->MODER |=  GPIO_MODER_MODER0_1;

	GPIOB->AFR[0] &= ~((0xF << (0 * 4)));
	GPIOB->AFR[0] |=  ((0x2 << (0 * 4))); // AF1 for TIM3_CH3

	// set speed, push-pull
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7;
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR0;

	// enable TIM3 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	// center aligned mode 3
	TIM3->CR1 &= ~TIM_CR1_CMS;
	TIM3->CR1 |=  TIM_CR1_CMS_1 | TIM_CR1_CMS_0; // CMS = 11

	// set auto reload value and prescaler
	TIM3->ARR = 2000 - 1; // because of APB1 clock is 50 MHz and center aligned mode 3
	TIM3->PSC = 0;        // 25 KHz

	// configure CH1, CH2 in CCMR1 (PWM mode 1, preload enable)
	TIM3->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);
	TIM3->CCMR1 |=  (6 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
	TIM3->CCMR1 |=  (6 << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;

	// configure CH3 in CCMR2
	TIM3->CCMR2 &= ~TIM_CCMR2_OC3M;
	TIM3->CCMR2 |= (6 << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;

	// enable outputs
	TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;

	// enable auto reload preload
	TIM3->CR1 |= TIM_CR1_ARPE;

	// force update to load registers
	TIM3->EGR |= TIM_EGR_UG;

	// zero percent duty cycle at the beginning
	TIM3->CCR1 = 0;
	TIM3->CCR2 = 0;
	TIM3->CCR3 = 0;

	// enable main output
	TIM3->BDTR |= TIM_BDTR_MOE;

	// start timer
	TIM3->CR1 |= TIM_CR1_CEN;
}
