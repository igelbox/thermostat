#include "timer.hpp"

#include <RTClib.h>

RTC_DS1307 tm;

namespace timer {
  bool initialize() {
    tm.begin();
    return tm.isrunning();
  }

  Time now() {
    DateTime now = tm.now();
    return Time(now.hour(), now.minute());
  }

  void adjust(const Time &time) {
    tm.adjust(DateTime(0, 0, 0, time.hour(), time.minute()));
  }
}
