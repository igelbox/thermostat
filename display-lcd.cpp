#include "display.hpp"
#include "config.hpp"

#include <LiquidCrystal_1602_RUS.h>

LiquidCrystal_1602_RUS lcd(LCD_RS, LCD_EE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
int row;

namespace display {
  void initialize() {
    lcd.begin(16, 2);
  }

  void clear() {
    lcd.clear();
  }

  void reset() {
    lcd.setCursor(0, row = 0);
  }

  void print(const char *message) {
    lcd.print(message);
  }

  void print(char character) {
    lcd.print(character);
  }

  void print(float value, int digits) {
    lcd.print(value, digits);
  }

  void println() {
    lcd.setCursor(0, ++row);
  }
}
