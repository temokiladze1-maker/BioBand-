# Hardware

This folder documents the hardware design.

## Working prototype
The functional prototype is built on **development boards and a breadboard**:
- Seeed XIAO ESP32-C3 (sensor node)
- ESP32 DevKit V1 (analyzer)
- Adafruit TMP117 + MAX30102 breakouts on a shared I2C bus

See `../diagrams/wiring.png` for the connection diagram.

## PCB concept
`../diagrams/pcb_concept.png` shows a **conceptual** single-board layout that would
integrate the XIAO and both sensors onto one compact 50 × 38 mm board for future
production. It is a design concept; the delivered prototype uses dev boards.

## Bill of materials
| Component | Qty | Notes |
|-----------|-----|-------|
| XIAO ESP32-C3 | 1 | sensor node MCU |
| ESP32 DevKit V1 | 1 | analyzer MCU |
| Adafruit TMP117 | 1 | I2C 0x48 |
| MAX30102 | 1 | I2C 0x57 |
| Breadboard + jumper wires | 1 set | prototyping |
