/*
 * software_timer.c
 *
 *  Created on: Sep 24, 2023
 *      Author: HaHuyen
 */

#include "software_timer.h"

/* Addition */
uint8_t timer2_flag[NUM_TIMER] = {0};
uint16_t timer2_counter[NUM_TIMER] = {0};

void timer_init(){
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start(&htim1);
}

void timer_EnableDelayUs(){
	HAL_TIM_Base_Start(&htim1);
}

void timer2_set(uint8_t index, uint16_t ms) {
	timer2_counter[index] = ms / TIMER_CYCLE_2;
//	timer2_flag[index] = 0;
}

void timer2_run(){
	for(uint8_t i = 0; i < NUM_TIMER; i++)
		if(timer2_counter[i] > 0) {
			timer2_counter[i]--;
			if(timer2_counter[i] == 0)
				timer2_flag[i] = 1;
		}
}

uint8_t is_timer2flag_set(uint8_t index) {
	if(timer2_flag[index]) {
		timer2_flag[index] = 0;
		return 1;
	}
	return 0;
}

uint16_t get_counter(uint8_t index) { return timer2_counter[index]; }

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM2) {
		timer2_run();
	}
}

void delay_us (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim1,0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim1) < us);  // wait for the counter to reach the us input in the parameter
}
