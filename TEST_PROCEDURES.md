# Test Procedures and Verification

This document describes the verification tests that map each specification (Chapter 1 of the report) to a concrete procedure and result. Run each test and record the measured value in the results table.

---

## Test environment

- **Equipment:** USB power, computer running Arduino IDE Serial Monitor and the web simulator, a reference thermometer (for Test 1), a stopwatch/phone (for Tests 3–5).
- **Setup:** XIAO + sensors powered and advertising; DevKit powered in Access Point mode; computer joined to `HealthAC`.

---

## Test 1 — Skin temperature accuracy

**Goal:** verify TMP117 reads within ±0.5 °C.
**Procedure:**
1. Flash `tests/test_2_sensor_check.ino` to the XIAO.
2. Let the sensor settle at room temperature for 2 minutes.
3. Compare the reported temperature to a reference thermometer in the same spot.
4. Repeat while gently warming the sensor with a fingertip.

**Acceptance:** |TMP117 − reference| ≤ 0.5 °C.
**Measured:** `[____ °C vs ____ °C]` → **[PASS/FAIL]**

---

## Test 2 — Heart-rate detection

**Goal:** verify the MAX30102 + algorithm report a plausible BPM.
**Procedure:**
1. With `test_2_sensor_check.ino` running, place a fingertip over the sensor.
2. Observe the `IR` value rise above 50000 (finger present).
3. Wait ~10 s for BPM to stabilize; compare against a manual pulse count (count beats for 30 s × 2).

**Acceptance:** reported BPM within ±10 BPM of manual count, in 60–120 range.
**Measured:** `[sensor ____ BPM vs manual ____ BPM]` → **[PASS/FAIL]**

---

## Test 3 — BLE connection range

**Goal:** verify the BLE link holds at ≥ 3 m.
**Procedure:**
1. Run the full firmware on both boards; confirm `Connected to XIAO_Health_Node`.
2. Move the XIAO away from the DevKit in 1 m steps.
3. At each distance, confirm `RX` lines continue on the DevKit Serial Monitor.

**Acceptance:** stable data flow at ≥ 3 m line-of-sight.
**Measured:** `[stable up to ____ m]` → **[PASS/FAIL]**

---

## Test 4 — Data update rate

**Goal:** verify ≥ 2 packets per second reach the DevKit.
**Procedure:**
1. With the system connected, count `RX` lines printed over 10 seconds.
2. Divide by 10 to get packets/second.

**Acceptance:** ≥ 2 packets/second.
**Measured:** `[____ packets/s]` → **[PASS/FAIL]**

---

## Test 5 — Simulator response time

**Goal:** verify the simulator reflects a change within 2 s.
**Procedure:**
1. With the simulator connected, warm the temperature sensor sharply.
2. Start a stopwatch when the Serial Monitor shows the new value.
3. Stop when the simulator's displayed temperature/AC target updates.

**Acceptance:** ≤ 2 s.
**Measured:** `[____ s]` → **[PASS/FAIL]**

---

## Results summary (copy into report Chapter 5.2)

| # | Specification | Target | Measured | Pass/Fail |
|---|---------------|--------|----------|-----------|
| 1 | Temp accuracy | ± 0.5 °C | `[__]` | `[__]` |
| 2 | Heart-rate detection | 60–120 BPM, ±10 | `[__]` | `[__]` |
| 3 | BLE range | ≥ 3 m | `[__]` | `[__]` |
| 4 | Update rate | ≥ 2 /s | `[__]` | `[__]` |
| 5 | Simulator response | ≤ 2 s | `[__]` | `[__]` |
