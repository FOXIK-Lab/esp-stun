//esp-stun modded
//credits for qwen code
#include "keyboard.h"
#include <string.h>

// Упрощённая клавиатура для WiFi паролей
// Функциональные: [ DEL ][  SPACE  ][ENTER]
// Row 0: 0 1 2 3 4 5 6 7 8 9
// Row 1: q w e r t y u i o p
// Row 2: a s d f g h j k l z
// Row 3: x c v b n m . / #
// Спецсимволы:
// Row 0: 0 1 2 3 4 5 6 7 8 9
// Row 1: ! @ # $ % ^ & * ( )
// Row 2: - _ = + [ ] { } \ |
// Row 3: ; : ' " , . / ? ~ ABC

bool useSymbols = false;

const char kb_letters[4][10][4] = {
  {"0","1","2","3","4","5","6","7","8","9"},
  {"q","w","e","r","t","y","u","i","o","p"},
  {"a","s","d","f","g","h","j","k","l","z"},
  {"x","c","v","b","n","m",".","/","#"}
};

const char kb_symbols[4][10][4] = {
  {"!","@","#","$","%","^","&","*","(",")"},
  {"-","_","=","+","[","]","{","}","\\","|"},
  {";",":","'","\"",",",".","/","?","~","`"},
  {"<",">","ABC"," "," "," "," "," "," "," "}
};

void keyboardInit(KeyboardState &kb) {
  memset(kb.buffer, 0, KEYBOARD_BUFFER_SIZE);
  kb.bufferPos = 0;
  kb.row = 2;  // Начинаем с букв
  kb.col = 0;
  kb.lastCol = 0;
  kb.active = true;
  kb.done = false;
  kb.lastPressTime = 0;
  kb.isHolding = false;
  useSymbols = false;
}

void keyboardRender(DisplayType &display, KeyboardState &kb) {
  display.clearDisplay();
  
  // Верхняя панель - введённый пароль (14 пикселей)
  display.fillRect(0, 0, 128, 14, WHITE);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(4, 4);
  
  if (kb.bufferPos > 0) {
    int maxLen = 22;
    int start = (kb.bufferPos > maxLen) ? kb.bufferPos - maxLen : 0;
    display.print(&kb.buffer[start]);
    display.print("_");
  } else {
    display.print("Password_");
  }
  
  // Функциональные кнопки сверху
  int funcY = 16;
  int funcH = 10;
  
  // DEL (левая, маленькая)
  bool delSelected = (kb.row == 0 && kb.col == 0);
  if (delSelected) {
    display.fillRect(2, funcY, 26, funcH, WHITE);
    display.setTextColor(BLACK);
  } else {
    display.drawRect(2, funcY, 26, funcH, WHITE);
    display.setTextColor(WHITE);
  }
  display.setCursor(7, funcY + 2);
  display.print("DEL");
  
  // SPACE (средняя)
  bool spaceSelected = (kb.row == 0 && kb.col == 1);
  if (spaceSelected) {
    display.fillRect(30, funcY, 44, funcH, WHITE);
    display.setTextColor(BLACK);
  } else {
    display.drawRect(30, funcY, 44, funcH, WHITE);
    display.setTextColor(WHITE);
  }
  display.setCursor(40, funcY + 2);
  display.print("SPACE");
  
  // ENTER
  bool enterSelected = (kb.row == 0 && kb.col == 2);
  if (enterSelected) {
    display.fillRect(76, funcY, 24, funcH, WHITE);
    display.setTextColor(BLACK);
  } else {
    display.drawRect(76, funcY, 24, funcH, WHITE);
    display.setTextColor(WHITE);
  }
  display.setCursor(79, funcY + 2);
  display.print("ENT");
  
  // BACK (правая)
  bool backSelected = (kb.row == 0 && kb.col == 3);
  if (backSelected) {
    display.fillRect(102, funcY, 24, funcH, WHITE);
    display.setTextColor(BLACK);
  } else {
    display.drawRect(102, funcY, 24, funcH, WHITE);
    display.setTextColor(WHITE);
  }
  display.setCursor(104, funcY + 2);
  display.print("BACK");
  
  // Кнопки клавиатуры (4 строки букв/символов)
  int keyW = 12;
  int keyH = 9;
  int startX = 4;
  int startY = 28;
  
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 10; c++) {
      int x = startX + c * keyW;
      int y = startY + r * keyH;
      
      // Реальная строка = kb.row - 1 (так как row 0 это функциональные)
      int realRow = kb.row - 1;
      bool isSelected = (realRow == r && c == kb.col && kb.row > 0);
      
      if (isSelected) {
        display.fillRect(x, y, keyW - 1, keyH - 1, WHITE);
        display.setTextColor(BLACK);
      } else {
        display.setTextColor(WHITE);
      }
      
      display.setCursor(x + 3, y + 1);
      if (useSymbols) {
        display.print(kb_symbols[r][c]);
      } else {
        display.print(kb_letters[r][c]);
      }
    }
  }
  
  display.display();
}

bool keyboardHandleInput(KeyboardState &kb, uint8_t button) {
  unsigned long currentTime = millis();
  
  switch (button) {
    case 0: { // UP
      kb.isHolding = false;
      if (kb.row == 0) {
        // На функциональных - переходим на предыдущий столбец букв, нижняя строка
        if (kb.lastCol > 0) {
          kb.lastCol--;
          kb.col = kb.lastCol;
          kb.row = 4;  // Нижняя строка букв
        }
      } else if (kb.row == 1) {
        // С первой строки букв - на функциональные
        kb.lastCol = kb.col;  // Сохраняем позицию
        kb.row = 0;
        // Определяем функциональную кнопку по столбцу
        if (kb.col <= 1) kb.col = 0;      // 0-1 -> DEL
        else if (kb.col <= 5) kb.col = 1; // 2-5 -> SPACE
        else if (kb.col <= 7) kb.col = 2; // 6-7 -> ENT
        else kb.col = 3;                  // 8-9 -> BACK
      } else {
        kb.row--;
      }
      break;
    }
      
    case 1: { // DOWN
      kb.isHolding = false;
      if (kb.row == 0) {
        // С функциональных - на первую строку букв
        kb.row = 1;
        kb.col = kb.lastCol;  // Возвращаемся на ту же позицию
      } else if (kb.row < 4) {
        kb.row++;
      } else {
        // На последней строке - переходим на следующий столбец, первая строка
        if (kb.col < 9) {
          kb.col++;
          kb.row = 1;
        }
      }
      break;
    }
      
    case 4: { // OK
      // Функциональные кнопки
      if (kb.row == 0) {
        if (kb.col == 0) {
          // DEL
          if (kb.bufferPos > 0) {
            kb.bufferPos--;
            kb.buffer[kb.bufferPos] = 0;
          }
        } else if (kb.col == 1) {
          // SPACE
          if (kb.bufferPos < KEYBOARD_BUFFER_SIZE - 1) {
            kb.buffer[kb.bufferPos++] = ' ';
            kb.buffer[kb.bufferPos] = 0;
          }
        } else if (kb.col == 2) {
          // ENTER - завершить ввод
          kb.done = true;
          return true;
        } else if (kb.col == 3) {
          // BACK - отмена и возврат
          kb.buffer[0] = 0;
          kb.bufferPos = 0;
          kb.done = true;
          return true;
        }
      } else {
        // Буквы/символы
        int realRow = kb.row - 1;
        const char* chStr;
        
        if (useSymbols) {
          chStr = kb_symbols[realRow][kb.col];
        } else {
          chStr = kb_letters[realRow][kb.col];
        }
        
        // Проверка кнопки переключения на спецсимволы
        if (chStr[0] == '#' || strcmp(chStr, "ABC") == 0) {
          useSymbols = !useSymbols;
          // Перемещаем курсор на первый символ
          kb.row = 1;
          kb.col = 0;
        } else if (chStr[0] != '\0' && kb.bufferPos < KEYBOARD_BUFFER_SIZE - 1) {
          kb.buffer[kb.bufferPos++] = chStr[0];
          kb.buffer[kb.bufferPos] = 0;
        }
        
        // Автоматический переход в следующую колонку
        kb.col++;
        if (kb.col >= 10) {
          kb.col = 0;
        }
      }
      break;
    }
      
    case 6: // BACK - возврат назад (отмена ввода)
      kb.isHolding = false;
      kb.buffer[0] = 0;
      kb.bufferPos = 0;
      kb.done = true;
      return true;
  }
  
  return false;
}

void keyboardHandleHold(KeyboardState &kb) {
  if (!kb.isHolding) return;
  
  unsigned long currentTime = millis();
  // Повторяем каждые 100мс при зажатии
  if (currentTime - kb.lastPressTime < 100) return;
  kb.lastPressTime = currentTime;
  
  // Функциональные кнопки
  if (kb.row == 0) {
    if (kb.col == 0) {
      // DEL - быстрое удаление
      if (kb.bufferPos > 0) {
        kb.bufferPos--;
        kb.buffer[kb.bufferPos] = 0;
      }
    }
  } else {
    // Буквы/символы - быстрое добавление
    int realRow = kb.row - 1;
    const char* chStr = useSymbols ? kb_symbols[realRow][kb.col] : kb_letters[realRow][kb.col];
    
    if (chStr[0] != '\0' && kb.bufferPos < KEYBOARD_BUFFER_SIZE - 1) {
      kb.buffer[kb.bufferPos++] = chStr[0];
      kb.buffer[kb.bufferPos] = 0;
    }
  }
}
