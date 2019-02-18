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
class Time: public ValueTimeHHMM<uint16_t&, LimitTime> {
  typedef ValueTimeHHMM<uint16_t&, LimitTime> super;
public:
  using super::value;
  using super::ValueTimeHHMM;
};

#pragma pack(1)
struct NVMem_TimeSpan {
  uint16_t on, off;
  bool is_active(uint16_t time) const {
    return (time >= on) && (time < off);
  }
};

#pragma pack(1)
struct NVMem {
  NVMem_TimeSpan light;
  NVMem_TimeSpan heat[2];
} nvmem;

Time miLight0_On(nvmem.light.on), miLight0_Off(nvmem.light.off);
List::Item miiLight0[2] = {
  { "Вкл:   ", miLight0_On },
  { "Откл:  ", miLight0_Off },
};
auto miLight0 = List::from(miiLight0);

Time miHeat0_On(nvmem.heat[0].on), miHeat0_Off(nvmem.heat[0].off);
List::Item miiHeat0[2] = {
  { "Вкл:   ", miHeat0_On },
  { "Откл:  ", miHeat0_Off },
};
auto miHeat0 = List::from(miiHeat0);

Time miHeat1_On(nvmem.heat[1].on), miHeat1_Off(nvmem.heat[1].off);
List::Item miiHeat1[2] = {
  { "Вкл:   ", miHeat1_On },
  { "Откл:  ", miHeat1_Off },
};
auto miHeat1 = List::from(miiHeat1);
List::Item miiHeat[] = {
  { "Утро>  ", miHeat0 },
  { "Вечер> ", miHeat1 },
};
auto miHeat = List::from(miiHeat);

uint16_t time;
Time miTime(time);
List::Item miiMain[] = {
  { "Время: ", miTime },
  { "Свет>  ", miLight0 },
  { "Грев>  ", miHeat },
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
  Serial.begin(9600);

  display::initialize();
  print("Init...");
  println();

  List::BACK_TEXT = "Назад";

  pciSetup(ENC_INC);
  pciSetup(ENC_DEC);
  pciSetup(ENC_BTN);
  pinMode(RLY_LIGHT, OUTPUT);
  pinMode(RLY_HEAT, OUTPUT);

  Wire.begin();
  if (!timer::initialize()) {
    print("Error: timer");
    return;
  };
  EEPROM.get(0, nvmem);

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
  time = clampHHMM(timer::now().total_minutes());
  NVMem prev_mem = nvmem;
  uint16_t prev_time = time;
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
  if (time != prev_time) {
    timer::adjust(timer::Time(time));
  }
  if (memcmp(&nvmem, &prev_mem, sizeof(nvmem))) {
    Serial.println('w');
    EEPROM.put(0, nvmem);
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
  bool light = nvmem.light.is_active(time);
  digitalWrite(RLY_LIGHT, light ? HIGH : LOW);
  bool heat = nvmem.heat[0].is_active(time) || nvmem.heat[1].is_active(time);
  digitalWrite(RLY_HEAT, heat ? HIGH : LOW);
}
