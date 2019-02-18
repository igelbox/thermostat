#pragma once

namespace buttons {
  void init(uint8_t pin);
  bool read(uint8_t pin, void (*long_click)() = nullptr);
}
