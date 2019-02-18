#pragma once

namespace display {
  void initialize();
  void clear();
  void reset();
  void print(const char *message);
  void print(char character);
  void print(float value, int digits);
  void println();
}
