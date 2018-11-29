#include <DHT.h>
#include <EEPROM.h>

DHT dht(2, DHT22);
unsigned short offset;
unsigned short eeprom_size;

unsigned short ensure_write(byte size) {
  unsigned short prev = offset;
  offset += size;
  if (offset > eeprom_size) {
    prev = 2;
    offset = prev + size;
  }
  return prev;
}

void commit_write() {
  EEPROM.put(0, offset);
}

void reset() {
  Serial.print("EEPROM Erasing ...");
  for (unsigned i = 0; i < eeprom_size; ++i) {
    EEPROM.write(i, 0);
  }
  Serial.println(" OK");
  offset = 2;
}

void printTH(float t, float h) {
  Serial.print("t:");
  Serial.print(t);
  Serial.print(", h:");
  Serial.println(h);
}

void dumpCell(unsigned i) {
  uint8_t t = EEPROM.read(i + 0);
  uint8_t h = EEPROM.read(i + 1);
  if (!t && !h) {
    return;
  }
  float tf = ((float)t) / 2.125f - 40.f;
  float hf = ((float)h) / 2.55f;
  printTH(tf, hf);
}

void dump() {
  for (unsigned i = offset; i < eeprom_size; i += 2) {
    dumpCell(i);
  }
  for (unsigned i = 2; i < offset; i += 2) {
    dumpCell(i);
  }
}

void setup() {
  Serial.begin(9600);

  dht.begin();
  eeprom_size = EEPROM.length();
  EEPROM.get(0, offset);
  Serial.print("EEPROM Offset: ");
  Serial.print(offset);
  Serial.print(", Size: ");
  Serial.println(eeprom_size);
  if (offset == 0xFFFF) {
    reset();
  } else if (!offset) {
    offset = 2;
  }
}

void loop() {
  for (int i = 30 * 60; i > 0 ; --i) {
    if (Serial.available()) {
      String cmd = Serial.readString();
      if (cmd == "?") {
        Serial.println("m, d, c");
        Serial.print("w:");
        Serial.println(i / 60.f);
      } else if (cmd == "m") {
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        printTH(t, h);
      } else if (cmd == "d") {
        dump();
      } else if (cmd == "c") {
        reset();
      }
    }
    delay(1000);
  }
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  printTH(t, h);
  uint8_t i = ensure_write(2);
  EEPROM.write(i + 0, (uint8_t)((t + 40.f) * 2.125f));
  EEPROM.write(i + 1, (uint8_t)(h * 2.55f));
  commit_write();
}
