#pragma once

#include <stdint.h>

namespace timer {
  class Time {
    uint16_t value;
  public:
    Time(uint8_t hour, uint8_t minute);
    explicit Time(uint16_t total_minutes);
    uint8_t hour() const;
    uint8_t minute() const;
    uint16_t total_minutes() const;
  };

  bool initialize();
  Time now();
  void adjust(const Time &time);
}
