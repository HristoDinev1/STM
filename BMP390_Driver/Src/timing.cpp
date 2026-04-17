/*
 * timing.cpp
 *
 *  Created on: Nov 18, 2025
 *      Author: icodi
 */
#include "timing.h"
#include "stm32h7xx_hal.h"

void init_for_flight() noexcept
{
	// Enable TIM2 clock
	__HAL_RCC_TIM2_CLK_ENABLE();

	// Set TIM2 to count microseconds
	TIM2->PSC = (SystemCoreClock / 1'000'000) - 1; // 1 MHz
	TIM2->ARR = 0xFFFFFFFF; // free running
	TIM2->CNT = 0;
	TIM2->CR1 |= TIM_CR1_CEN; // enable counter
}

void delay_us(uint32_t us) noexcept
{
	uint32_t start = TIM2->CNT;
	while ((TIM2->CNT - start) < us)
	{
		// spin (poll)
	}
}
