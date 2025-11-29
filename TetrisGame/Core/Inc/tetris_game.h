/*
 * tetris.h
 *
 *  Created on: Nov 21, 2025
 *      Author: LEGION
 */

#ifndef INC_TETRIS_GAME_
#define INC_TETRIS_GAME_

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "main.h" // Chứa định nghĩa STM32
#include "lcd.h"
#include "button.h"
#include "touch.h"
#include "software_timer.h"
#include "at24c.h"
#include "picture.h"
#include "adc.h"

// --- CẤU HÌNH MÀN HÌNH LCD 240x320 ---
#define BLOCK_SIZE  15
#define COLS        10
#define ROWS        20
#define SIDEBAR_WIDTH_BLOCKS 6

#define SCREEN_WIDTH_GAME  ((COLS + SIDEBAR_WIDTH_BLOCKS) * BLOCK_SIZE) // 240
#define SCREEN_HEIGHT_GAME 320

#define MENU_BKLOGO_X	75
#define MENU_BKLOGO_Y	10
#define MENU_GAMELOGO_X	15
#define MENU_GAMELOGO_Y	120 // 60
#define MENU_HISCORE_X 	60
#define MENU_HISCORE_Y	170 // 130

#define SIDEBAR_X	(COLS * BLOCK_SIZE) // 150px

#define NEXTBOX_W	75
#define NEXTBOX_H	165
#define NEXTBOX_X	(SIDEBAR_X + 10)
#define NEXTBOX_Y	(OFFSET_Y + 135)

#define PAUSE_BOX_W 	160
#define PAUSE_BOX_H		130
#define PAUSE_BOX_X		((SCREEN_WIDTH_GAME - PAUSE_BOX_W) / 2)	// (240-160)/2 = 40
#define PAUSE_BOX_Y		((SCREEN_HEIGHT_GAME - PAUSE_BOX_H) / 2) // (320-130)/2 = 95

#define GO_BOX_W 		160
#define GO_BOX_H		160
#define GO_BOX_X		((SCREEN_WIDTH_GAME - GO_BOX_W) / 2) // 40
#define GO_BOX_Y		((SCREEN_HEIGHT_GAME - GO_BOX_H) / 2) // 80

// Offset để căn giữa bàn cờ theo chiều dọc (320 - 300)/2 = 10
#define OFFSET_Y    10

// --- BẢNG MÀU RGB565 (Đã convert từ RGB888) ---
// Công thức: ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3)
#define C_BG        0x0862  // Dark Navy {13, 17, 23}
#define C_GRID      0x1925  // Grid Line {30, 35, 45}
#define C_SIDEBAR   0x0862  // Sidebar (Same as BG)
#define C_NEXT_BG   0x1164  // Next Box BG {22, 27, 34}
#define C_TXT_LBL   0x8C93  // Label Gray {139, 148, 158}
#define C_TXT_VAL   0xFFFF  // White
#define C_BTN_GREEN 0x2E66  // Green {46, 204, 113}
#define C_BTN_BLUE  0x34D3  // Blue {52, 152, 219}
#define C_BTN_RED   0xE267  // Red {231, 76, 60}

// Màu gạch Tetris (I, J, L, O, S, T, Z)
#define C_I   0x07FF // Cyan
#define C_J   0x001F // Blue
#define C_L   0xFD20 // Orange
#define C_O   0xFFE0 // Yellow
#define C_S   0x07E0 // Green
#define C_T   0xA01F // Purple
#define C_Z   0xF800 // Red


// Trạng thái chương trình
#define STATE_MENU 		0
#define STATE_START		1
#define STATE_PLAYING 	2
#define STATE_PAUSED 	3
#define STATE_GAMEOVER 	4

#define STATE_SPAWN 10
#define STATE_DROP 11
#define STATE_SCORE 12
#define STATE_STOP_PLAYING 13

// Số lượng
#define NUM_NEXTBLOCK 2

// --- CẤU TRÚC DỮ LIỆU ---
typedef struct {
    uint16_t x, y, w, h;
    uint16_t color;
    bool isHovered; // Nhấn cảm ứng
} Button;

typedef enum {
	I = 0, J, L, O, S, T, Z
} BlockType;

typedef struct {
	uint16_t x;
	uint16_t y;
} Point;
/*
 Part position:
	 0 {0, 0} 	1 {1, 0}
	 2 {0, 1} 	3 {1, 1}
	 4 {0, 2} 	5 {1, 2}
	 6 {0, 3}	7 {1, 3}
*/
typedef struct {
	BlockType type;
	Point part[4];
} Block;

typedef struct NextQueue {
	BlockType queue[NUM_NEXTBLOCK];
	uint8_t headIndex;
	uint8_t num;
	int (*pop_front)(struct NextQueue* this);
	int (*push_back)(struct NextQueue* this, BlockType type);
} NextQueue;

// --- HÀM API ---
void Tetris_Init(void);
void Tetris_Run(void);

#endif /* INC_TETRIS_GAME_ */
