#include <Arduino.h>
#include <Wire.h>

const uint8_t EEPROM_ADDR = 0x50;

void dump() {
  Serial.println("I2C scan:");
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) Serial.printf("  found 0x%02X\n", a);
  }

  Serial.println("\nEEPROM dump:");
  for (uint16_t base = 0; base < 256; base += 16) {
    Wire.beginTransmission(EEPROM_ADDR);
    Wire.write((uint8_t)base);
    if (Wire.endTransmission(false) != 0) {
      Serial.printf("NACK at 0x%02X\n", base);
      return;
    }
    Wire.requestFrom(EEPROM_ADDR, (uint8_t)16);
    Serial.printf("%02X: ", base);
    for (int i = 0; i < 16 && Wire.available(); i++)
      Serial.printf("%02X ", Wire.read());
    Serial.println();
  }
  Serial.println("DONE");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);
  Wire.setClock(100000);
  dump();
}

void loop() {
  delay(10000);
  Serial.println("\n--- redump ---");
  dump();
}
