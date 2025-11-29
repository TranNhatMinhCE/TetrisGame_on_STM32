/*
 * software_timer.h
 *
 *  Created on: Sep 24, 2023
 *      Author: HaHuyen
 */

#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include "tim.h"
#include "led_7seg.h"

#define TIMER_CYCLE_2 1

extern uint16_t flag_timer2;

void timer_init();
void setTimer2(uint16_t duration);
void timer_EnableDelayUs();
void delay_us (uint16_t us);

/* Addition */
#define NUM_TIMER		10

uint8_t is_timer2flag_set(uint8_t index);
void timer2_init(void);
void timer2_set(uint8_t index, uint16_t ms);
uint16_t get_counter(uint8_t index);
#endif /* INC_SOFTWARE_TIMER_H_ */
