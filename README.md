# Test Sketches

Standalone sketches to validate each layer before running the full firmware.

| Sketch | Board | Checks |
|--------|-------|--------|
| `test_1_i2c_scanner.ino` | XIAO | both sensors respond on I2C (0x48, 0x57) |
| `test_2_sensor_check.ino` | XIAO | live temperature, IR, and BPM values |
| `test_3_ble_scan.ino` | DevKit | DevKit can see the XIAO's BLE advertisement |

Run them in order. If all three pass, the full firmware will work.
See `../../docs/TEST_PROCEDURES.md` for formal verification procedures.
