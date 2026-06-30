# Health-Driven Smart AC Control

A two-microcontroller embedded system that reads a person's **skin temperature** and **heart rate**, analyzes them, and adjusts a simulated air-conditioner setpoint for personalized thermal comfort.

> CE490B Final Project — Ilia State University, School of Technology
> Computer Engineering BSc

---

## System overview

```
TMP117 + MAX30102  --I2C-->  XIAO ESP32-C3  --BLE-->  ESP32 DevKit V1  --Wi-Fi-->  Web AC Simulator
   (sensors)                  (sensor node)            (analyzer + AP)              (browser)
```

- **XIAO ESP32-C3** reads two I2C sensors, computes BPM with a peak-detection algorithm, and advertises the data over Bluetooth Low Energy (BLE).
- **ESP32 DevKit V1** connects as a BLE client, runs the comfort-analysis logic, and serves the result over its own Wi-Fi Access Point.
- **Web simulator** (HTML/JS) polls the DevKit and visualizes sensor data, the AC setpoint, and a reacting room.

![System block diagram](diagrams/block_diagram.png)

---

## Repository structure

```
health-ac/
├── firmware/
│   ├── xiao_sensor_node/      XIAO ESP32-C3 firmware (sensors + BLE server)
│   ├── devkit_analyzer/       ESP32 DevKit firmware (BLE client + Wi-Fi AP + logic)
│   └── tests/                 Standalone test sketches (I2C, sensors, BLE)
├── simulator/                 Web AC simulator (open index.html in a browser)
├── diagrams/                  Block, wiring, flowchart, PCB-concept (SVG + PNG)
├── hardware/                  PCB concept / wiring source files
├── docs/                      User manual, test procedures, setup guide
└── README.md
```

---

## Quick start

1. **Install** the Arduino IDE + ESP32 board support.
2. **Install libraries:** NimBLE-Arduino, Adafruit TMP117, SparkFun MAX3010x, Adafruit BusIO.
3. **Flash** `firmware/xiao_sensor_node` to the XIAO (partition: *Huge APP*).
4. **Flash** `firmware/devkit_analyzer` to the DevKit (partition: *Huge APP*).
5. On your computer, **join the Wi-Fi network `HealthAC`** (password `health1234`).
6. **Open** `simulator/index.html`, enter IP `192.168.4.1`, click **Connect**.
7. Place a fingertip on the MAX30102 — heart rate and temperature appear, and the AC reacts.

Full instructions: see [`docs/USER_MANUAL.md`](docs/USER_MANUAL.md).

---

## Hardware

| Component | Role | I2C address |
|-----------|------|-------------|
| Seeed XIAO ESP32-C3 | Sensor node, BLE server | — |
| ESP32 DevKit V1 | Analyzer, Wi-Fi AP, web server | — |
| Adafruit TMP117 | Skin temperature | 0x48 |
| MAX30102 | Heart rate (BPM) | 0x57 |

Wiring: both sensors share one I2C bus — `SDA→D4`, `SCL→D5`, `VIN→3V3`, `GND→GND`.
See [`diagrams/wiring.png`](diagrams/wiring.png).

---

## Testing

Standalone sketches in `firmware/tests/` validate each layer before integration:

- `test_1_i2c_scanner` — confirms both sensors respond (0x48, 0x57).
- `test_2_sensor_check` — prints live temperature, IR, and BPM.
- `test_3_ble_scan` — confirms the DevKit can see the XIAO over BLE.

Verification procedures and results: [`docs/TEST_PROCEDURES.md`](docs/TEST_PROCEDURES.md).

---

## Team

| Member | Role |
|--------|------|
| [Member 1] | [role] |
| [Member 2] | [role] |
| [Member 3] | [role] |

## License

Released under the MIT License — see [`LICENSE`](LICENSE).
