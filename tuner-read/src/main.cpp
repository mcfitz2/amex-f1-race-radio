#include <Arduino.h>
#include <Wire.h>

// RDA5807M read-only poll. Reg access via 0x11 (random), status via 0x10 (seq from 0x0A).
const uint8_t RDA_SEQ = 0x10, RDA_RND = 0x11;

bool readReg(uint8_t reg, uint16_t &val) {
  Wire.beginTransmission(RDA_RND);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(RDA_RND, (uint8_t)2) != 2) return false;
  val = Wire.read() << 8;
  val |= Wire.read();
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);
  Wire.setClock(100000);
  Serial.println("tuner poll; press presets");
}

uint16_t last[3] = {0, 0, 0};
uint8_t last60[5] = {0};

// TEA5767-style read at 0x60: 5 bytes, PLL = byte0[5:0]<<8 | byte1
void poll60() {
  if (Wire.requestFrom((uint8_t)0x60, (uint8_t)5) != 5) return;
  uint8_t b[5];
  for (int i = 0; i < 5; i++) b[i] = Wire.read();
  if (memcmp(b, last60, 5) == 0) return;
  memcpy(last60, b, 5);
  uint16_t pll = ((b[0] & 0x3F) << 8) | b[1];
  Serial.printf("0x60: %02X %02X %02X %02X %02X pll=%u -> hi %.3f MHz / lo %.3f MHz\n",
                b[0], b[1], b[2], b[3], b[4], pll,
                (pll * 32768.0 / 4 - 225000) / 1e6, (pll * 32768.0 / 4 + 225000) / 1e6);
}

void loop() {
  uint16_t r0a, r03, r07;
  poll60();
  if (!readReg(0x0A, r0a) || !readReg(0x03, r03) || !readReg(0x07, r07)) {
    Serial.println("read fail");
    delay(500);
    return;
  }
  if (r0a != last[0] || r03 != last[1] || r07 != last[2]) {
    last[0] = r0a; last[1] = r03; last[2] = r07;
    uint8_t band = (r03 >> 2) & 3, space = r03 & 3;
    uint16_t readchan = r0a & 0x3FF;
    float base = band == 0 ? 87.0 : band == 1 ? 76.0 : band == 2 ? 76.0 : ((r07 & 0x200) ? 65.0 : 50.0);
    float step = space == 0 ? 0.1 : space == 1 ? 0.2 : space == 2 ? 0.05 : 0.025;
    Serial.printf("r0A=%04X r03=%04X r07=%04X band=%u space=%u READCHAN=%u CHAN=%u -> %.3f MHz (tuned %.3f)\n",
                  r0a, r03, r07, band, space, readchan, r03 >> 6,
                  base + (r03 >> 6) * step, base + readchan * step);
  }
  delay(200);
}
