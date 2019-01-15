#include "Adafruit_AM2320.h"
#include "RTClib.h"
#include <LiquidCrystal.h>

Adafruit_AM2320 th = Adafruit_AM2320();
RTC_DS1307 tm;
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

void setup() {
  // Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("init...");

  Wire.begin();
  th.begin();
  tm.begin();

  lcd.print("ok");
}

#define out lcd

void loop() {
  DateTime now = tm.now();
  lcd.clear();
  lcd.setCursor(0, 0);
  out.print(th.readTemperature());
  out.print(" C, ");
  out.print(th.readHumidity());
  out.print(" H");

  lcd.setCursor(0, 1);
  char buff[13];
  sprintf(buff, "%02d:%02d:%02d.%03d", now.hour(), now.minute(), now.second(), millis() % 1000);
  out.print(buff);
  delay(2000);
}
