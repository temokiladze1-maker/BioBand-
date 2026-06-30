/* ============================================================
   TEST 2 — SENSOR SANITY CHECK
   ------------------------------------------------------------
   Purpose: confirm both sensors return sensible values before
   integrating BLE. Reads TMP117 temperature and MAX30102 IR/BPM
   and prints them once per second.

   Expected:
     - Temp ~ 20-35 C at room / on skin.
     - IR value jumps above ~50000 when a finger covers MAX30102.
     - BPM stabilises to 60-100 after a few seconds with a finger.

   Libraries: Adafruit TMP117, SparkFun MAX3010x
   Board: XIAO_ESP32C3
   ============================================================ */

#include <Wire.h>
#include <Adafruit_TMP117.h>
#include "MAX30105.h"
#include "heartRate.h"

Adafruit_TMP117 tmp117;
MAX30105 particleSensor;

const byte RATE_SIZE = 8;
byte rates[RATE_SIZE]; byte rateSpot = 0;
long lastBeat = 0; int beatAvg = 0;

void setup() {
  Serial.begin(115200);
  delay(400);
  Wire.begin();

  Serial.println("\n=== Sensor Sanity Check ===");
  if (!tmp117.begin(0x48, &Wire)) Serial.println("TMP117 NOT found");
  else Serial.println("TMP117 OK");

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) Serial.println("MAX30102 NOT found");
  else {
    Serial.println("MAX30102 OK");
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
  }
}

void loop() {
  long ir = particleSensor.getIR();
  if (ir > 50000 && checkForBeat(ir)) {
    long delta = millis() - lastBeat; lastBeat = millis();
    float bpm = 60.0 / (delta / 1000.0);
    if (bpm > 20 && bpm < 255) {
      rates[rateSpot++] = (byte)bpm; rateSpot %= RATE_SIZE;
      int sum = 0; for (byte i=0;i<RATE_SIZE;i++) sum += rates[i];
      beatAvg = sum / RATE_SIZE;
    }
  }

  static unsigned long t = 0;
  if (millis() - t > 1000) {
    t = millis();
    float temp = NAN;
    sensors_event_t e;
    if (tmp117.getEvent(&e)) temp = e.temperature;
    Serial.printf("Temp=%.2f C | IR=%ld | finger=%s | BPM=%d\n",
                  temp, ir, (ir>50000?"yes":"no"), beatAvg);
  }
}
