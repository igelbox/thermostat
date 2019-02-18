#include "buttons.hpp"

namespace buttons {
  void init(uint8_t pin) {
    pinMode(pin, INPUT_PULLUP);
  }

  bool read(uint8_t pin, void (*long_click)() = nullptr) {
    if (digitalRead(pin) != LOW) {
      return false;
    }

    unsigned long ms = millis();
    boolean result = true;
    do {
      delay(150);
      unsigned long dt = millis() - ms;
      if (long_click && (dt > 1000)) {
        long_click();
        result = false;
      }
    } while (digitalRead(pin) == LOW);

    return result;
  }
}
