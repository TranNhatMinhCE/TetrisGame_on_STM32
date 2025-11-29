/*
 * button.c
 */

/* Includes */
#include "button.h"

/* Variables */
uint16_t button_count[16] = {0};
static uint16_t button_spi_buffer = 0x0000;

uint16_t KeyReg[MAX_BUTTON][3];
uint16_t TimeOutForKeyPress[MAX_BUTTON];
uint16_t button_pressed[MAX_BUTTON] = {0};
uint16_t button_long_pressed[MAX_BUTTON] = {0};
uint16_t button_flag[MAX_BUTTON] = {0};


/* Functions */
/**
 * @brief  	Init matrix button
 * @param  	None
 * @retval 	None
 */
void button_init() {
	HAL_GPIO_WritePin(BTN_LOAD_GPIO_Port, BTN_LOAD_Pin, 1);

	for (int i = 0; i < MAX_BUTTON; i++) {
        for (int j = 0; j < 3; j++) {
            KeyReg[i][j] = NORMAL_STATE;
        }
        TimeOutForKeyPress[i] = TIME_OUT;
    }
}

/**
 * @brief  	Scan matrix button
 * @param  	None
 * @note  	Call every 50ms
 * @retval 	None
 */
void button_scan() {
	HAL_GPIO_WritePin(BTN_LOAD_GPIO_Port, BTN_LOAD_Pin, 0);
	HAL_GPIO_WritePin(BTN_LOAD_GPIO_Port, BTN_LOAD_Pin, 1);
	HAL_SPI_Receive(&hspi1, (void*) &button_spi_buffer, 2, 10);

	int button_index = 0;
	uint16_t mask = 0x8000;
	for (int i = 0; i < 16; i++) {
		if (i >= 0 && i <= 3) {
			button_index = i + 4;
		} else if (i >= 4 && i <= 7) {
			button_index = 7 - i;
		} else if (i >= 8 && i <= 11) {
			button_index = i + 4;
		} else {
			button_index = 23 - i;
		}
		if (button_spi_buffer & mask)
			button_count[button_index] = NORMAL_STATE;
		else
			button_count[button_index] = PRESSED_STATE;
		mask = mask >> 1;
	}
}

int isButtonPressed(int number) {
	if(button_flag[number] == 1){
		button_flag[number] = 0;
		return 1;
	}
	return 0;
}

int isButtonLongPressed(int number){
	if(button_long_pressed[number] == 1){
		button_long_pressed[number] = 0;
		return 1;
	}
	return 0;
}

void getKeyInput() {
    for (int i = 0; i < MAX_BUTTON; i++) {
        KeyReg[i][1] = KeyReg[i][0];
        KeyReg[i][0] = button_count[i];
    }

    button_scan();

    for (int i = 0; i < MAX_BUTTON; i++) {

        if ((button_count[i] == KeyReg[i][0]) && (KeyReg[i][0] == KeyReg[i][1]))
        {
        	if (KeyReg[i][1] != KeyReg[i][2]) {
                KeyReg[i][2] = KeyReg[i][1];

                if (KeyReg[i][2] == PRESSED_STATE) {
                    TimeOutForKeyPress[i] = TIME_OUT;
                    button_flag[i] = 1;
                }
            }
            else {
                TimeOutForKeyPress[i]--;

                if (TimeOutForKeyPress[i] == 0) {
                    TimeOutForKeyPress[i] = TIME_OUT;

                    if (KeyReg[i][2] == PRESSED_STATE){
                    	button_flag[i] = 0;
                        button_long_pressed[i] = 1;
                    }
                }
            }
        }
    }

}

