/* ============================================================
   TEST 1 — I2C SCANNER
   ------------------------------------------------------------
   Purpose: verify that both sensors are wired correctly and
   respond on the I2C bus before running the full firmware.

   Expected result on XIAO ESP32-C3 with both sensors connected:
     Found I2C device at 0x48   (TMP117)
     Found I2C device at 0x57   (MAX30102)

   If a device is missing, check that sensor's VIN, GND, SDA, SCL.

   Board: XIAO_ESP32C3   |   Wiring: SDA->D4, SCL->D5
   ============================================================ */

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(500);
  Wire.begin();                 // XIAO default SDA=D4, SCL=D5
  Serial.println("\n=== I2C Scanner ===");
}

void loop() {
  byte count = 0;
  Serial.println("Scanning...");
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  Found I2C device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      if (addr == 0x48) Serial.print("  -> TMP117 (temperature)");
      if (addr == 0x57) Serial.print("  -> MAX30102 (heart rate)");
      Serial.println();
      count++;
    }
  }
  if (count == 0) Serial.println("  No I2C devices found - check wiring!");
  Serial.printf("Done. %d device(s) found.\n\n", count);
  delay(4000);
}
