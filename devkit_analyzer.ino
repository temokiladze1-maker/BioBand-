/* ============================================================
   ESP32 DevKit V1  —  ANALYZER + WEB SERVER  (NimBLE, no DB)
   ------------------------------------------------------------
   NimBLE client for a stable link to the XIAO. Lightweight stack
   means no more periodic disconnects, and it frees enough memory
   that the database can be added back later.

   Libraries to install:
     - NimBLE-Arduino   (by h2zero)
     (WiFi + WebServer are in the ESP32 core)

   Board: ESP32 Dev Module
   Partition: Huge APP (3MB No OTA/1MB SPIFFS)
   ============================================================ */

#include <WiFi.h>
#include <WebServer.h>
#include <NimBLEDevice.h>

// ---------- Access Point settings ----------
// The DevKit CREATES this WiFi network. Your laptop joins it directly.
// After flashing, connect your computer to WiFi name "HealthAC" (password below),
// then open the simulator and use IP  192.168.4.1
const char* AP_SSID = "HealthAC";
const char* AP_PASS = "health1234";   // min 8 chars; change if you like

// ---------- BLE identifiers (must match XIAO) ----------
static NimBLEUUID serviceUUID("12345678-1234-1234-1234-1234567890ab");
static NimBLEUUID charUUID   ("abcd1234-5678-90ab-cdef-1234567890ab");

WebServer server(80);

volatile int   latestBpm    = 0;
volatile float latestTemp   = 0;
volatile bool  latestFinger = false;
volatile float acSetpoint   = 24.0;
String         acReason     = "idle";

static const NimBLEAdvertisedDevice* advDevice = nullptr;
static volatile bool doConnect = false;   // set by scan callback
static volatile bool connected = false;
static volatile bool wantRescan = false;  // set by disconnect callback

// ---------- In-memory history (last 20 readings) ----------
struct Reading { unsigned long ts; int bpm; float temp; float sp; String reason; };
const int HIST = 20;
Reading history[HIST];
int histCount = 0, histHead = 0;
void pushHistory(int bpm, float temp, float sp, const String &reason) {
  history[histHead] = { millis(), bpm, temp, sp, reason };
  histHead = (histHead + 1) % HIST;
  if (histCount < HIST) histCount++;
}

/* ---------------- ANALYSIS (unchanged) ---------------- */
float analyze(float skinTemp, int bpm, String &reasonOut) {
  float target = 24.0;
  bool hot  = skinTemp > 35.0;
  bool cold = skinTemp < 32.0;
  bool fast = bpm > 100;
  bool slow = bpm > 0 && bpm < 60;
  if (hot && fast)      { target = 20.0; reasonOut = "Hot skin + high BPM -> cooling hard"; }
  else if (hot)         { target = 21.5; reasonOut = "Hot skin -> cooling"; }
  else if (fast)        { target = 22.0; reasonOut = "Elevated BPM -> mild cooling"; }
  else if (cold && slow){ target = 27.0; reasonOut = "Cold skin + low BPM -> warming"; }
  else if (cold)        { target = 26.0; reasonOut = "Cold skin -> mild warming"; }
  else if (slow)        { target = 25.0; reasonOut = "Low BPM -> slight warming"; }
  else                  { target = 24.0; reasonOut = "Comfortable -> holding"; }
  if (target < 18.0) target = 18.0;
  if (target > 28.0) target = 28.0;
  return target;
}

/* ---------------- BLE notify handler ---------------- */
void notifyCB(NimBLERemoteCharacteristic* c, uint8_t* data, size_t len, bool isNotify) {
  String json = String((char*)data).substring(0, len);
  int bpm = 0; float temp = 0; bool finger = false;
  int bi = json.indexOf("\"bpm\":");
  int ti = json.indexOf("\"temp\":");
  int fi = json.indexOf("finger\":");
  if (bi >= 0) bpm  = json.substring(bi + 6).toInt();
  if (ti >= 0) temp = json.substring(ti + 7).toFloat();
  if (fi >= 0) finger = json.indexOf("true", fi) >= 0;

  latestBpm = bpm; latestTemp = temp; latestFinger = finger;
  String reason;
  float sp = analyze(temp, bpm, reason);
  acSetpoint = sp; acReason = reason;
  pushHistory(bpm, temp, sp, reason);
  Serial.printf("RX bpm=%d temp=%.2f -> AC=%.1f (%s)\n", bpm, temp, sp, reason.c_str());
}

/* ---------------- Client callbacks ---------------- */
class ClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* c) override {
    Serial.println("Connected to XIAO_Health_Node");
    connected = true;
  }
  void onDisconnect(NimBLEClient* c, int reason) override {
    connected = false;
    wantRescan = true;             // handled safely in loop(), not here
    Serial.printf("BLE disconnected (reason %d)\n", reason);
  }
};
static ClientCallbacks clientCB;

/* ---------------- Scan callbacks ---------------- */
class ScanCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* dev) override {
    if (dev->getName() == "XIAO_Health_Node" ||
        (dev->haveServiceUUID() && dev->isAdvertisingService(serviceUUID))) {
      Serial.println(">> Found XIAO_Health_Node");
      NimBLEDevice::getScan()->stop();
      advDevice = dev;
      doConnect = true;
    }
  }
};

bool connectToServer() {
  Serial.println("Connecting to XIAO...");

  // Reuse an existing client if we have one, else create.
  NimBLEClient* pClient = nullptr;
  if (NimBLEDevice::getCreatedClientCount()) {
    pClient = NimBLEDevice::getDisconnectedClient();
  }
  if (!pClient) {
    pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(&clientCB, false);
    pClient->setConnectionParams(12, 24, 0, 200);  // gentler timing -> fewer 520s
    pClient->setConnectTimeout(10 * 1000);
  }

  bool ok = false;
  for (int attempt = 0; attempt < 3 && !ok; attempt++) {
    Serial.printf("  attempt %d\n", attempt + 1);
    ok = pClient->connect(advDevice);
    if (!ok) delay(500);
  }
  advDevice = nullptr;          // pointer is consumed; don't reuse it (prevents crash)

  if (!ok) { Serial.println("connect() failed after retries"); return false; }

  NimBLERemoteService* svc = pClient->getService(serviceUUID);
  if (!svc) { Serial.println("Service not found"); pClient->disconnect(); return false; }

  NimBLERemoteCharacteristic* ch = svc->getCharacteristic(charUUID);
  if (!ch) { Serial.println("Characteristic not found"); pClient->disconnect(); return false; }

  if (ch->canNotify()) ch->subscribe(true, notifyCB);
  return true;
}

/* ---------------- Web server ---------------- */
void handleData() {
  char buf[256];
  snprintf(buf, sizeof(buf),
    "{\"bpm\":%d,\"skin_temp\":%.2f,\"finger\":%s,"
    "\"ac_setpoint\":%.1f,\"reason\":\"%s\"}",
    latestBpm, latestTemp, latestFinger ? "true":"false",
    acSetpoint, acReason.c_str());
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", buf);
}
void handleHistory() {
  String out = "[";
  for (int i = 0; i < histCount; i++) {
    int idx = (histHead - 1 - i + HIST) % HIST;
    Reading &r = history[idx];
    if (i) out += ",";
    out += "{\"ts\":" + String(r.ts) + ",\"bpm\":" + String(r.bpm) +
           ",\"skin_temp\":" + String(r.temp,2) + ",\"ac_setpoint\":" + String(r.sp,1) +
           ",\"reason\":\"" + r.reason + "\"}";
  }
  out += "]";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", out);
}
void handleRoot() { server.send(200, "text/plain", "ESP32 Analyzer (NimBLE). GET /data or /history."); }

void setup() {
  Serial.begin(115200);
  delay(300);

  // ---- Start the DevKit's own WiFi network (Access Point) ----
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(500);
  IPAddress ip = WiFi.softAPIP();
  Serial.println("\n=== Access Point started ===");
  Serial.printf("1. On your computer, join WiFi:  %s\n", AP_SSID);
  Serial.printf("2. Password:  %s\n", AP_PASS);
  Serial.printf("3. In the simulator, use IP:  %s\n", ip.toString().c_str());
  Serial.println("   (this IP never changes)");
  Serial.println("============================");

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/history", handleHistory);
  server.begin();

  // NimBLE
  Serial.println("Starting NimBLE...");
  NimBLEDevice::init("");
  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setScanCallbacks(new ScanCallbacks());
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(99);
  scan->start(0, false, false);   // 0 = scan forever until match
  Serial.println("BLE scan started.");
}

void loop() {
  // Restart scanning after a disconnect (done here, safely, not in callback).
  if (wantRescan && !connected) {
    wantRescan = false;
    Serial.println("Re-scanning...");
    NimBLEDevice::getScan()->start(0, false, false);
  }

  if (doConnect) {
    doConnect = false;
    if (connectToServer()) Serial.println("Subscribed, receiving data.");
    else {
      Serial.println("Connect failed, re-scanning");
      NimBLEDevice::getScan()->start(0, false, false);
    }
  }

  server.handleClient();
  delay(2);
}
