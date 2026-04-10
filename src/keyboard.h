//esp-stun modded
//credits for qwen code
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "display.h"
#include "CONFIG.h"

#define KEYBOARD_BUFFER_SIZE 64
#define KB_COLS 10
#define KB_ROWS 4  // 4 строки букв + 1 строка функциональных

struct KeyboardState {
  char buffer[KEYBOARD_BUFFER_SIZE];
  uint8_t bufferPos;
  uint8_t row;  // 0 = функциональные, 1-4 = буквы
  uint8_t col;
  uint8_t lastCol;  // Последний столбец перед функциональными
  bool active;
  bool done;
  unsigned long lastPressTime;  // Время последнего нажатия
  bool isHolding;  // Флаг зажатия
};

// Раскладки: буквы, цифры
extern const char kb_letters[KB_ROWS][KB_COLS][4];
extern const char kb_symbols[KB_ROWS][KB_COLS][4];

void keyboardInit(KeyboardState &kb);
void keyboardRender(DisplayType &display, KeyboardState &kb);
bool keyboardHandleInput(KeyboardState &kb, uint8_t button);
void keyboardHandleHold(KeyboardState &kb);

#endif
