/*
 * button.h
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

/* Includes */
#include <stdint.h>
#include "spi.h"
#include "gpio.h"

#define MAX_BUTTON 		10
#define TIME_OUT 		30

#define NORMAL_STATE 	0
#define PRESSED_STATE 	1

/* Variables */
extern uint16_t button_count[16];

/* Functions */
void button_init();

void getKeyInput();
int isButtonPressed(int number);
int isButtonLongPressed(int number);

#endif /* INC_BUTTON_H_ */
