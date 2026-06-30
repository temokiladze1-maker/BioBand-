/* ============================================================
   XIAO ESP32-C3  —  SENSOR NODE  (NimBLE version)
   ------------------------------------------------------------
   Same job as before, but uses the lightweight NimBLE stack
   for a stable connection to the DevKit.

   Sensors:
     - TMP117    (I2C 0x48)  -> skin temperature
     - MAX30102  (I2C 0x57)  -> heart rate (BPM)

   Libraries to install:
     - NimBLE-Arduino            (by h2zero)
     - SparkFun MAX3010x Pulse and Proximity Sensor Library
     - Adafruit TMP117
     - Adafruit BusIO

   Board: XIAO_ESP32C3
   Partition: Huge APP (3MB No OTA/1MB SPIFFS)

   Wiring (unchanged):
     SDA -> D4 (GPIO6)   SCL -> D5 (GPIO7)
     VIN -> 3V3          GND -> GND
   ============================================================ */

#include <Wire.h>
#include <Adafruit_TMP117.h>
#include "MAX30105.h"
#include "heartRate.h"

#include <NimBLEDevice.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"

Adafruit_TMP117 tmp117;
MAX30105        particleSensor;
bool tmpOK = false;

NimBLECharacteristic* pCharacteristic;
bool deviceConnected = false;

// ---- Heart-rate state ----
const byte RATE_SIZE = 8;
byte  rates[RATE_SIZE];
byte  rateSpot = 0;
long  lastBeat = 0;
float beatsPerMinute = 0;
int   beatAvg = 0;
float skinTempC = 0;

// ---- Server callbacks (NimBLE style) ----
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* s, NimBLEConnInfo& info) override {
    deviceConnected = true;
    Serial.println("DevKit connected");
  }
  void onDisconnect(NimBLEServer* s, NimBLEConnInfo& info, int reason) override {
    deviceConnected = false;
    Serial.println("DevKit disconnected, re-advertising");
    NimBLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  delay(300);
  Wire.begin();

  if (!tmp117.begin(0x48, &Wire)) {
    Serial.println("ERROR: TMP117 not found.");
    tmpOK = false;
  } else {
    Serial.println("TMP117 OK");
    tmpOK = true;
  }

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("ERROR: MAX30102 not found.");
  } else {
    Serial.println("MAX30102 OK");
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
  }

  // ---- NimBLE server ----
  NimBLEDevice::init("XIAO_Health_Node");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);          // max TX power -> better range/stability

  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  pService->start();

  NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
  pAdv->addServiceUUID(SERVICE_UUID);
  pAdv->enableScanResponse(true);
  NimBLEDevice::startAdvertising();
  Serial.println("BLE advertising as XIAO_Health_Node");
}

void readHeartRate() {
  long irValue = particleSensor.getIR();
  if (irValue < 50000) { beatAvg = 0; return; }

  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60.0 / (delta / 1000.0);
    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      int sum = 0;
      for (byte x = 0; x < RATE_SIZE; x++) sum += rates[x];
      beatAvg = sum / RATE_SIZE;
    }
  }
}

void loop() {
  readHeartRate();

  static unsigned long lastSend = 0;
  if (millis() - lastSend >= 500) {
    lastSend = millis();

    if (tmpOK) {
      sensors_event_t temp;
      if (tmp117.getEvent(&temp)) skinTempC = temp.temperature;
    }

    long ir = particleSensor.getIR();
    bool finger = ir > 50000;

    char payload[96];
    snprintf(payload, sizeof(payload),
             "{\"bpm\":%d,\"temp\":%.2f,\"finger\":%s}",
             beatAvg, skinTempC, finger ? "true" : "false");

    if (deviceConnected) {
      pCharacteristic->setValue((uint8_t*)payload, strlen(payload));
      pCharacteristic->notify();
    }
    Serial.println(payload);
  }
}
