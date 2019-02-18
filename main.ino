#include "display.hpp"
#include "config.hpp"
#include "buttons.hpp"
#include "menu.hpp"
#include "timer.hpp"

#include <Wire.h>
#include <RotaryEncoder.h>
#include <EEPROM.h>

using namespace display;
using namespace menu;

void Printer::print(const char *message) { display::print(message); }
void Printer::print(char character) { display::print(character); }
void Printer::print(int value) { display::print(value, 0); }
void Printer::print(float value, int digits) { display::print(value, digits); }
void Printer::println() { print("               "); display::println(); }
menu::Printer out;

typedef ValueIncDecLimiter<uint16_t>::Repeat<0, 24 * 60 - 1> LimitTime;
class Time: public ValueTimeHHMM<uint16_t, LimitTime> {
  typedef ValueTimeHHMM<uint16_t, LimitTime> super;
public:
  using super::value;
  Time(): super(0) {}
};

Time miLight0_On, miLight0_Off;
List::Item miiLight0[2] = {
  { "Вкл:   ", miLight0_On },
  { "Откл:  ", miLight0_Off },
};
auto miLight0 = List::from(miiLight0);

Time miTime;
List::Item miiMain[] = {
  { "Время: ", miTime },
  { "Свет>  ", miLight0 },
};
auto miMain = List::from(miiMain);

void pciSetup(uint8_t pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

uint16_t clampHHMM(uint16_t value) {
  return value >= 24 * 60 ? 24 * 60 - 1 : value;
}

bool run = false;
void setup() {
  // Serial.begin(9600);

  display::initialize();
  print("Init...");
  println();

  List::BACK_TEXT = "Назад";

  pciSetup(ENC_INC);
  pciSetup(ENC_DEC);
  pciSetup(ENC_BTN);
  pinMode(RLY_LIGHT, OUTPUT);

  Wire.begin();
  if (!timer::initialize()) {
    print("Error: timer");
    return;
  };
  EEPROM.get(0, miLight0_On.value);
  miLight0_On.value = clampHHMM(miLight0_On.value);
  EEPROM.get(2, miLight0_Off.value);
  miLight0_Off.value = clampHHMM(miLight0_Off.value);

  menu::focused = &miMain;
  print("OK");
  run = true;
}

RotaryEncoder enc(ENC_INC, ENC_DEC);
ISR(PCINT1_vect) {
  enc.tick();
}

unsigned long last_action_time;
AbstractItem *cleared = nullptr;
int prev_pos = 0;

void loop() {
  if (!run) {
    return;
  }

  unsigned long ms = millis();
  uint16_t time = miTime.value = clampHHMM(timer::now().total_minutes());
  uint16_t time0 = miLight0_On.value, time1 = miLight0_Off.value;
  auto pos = -enc.getPosition();
  auto delta = pos - prev_pos;
  if (delta) {
    prev_pos = pos;
    for (; delta < 0; ++delta) {
      focused->exec('-');
      last_action_time = ms;
    }
    for (; delta > 0; --delta) {
      focused->exec('+');
      last_action_time = ms;
    }
  }
  if (buttons::read(ENC_BTN)) {
    focused->exec('>');
    last_action_time = ms;
  }
  if (miTime.value != time) {
    timer::adjust(timer::Time(miTime.value));
  }
  if (miLight0_On.value != time0) {
    EEPROM.put(0, miLight0_On.value);
  }
  if (miLight0_Off.value != time1) {
    EEPROM.put(2, miLight0_Off.value);
  }
  auto last_action_timeout = ms - last_action_time;
  out.blink = (last_action_timeout / 500) % 2;
  if (cleared != focused) {
    cleared = focused;
    display::clear();
  } else {
    display::reset();
  }
  focused->draw_screen(out, 2);
  bool light = (time >= time0) && (time < time1);
  digitalWrite(RLY_LIGHT, light ? HIGH : LOW);
}
