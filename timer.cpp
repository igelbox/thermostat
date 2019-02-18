#include "timer.hpp"

namespace timer {
  Time::Time(uint8_t hour, uint8_t minute)
    :value(hour) {
      value = value * 60 + minute;
  }

  Time::Time(uint16_t total_minutes)
    :value(total_minutes) {
  }

  uint8_t Time::hour() const {
    return value / 60;
  }

  uint8_t Time::minute() const {
    return value % 60;
  }

  uint16_t Time::total_minutes() const {
    return value;
  }
}
