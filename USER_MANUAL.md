# User Manual — Health-Driven Smart AC Control

This manual lets a competent user set up, configure, and operate the system from scratch. It also serves as **Appendix D** of the project report.

---

## 1. Required tools and libraries

**Software**
- Arduino IDE 2.x
- ESP32 board support (Boards Manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`)

**Arduino libraries** (Library Manager)
- `NimBLE-Arduino` (by h2zero)
- `Adafruit TMP117`
- `Adafruit BusIO` (dependency)
- `SparkFun MAX3010x Pulse and Proximity Sensor Library`

---

## 2. Hardware assembly

Connect both sensors to the XIAO on a shared I2C bus:

| Sensor pin | XIAO pin |
|-----------|----------|
| SDA | D4 (GPIO6) |
| SCL | D5 (GPIO7) |
| VIN | 3V3 |
| GND | GND |

Both sensors' SDA join D4, both SCL join D5, both VIN to 3V3, both GND to GND.
The DevKit needs only USB power (no sensors attached).

---

## 3. Flashing the firmware

### XIAO sensor node
1. Open `firmware/xiao_sensor_node/xiao_sensor_node.ino`.
2. Tools → Board → **XIAO_ESP32C3**.
3. Tools → Partition Scheme → **Huge APP (3MB No OTA/1MB SPIFFS)**.
4. Select the XIAO's port → Upload.
5. Serial Monitor (115200) should show: `TMP117 OK`, `MAX30102 OK`, `BLE advertising as XIAO_Health_Node`.

### DevKit analyzer
1. Open `firmware/devkit_analyzer/devkit_analyzer.ino`.
2. Tools → Board → **ESP32 Dev Module**.
3. Tools → Partition Scheme → **Huge APP (3MB No OTA/1MB SPIFFS)**.
4. Select the DevKit's port → Upload.
5. Serial Monitor should show the Access Point info and `BLE scan started.`

> **Tip:** with both boards plugged in, choose the correct **Port** for each before uploading, or you may flash the wrong board.

---

## 4. Network setup (Access Point mode)

The DevKit creates its own Wi-Fi network — no router or internet needed.

1. After flashing, the DevKit Serial Monitor prints:
   ```
   === Access Point started ===
   1. On your computer, join WiFi:  HealthAC
   2. Password:  health1234
   3. In the simulator, use IP:  192.168.4.1
   ```
2. On your computer, open the Wi-Fi menu and **join `HealthAC`** (password `health1234`).
3. Your computer will show "no internet" — this is normal; the link is direct.

To change the network name or password, edit `AP_SSID` / `AP_PASS` at the top of the DevKit firmware.

---

## 5. Running the simulator

1. Open `simulator/index.html` in a browser (double-click; the address bar should read `file://...`).
2. In the IP field, enter **192.168.4.1**.
3. Click **Connect** — the indicator turns green and live data appears.
4. Place a fingertip gently on the MAX30102. Within a few seconds:
   - **Heart rate** rises from `--` to a real BPM.
   - **Skin temperature** shows the current reading.
   - The **AC target** and **room animation** react to the comfort logic.

---

## 6. Using the system

- **Warm the temperature sensor** (pinch it): skin temp rises, the AC switches toward **COOLING** with a lower setpoint.
- **Remove the finger:** BPM returns to 0 (no reading).
- The **Decision Log** records each change with a timestamp and the reason.

The comfort logic (skin temp + BPM → AC setpoint) is in the `analyze()` function of the DevKit firmware and can be tuned.

---

## 7. Maintenance and troubleshooting

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| `TMP117 NOT found` | loose SDA/SCL/VIN/GND on TMP117 | re-seat the four wires, reset XIAO |
| BPM stays 0 | finger not covering sensor / too much pressure | rest finger lightly and fully over the LED |
| Simulator "no response" | computer not on `HealthAC` | join the AP; use IP `192.168.4.1` |
| DevKit reboot loop | wrong partition scheme | set **Huge APP** and re-flash |
| BLE won't connect | XIAO not advertising | reset XIAO, keep boards within ~1 m |

Firmware updates are done over USB from the Arduino IDE. Each board is modular and can be replaced independently.

---

## 8. Future: connecting a real AC

The DevKit already computes a target setpoint. To drive a real unit, replace the web output in `notifyCallback()` with an IR command (e.g. `IRremoteESP8266`) or a smart-AC API call — the rest of the pipeline is unchanged.
