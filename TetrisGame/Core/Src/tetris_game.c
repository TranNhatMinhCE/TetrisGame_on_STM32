#include <tetris_game.h>

// --- PHƯƠNG THỨC CHO STRUCT QUEUE ---
int pop_front(NextQueue* this);
int push_back(NextQueue* this, BlockType type);
// --- HÀM TẠO SÔ NGẪU NHIÊN ---
int Get_Random_Range(int min, int max);

// --- DỮ LIỆU FONT BITMASK ---
const uint8_t FONT_ALPHA[26][5] = {
    {14,17,31,17,17}, {30,17,30,17,30}, {14,17,16,17,14}, {28,18,17,18,28}, {31,16,30,16,31},
    {31,16,30,16,16}, {14,17,23,17,14}, {17,17,31,17,17}, {14,4,4,4,14},    {31,4,4,20,12},
    {17,18,28,18,17}, {16,16,16,16,31}, {17,27,21,17,17}, {17,25,21,19,17}, {14,17,17,17,14},
    {30,17,30,16,16}, {14,17,17,21,14}, {30,17,30,20,18}, {15,16,14,1,30},  {31,4,4,4,4},
    {17,17,17,17,14}, {17,17,17,10,4},  {17,17,21,21,10}, {17,10,4,10,17},  {17,10,4,4,4}, {31,2,4,8,31}
};
const uint8_t FONT_NUM[10][5] = {
    {14,17,17,17,14}, {4,12,4,4,14}, {31,1,31,16,31}, {31,1,15,1,31}, {17,17,31,1,1},
    {31,16,31,1,31}, {31,16,31,17,31}, {31,1,2,4,4}, {31,17,31,17,31}, {31,17,31,1,31}
};
const uint8_t LOGO_PATTERNS[6][5] = {
    {31, 4, 4, 4, 4}, {31, 16, 30, 16, 31}, {31, 4, 4, 4, 4},
    {30, 17, 30, 17, 17}, {14, 4, 4, 4, 14}, {15, 16, 14, 1, 30}
};
const uint16_t LOGO_COLORS[6] = { C_T, C_I, C_Z, C_J, C_L, C_S }; // Màu tương ứng T E T R I S

// --- BIẾN TOÀN CỤC GAME ---
// 7 loại gạch
Block blockSet[7] = {
	{I, {{1, 0}, {1, 1}, {1, 2}, {1, 3}}},
	{J, {{1, 0}, {1, 1}, {1, 2}, {0, 2}}},
	{L, {{0, 0}, {1, 0}, {1, 1}, {1, 2}}},
	{O, {{0, 0}, {1, 0}, {0, 1}, {1, 1}}},
	{S, {{1, 0}, {1, 1}, {0, 1}, {0, 2}}},
	{T, {{1, 0}, {1, 1}, {0, 1}, {1, 2}}},
	{Z, {{0, 0}, {0, 1}, {1, 1}, {1, 2}}}
};

uint16_t blockColor[7] = {
		0x07FF, // Cyan
		0x001F, // Blue
		0xFD20, // Orange
		0xFFE0, // Yellow
		0x07E0, // Green
		0xA01F, // Purple
		0xF800 // Red
};

#define rnd				Get_Random_Range(0, 6)
#define DEF_TIME_DROP	500 // 0.5s
#define MIN_TIME		10

#define BASE_ADD	789
uint16_t score = 0;
uint16_t highScore = 0;

uint16_t field[ROWS][COLS] = {0}; // Lưới game logic
Block currentBlock;
Block preBlock;
NextQueue nextBlocks = {{0}, 0, 0, &pop_front, &push_back};

int dx = 0;
int dy = 0;
uint8_t rotate = 0; // Mặc định không xoay
uint8_t move = 0;

uint8_t state = STATE_MENU;
uint8_t playingState = STATE_SPAWN;

// Nút bấm
Button btnStart = {70, 220, 100, 30, C_BTN_GREEN, false};
Button btnResume =  {0, 0, 120, 30, C_BTN_BLUE, false};
Button btnRestart = {0, 0, 120, 30, C_BTN_RED, false};
Button btnPause = {SIDEBAR_X + 10, OFFSET_Y + 5, SIDEBAR_X + 40, OFFSET_Y + 30, C_BTN_RED, false};

uint8_t isButtonTouch(Button *btn) {
	if(!touch_IsTouched()) btn->isHovered = 0;
	else {
		btn->isHovered = touch_GetX() > btn->x && touch_GetX() < (btn->x + btn->w)
								&& touch_GetY() > btn->y && touch_GetY() < (btn->y + btn->h);
	}
	return btn->isHovered;
}

// --- HÀM HỖ TRỢ VẼ (WRAPPER) ---
// Vẽ 1 "pixel to" (thực chất là hình vuông nhỏ)
void DrawBigPixel(uint16_t x, uint16_t y, uint8_t size, uint16_t color) {
    lcd_Fill(x, y, x + size - 1, y + size - 1, color);
}

// Vẽ ký tự từ Bitmask
void DrawCharBitmap(uint16_t x, uint16_t y, uint8_t size, const uint8_t pattern[5], uint16_t color) {
    for (int r = 0; r < 5; r++) {
        for (int c = 0; c < 5; c++) {
            if ((pattern[r] >> (4 - c)) & 1) {
                DrawBigPixel(x + c * size, y + r * size, size, color);
            }
        }
    }
}

// Vẽ chuỗi ký tự Pixel
void DrawPixelString(const char *text, uint16_t x, uint16_t y, uint8_t size, uint16_t color) {
    while (*text) {
        char c = *text;
        if (c >= 'A' && c <= 'Z') {
            DrawCharBitmap(x, y, size, FONT_ALPHA[c - 'A'], color);
        } else if (c >= '0' && c <= '9') {
            DrawCharBitmap(x, y, size, FONT_NUM[c - '0'], color);
        }
        x += size * 6; // Khoảng cách giữa các chữ
        text++;
    }
}

// Vẽ chuỗi số nguyên
void DrawPixelNum(int num, uint16_t x, uint16_t y, uint8_t size, uint16_t color) {
    char buffer[10];
    sprintf(buffer, "%d", num); // Cần include <stdio.h>
    DrawPixelString(buffer, x, y, size, color);
}

// --- HÀM VẼ MÀN HÌNH CHÍNH ---
#define DRAW_SCORE()	lcd_Fill(SIDEBAR_X + 10, OFFSET_Y + 65, 240, OFFSET_Y + 80 ,C_BG); \
						DrawPixelNum(score, SIDEBAR_X + 10, OFFSET_Y + 65, 2, C_TXT_VAL);

// Vẽ Logo TETRIS
void DrawLogo(uint16_t x, uint16_t y, uint8_t size) {
    for (int i = 0; i < 6; i++) {
        DrawCharBitmap(x, y, size, LOGO_PATTERNS[i], LOGO_COLORS[i]);
        x += size * 6;
    }
}

// Vẽ Nút bấm
void DrawButton(Button btn, const char *text) {
    // Vẽ bóng đổ
    lcd_Fill(btn.x, btn.y + 4, btn.x + btn.w - 1, btn.y + btn.h + 4 - 1, 0x0000); // Đen mờ làm bóng
    // Vẽ thân nút
    lcd_Fill(btn.x, btn.y, btn.x + btn.w - 1, btn.y + btn.h - 1, btn.color);

    // Căn giữa text (ước lượng)
    uint16_t textLen = strlen(text) * 2 * 6; // size 2
    uint16_t textX = btn.x + (btn.w - textLen) / 2;
    uint16_t textY = btn.y + (btn.h - 10) / 2;
    DrawPixelString(text, textX, textY, 2, C_TXT_VAL);
}

// Hàm vẽ một hộp thoại nổi (Nền tối, viền sáng)
void DrawDialogBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    // Vẽ nền hộp
    lcd_Fill(x, y, x + w - 1, y + h - 1, C_NEXT_BG);
    // Vẽ viền hộp
    lcd_DrawRectangle(x, y, x + w - 1, y + h - 1, C_GRID);
    // Vẽ thêm 1 viền bóng mờ bên ngoài cho đẹp (tùy chọn)
    lcd_DrawRectangle(x-1, y-1, x + w, y + h, C_BG);
}

void DrawCell(uint16_t x, uint16_t y, uint16_t color) {
	uint16_t X = x * BLOCK_SIZE + 1;
	uint16_t Y = OFFSET_Y + y * BLOCK_SIZE + 1;
	lcd_Fill(X, Y, X + BLOCK_SIZE - 2, Y + BLOCK_SIZE - 2, color);
}

void DrawBlock(Block *block, uint16_t color) {
	if(color == 0) color = blockColor[block->type];
	for(uint8_t i = 0; i < 4; i++)
        DrawCell(block->part[i].x, block->part[i].y, color);
}

void DrawCellMap() {
	uint16_t color;
    for (uint16_t y = 0; y < ROWS; y++)
        for (uint16_t x = 0; x < COLS; x++)
            if (field[y][x] != 0) {
                color = blockColor[field[y][x] - 1];
                DrawCell(x, y, color);
            }
            else {
            	DrawCell(x, y, C_BG);
            }
}

void DrawNextBlock() {

    lcd_Fill(NEXTBOX_X, NEXTBOX_Y, NEXTBOX_X + NEXTBOX_W - 1, NEXTBOX_Y + NEXTBOX_H - 1, C_NEXT_BG);
    lcd_DrawRectangle(NEXTBOX_X, NEXTBOX_Y, NEXTBOX_X + NEXTBOX_W - 1, NEXTBOX_Y + NEXTBOX_H - 1, C_GRID);

	uint16_t x = NEXTBOX_X + 25, y = NEXTBOX_Y + 15;
	uint8_t count = nextBlocks.num;
	uint8_t i = nextBlocks.headIndex;
	BlockType type;
	uint16_t drawX, drawY;
	while (count > 0) {
		type = nextBlocks.queue[i];
		for(uint8_t j = 0; j < 4; j++) {
			drawX = (blockSet[type].part[j].x * BLOCK_SIZE) + x;
			drawY = (blockSet[type].part[j].y * BLOCK_SIZE) + y;
			lcd_Fill(drawX, drawY, drawX + BLOCK_SIZE - 2, drawY + BLOCK_SIZE - 2, blockColor[type]);
		}
		if(++i == NUM_NEXTBLOCK) i = 0;
		count--;
		y += 5*BLOCK_SIZE ;
	}
}

void DrawSidebar(void) {
    // Nút Menu (3 gạch)
    uint16_t mx = SIDEBAR_X + 15;
    uint16_t my = OFFSET_Y + 10;
    for(int i = 0; i < 3; i++) {
        lcd_Fill(mx, my + i*6, mx + 20, my + i*6 + 3, 0x7BEF); // Màu xám
    }

    // SCORE
    DrawPixelString("SCORE", SIDEBAR_X + 10, OFFSET_Y + 50, 2, C_TXT_LBL);

    // NEXT BOX
    DrawPixelString("NEXT", SIDEBAR_X + 10, OFFSET_Y + 120, 2, C_TXT_LBL);

    lcd_Fill(NEXTBOX_X, NEXTBOX_Y, NEXTBOX_X + NEXTBOX_W - 1, NEXTBOX_Y + NEXTBOX_H - 1, C_NEXT_BG);
    lcd_DrawRectangle(NEXTBOX_X, NEXTBOX_Y, NEXTBOX_X + NEXTBOX_W - 1, NEXTBOX_Y + NEXTBOX_H - 1, C_GRID);
}

void DrawGrid(void) {
    // Vẽ lưới
    for (int i = 0; i <= COLS; i++) {
        lcd_DrawLine(i * BLOCK_SIZE, OFFSET_Y, i * BLOCK_SIZE, OFFSET_Y + ROWS * BLOCK_SIZE, C_GRID);
    }
    for (int i = 0; i <= ROWS; i++) {
        lcd_DrawLine(0, OFFSET_Y + i * BLOCK_SIZE, COLS * BLOCK_SIZE, OFFSET_Y + i * BLOCK_SIZE, C_GRID);
    }
}

// --- HÀM VẼ MÀN HÌNH CHO CÁC TRẠNG THÁI ---
void Render_MenuState(void) {
    lcd_Clear(C_BG);
    DrawLogo(MENU_GAMELOGO_X, MENU_GAMELOGO_Y, 6);

    lcd_show_picture(MENU_BKLOGO_X, MENU_BKLOGO_Y, 90, 90, gImageLogo); // Logo Bach Khoa
//    lcd_show_picture(MENU_BKLOGO_X, MENU_BKLOGO_Y, 60, 60, gImageLogo); // Logo Bach Khoa

    DrawPixelString("HIGH SCORE", MENU_HISCORE_X, MENU_HISCORE_Y, 2, 0xFFE0); // Màu vàng
    uint16_t scoreX = (highScore > 999) ? 95 : 105;
    DrawPixelNum(highScore, scoreX, MENU_HISCORE_Y + 20, 3, C_TXT_VAL);

    DrawButton(btnStart, "START");

    uint16_t authorY = btnStart.y + btnStart.h + 20;
    DrawPixelString("DESIGNED BY", 10, authorY, 2, RED);
    DrawPixelString("TRAN NHAT MINH", 40, authorY + 20, 2, RED);
}

void Render_StartState() {
	lcd_Clear(C_BG);
	DrawGrid();
	DrawSidebar();
	DRAW_SCORE();
}

void Render_PlayingState(void) {

	Render_StartState();

    DrawNextBlock();
    DrawCellMap();
    DrawBlock(&currentBlock, 0); // Vẽ Block đang rơi
}

void Render_PausedState(void) {
    // 1. Vẽ lại trạng thái game đang chơi (để làm nền)
    // 2. Tính toán vị trí hộp thoại (Căn giữa màn hình 240x320)
    // 3. Vẽ hộp thoại
    DrawDialogBox(PAUSE_BOX_X, PAUSE_BOX_Y, PAUSE_BOX_W, PAUSE_BOX_H);

    // 4. Vẽ tiêu đề "PAUSED"
    // Căn giữa text: boxX + (boxW - text_width)/2. Text width "PAUSED" size 2 ~= 70px
    DrawPixelString("PAUSED", PAUSE_BOX_X + 45, PAUSE_BOX_Y + 15, 2, C_TXT_VAL);

    // 5. Cập nhật vị trí và vẽ các nút
    // Nút Resume
    btnResume.x = PAUSE_BOX_X + 20; // Căn giữa hộp: 40 + (160-120)/2 = 60
    btnResume.y = PAUSE_BOX_Y + 50;
    DrawButton(btnResume, "RESUME");

    // Nút Restart
    btnRestart.x = PAUSE_BOX_X + 20;
    btnRestart.y = PAUSE_BOX_Y + 90;
    DrawButton(btnRestart, "RESTART");
}

void Render_GameOverState(void) {
    // 1. Vẽ lại trạng thái game cuối cùng làm nền
    // 2. Hộp thoại cao hơn một chút để chứa điểm số
    // 3. Vẽ hộp thoại
    DrawDialogBox(GO_BOX_X, GO_BOX_Y, GO_BOX_W, GO_BOX_H);

    // 4. Tiêu đề "GAME OVER" (Màu đỏ)
    DrawPixelString("GAME OVER", GO_BOX_X + 30, GO_BOX_Y + 15, 2, C_BTN_RED);

    // 5. Hiển thị điểm số cuối cùng
//    DrawPixelString("SCORE", GO_BOX_X + 50, GO_BOX_Y + 35, 2, C_TXT_LBL);

    // Căn giữa số điểm (ước lượng tùy số chữ số, ở đây căn cho 3-4 chữ số)
    uint16_t scoreX = (score > 999) ? GO_BOX_X + 55 : GO_BOX_X + 65;
    DrawPixelNum(score, scoreX, GO_BOX_Y + 40, 2, C_TXT_VAL);

    // 6. Nút Restart (Dùng lại nút btnRestart)
    btnResume.x = GO_BOX_X + 20;
    btnResume.y = GO_BOX_Y + 70;
    DrawButton(btnResume, "RESTART");

    btnRestart.x = GO_BOX_X + 20;
    btnRestart.y = GO_BOX_Y + 110;
    DrawButton(btnRestart, "QUIT");
}

// ------------- HÀM PHỤ TRỢ ------------------
int pop_front(NextQueue* this) {
	if(this->num == 0) return -1;
	BlockType ret = this->queue[this->headIndex];
	if(++this->headIndex >= NUM_NEXTBLOCK) this->headIndex = 0;
	this->num--;
	return ret;
}

int push_back(NextQueue* this, BlockType type) {
	if(this->num == NUM_NEXTBLOCK) return -1;
	uint8_t nextSlot = this->headIndex + this->num;
	if(nextSlot >= NUM_NEXTBLOCK) nextSlot -= NUM_NEXTBLOCK;
	this->queue[nextSlot] = type;
	this->num++;
	return 0;
}
void Random_Init(void) {
    uint32_t seed_value = 0;

    // 1. Bắt đầu ADC
    HAL_ADC_Start(&hadc1);

    // 2. Đợi chuyển đổi xong
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        // 3. Lấy giá trị nhiễu từ chân thả nổi làm Seed
        seed_value = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);

    // 4. Cài đặt hạt giống cho bộ sinh số ngẫu nhiên
    srand(seed_value);
}

int Get_Random_Range(int min, int max) {
    return min + rand() % (max - min + 1);
}

static void resetButton() {
	for(uint8_t i = 0; i < 5; i++) {
		isButtonPressed(i);
		isButtonLongPressed(i);
	}
}

static void clearBitField() {
	for(uint8_t i = 0; i < ROWS; i++)
		for(uint8_t j = 0; j < COLS; j++)
			field[i][j] = 0;
}

static void spawnBlock() {
	 int temp = nextBlocks.pop_front(&nextBlocks);
	 if(temp != -1) currentBlock = blockSet[temp];

	 nextBlocks.push_back(&nextBlocks, rnd);

	 for(uint8_t i = 0; i < 4; i++) currentBlock.part[i].x += 3;
}

static uint8_t check_and_update_newPos() {
	Point pointSet[4];
// Check
	if(!rotate) {
		for(uint8_t i = 0; i < 4; i++) {
			pointSet[i] = currentBlock.part[i];
			pointSet[i].x += dx;
			pointSet[i].y += dy;
			if (pointSet[i].x < 0 || pointSet[i].x >= COLS || pointSet[i].y >= ROWS) return 0;
			if (field[pointSet[i].y][pointSet[i].x] != 0) return 0;
		}
	}
	else {
        Point pivot = currentBlock.part[1];
        int x, y;
        for(uint8_t i = 0; i < 4; i++) {
        	pointSet[i] = currentBlock.part[i];
        	if (i == 1) continue;
            x = pointSet[i].y - pivot.y;
            y = pointSet[i].x - pivot.x;
            pointSet[i].x = pivot.x - x;
            pointSet[i].y = pivot.y + y;
			if (pointSet[i].x < 0 || pointSet[i].x >= COLS || pointSet[i].y >= ROWS) return 0;
			if (field[pointSet[i].y][pointSet[i].x] != 0) return 0;
        }
	}
// Update if position is valid
	for(uint8_t i = 0; i < 4; i++) {
		preBlock.part[i] = currentBlock.part[i];
		currentBlock.part[i] = pointSet[i];
	}

    return 1;
}

void Playing()
{
	switch(playingState) {
		case STATE_SPAWN:
			spawnBlock();
		    DrawNextBlock();
			DrawBlock(&currentBlock, 0);
			if(!check_and_update_newPos()) {
				Render_GameOverState();
				playingState = STATE_STOP_PLAYING;
				return;
			}
//			resetButton();
			playingState = STATE_DROP;
			break;
		case STATE_DROP:
			if(is_timer2flag_set(1)) {
				timer2_set(1, DEF_TIME_DROP);
				dy += 1;
				move = 1;
			}
			if(isButtonPressed(0) || isButtonLongPressed(0)) { dx -= 1; move = 1; }
			if(isButtonPressed(1) || isButtonLongPressed(1)) { dx += 1; move = 1; }
			if(isButtonPressed(2) || isButtonLongPressed(2)) {
				if(get_counter(1) > MIN_TIME) timer2_set(1, MIN_TIME);
			}

			if(isButtonPressed(3) || isButtonLongPressed(3)) { rotate += 1; move = 1; }

			if(move) {
				uint8_t check = check_and_update_newPos();
				if(check) {
					DrawBlock(&preBlock, C_BG);
					DrawBlock(&currentBlock, 0);
				}
				else if(!check && dy > 0) playingState = STATE_SCORE;

				move = 0;
				rotate = dx = dy = 0;
			}
			break;
		case STATE_SCORE:

			for(uint8_t i = 0; i < 4; i++) field[currentBlock.part[i].y][currentBlock.part[i].x] = currentBlock.type + 1;

			// Check if player SCORE
			int k = ROWS - 1;
			for(int i = ROWS-1; i > 0; i--) {
				int c = 0;
				for(int j = 0; j < COLS; j++) if(field[i][j]) c++;
				if(c == COLS) {
					score += 10;
				}
				else {
					for(int j = 0; j < COLS; j++) {
						DrawCell(j, i, C_BG);
						DrawCell(j, k, blockColor[field[i][j] - 1]);
						field[k][j] = field[i][j];
					}
					k--;
				}
			}
			DRAW_SCORE();
			playingState = STATE_SPAWN;
			break;
		case STATE_STOP_PLAYING:
			break;
		default:
			break;
	}

}

// --- HÀM PUBLIC ---
void Tetris_Init(void) {
    lcd_init();
    timer_init();
    button_init();
	touch_init(); // Phải khởi tạo sau LCD
	HAL_ADC_Start(&hadc1);
    lcd_Clear(C_BG); // Xóa màn hình bằng màu nền xanh đen
	at24c_Read(BASE_ADD, (uint8_t*)(&highScore), 2);
}

void Tetris_Run(void) {
    static uint8_t oneShot = 1;

    switch(state)
    {
    	case STATE_MENU:
    	    if(oneShot) {
    	        Render_MenuState();
    	        oneShot = 0;
    	    }
    		if(isButtonPressed(4)) state = STATE_START;
    		if(isButtonTouch(&btnStart)) state = STATE_START;
    		break;
    	case STATE_START:
    	    Render_StartState();

			score = 0;
    	    clearBitField();
    		while(nextBlocks.num < NUM_NEXTBLOCK) nextBlocks.push_back(&nextBlocks, rnd);
    		dx = dy = 0;
    		rotate = move = 0;

    		state = STATE_PLAYING; playingState = STATE_SPAWN;

    		timer2_set(1, DEF_TIME_DROP);
    		resetButton();

    		break;
    	case STATE_PLAYING:
    		if(isButtonPressed(4) || isButtonTouch(&btnPause)) {
    			Render_PausedState();
    			state = STATE_PAUSED;
    			break;
    		}
    		if(isButtonPressed(5)) {
    			state = STATE_START;
    			break;
    		}

    		Playing();

			if(playingState == STATE_STOP_PLAYING) {
				if(score > highScore) {
					highScore = score;
					at24c_Write(BASE_ADD, (uint8_t*)(&highScore), 2);
				}
				Render_GameOverState();
				state = STATE_GAMEOVER;
			}

    		break;
    	case STATE_PAUSED:
    		if(isButtonPressed(4) || isButtonTouch(&btnResume)) {
    			Render_PlayingState();
    			state = STATE_PLAYING;
    			break;
    		}
    		if(isButtonPressed(5) || isButtonTouch(&btnRestart)) state = STATE_START;
    		break;
    	case STATE_GAMEOVER:
    		if(isButtonPressed(4) || isButtonTouch(&btnRestart)) {
    			state = STATE_MENU;
    			oneShot = 1;
    			break;
    		}
    		if(isButtonPressed(5) || isButtonTouch(&btnResume)) state = STATE_START;
    		break;
    	default:
    		break;
    }
}
