# Setup Guide (from zero)

Step-by-step to get the whole system running, including the test sketches.

## Step 0 — One-time tooling
1. Install Arduino IDE 2.x.
2. File → Preferences → Additional Boards Manager URLs, add:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Boards Manager → install **esp32 by Espressif**.
4. Library Manager → install: **NimBLE-Arduino**, **Adafruit TMP117**, **Adafruit BusIO**, **SparkFun MAX3010x**.

## Step 1 — Wire the sensors (see diagrams/wiring.png)
Both sensors share I2C: SDA→D4, SCL→D5, VIN→3V3, GND→GND.

## Step 2 — Validate with test sketches (recommended before full firmware)
1. `firmware/tests/test_1_i2c_scanner.ino` → confirm `0x48` and `0x57` appear.
2. `firmware/tests/test_2_sensor_check.ino` → confirm temp + BPM read correctly.
3. Flash `firmware/xiao_sensor_node` to the XIAO; then on the DevKit run
   `firmware/tests/test_3_ble_scan.ino` → confirm `XIAO_Health_Node` is seen.

## Step 3 — Flash the real firmware
- XIAO ← `firmware/xiao_sensor_node` (Board: XIAO_ESP32C3, Partition: Huge APP).
- DevKit ← `firmware/devkit_analyzer` (Board: ESP32 Dev Module, Partition: Huge APP).

## Step 4 — Connect and run
1. Join Wi-Fi **HealthAC** / `health1234` on your computer.
2. Open `simulator/index.html`, enter **192.168.4.1**, Connect.
3. Finger on the MAX30102 → live data + AC reaction.

See USER_MANUAL.md for troubleshooting.
