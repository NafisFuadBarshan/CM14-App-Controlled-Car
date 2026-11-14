# CM14 App-Controlled Car — WiFi Robot (ESP8266)

**Course:** CSE 316 – Peripheral & Interfacing Lab, University of Asia Pacific  
**Project:** CM14 App-Controlled Car – Wireless Remote Control and IoT Integration  
**Authors:** Nafis Fuad Barshan (21201166), Tanvir Rahan Rifat (22101065) – Section B1  
**Supervisor:** A S Zaforullah Momtaz, Assistant Professor, Dept. of CSE

A 4-wheel-drive smart car built on a **NodeMCU ESP8266** and a **NodeMCU Motor Shield**.
The car runs a small web server; a smartphone app (or any browser) sends single-letter
commands over WiFi to drive, steer, change speed, switch the headlights, and sound the horn.

---

## Features

- WiFi control over HTTP (`/?State=<command>`)
- Auto mode fallback: connects to your WiFi (STA); if that fails, creates its own hotspot (AP)
- 8-direction driving (forward, backward, left, right + 4 diagonals) and stop
- 10 speed levels via PWM (330–1023)
- Headlights (LED) and horn (active buzzer)
- WiFi status LED
- Over-the-air (OTA) firmware updates
- Optional failsafe: stop the motors if no command is received

## Hardware

| Qty | Component | Notes |
|----|-----------|-------|
| 1 | NodeMCU ESP8266 (ESP-12E) | Main controller |
| 1 | NodeMCU Motor Shield (L293D) | Drives the motors |
| 2 or 4 | Geared DC motors + wheels | FL, FR, BL, BR (left pair on Motor A, right pair on Motor B) |
| 1 | Active buzzer | Horn |
| 2 | Bright white LEDs (+ 220 Ω resistors if not on a module) | Headlights |
| 2 | 18650 Li-ion cells (3.7 V) in a holder | 7.4 V in series, to the shield power terminal |
| – | Chassis, jumper wires, switch | |

## Pin Map

| Function | NodeMCU pin | GPIO |
|----------|-------------|------|
| Motor A speed (PWM) | D1 | 5 |
| Motor B speed (PWM) | D2 | 4 |
| Motor A direction | D3 | 0 |
| Motor B direction | D4 | 2 |
| Buzzer | D5 | 14 |
| Headlight LEDs | D8 | 15 |
| WiFi status LED (active-low) | D0 | 16 |

> **Note:** the lab report text says LEDs on D5 and buzzer on D6. The firmware and the circuit
> diagram use **buzzer = D5** and **LEDs = D8**. Follow the table above (or update the report).

## Wiring (see circuit diagram)

1. Plug the NodeMCU into the motor shield.
2. Connect the **left-side motors** (FL + BL) in parallel to the **Motor A** terminals and the
   **right-side motors** (FR + BR) to **Motor B**. If a side spins the wrong way, swap its two wires.
3. Buzzer: `+` to VCC, signal to D5.
4. LEDs: signal to D8, other side to VCC/GND as shown in the diagram.
5. Battery pack (2 × 18650 = 7.4 V) to the shield's power input. Check polarity before switching on.

## Software Setup

1. Install the **Arduino IDE**.
2. Add the ESP8266 board package: *File → Preferences → Additional Boards Manager URLs*  
   `http://arduino.esp8266.com/stable/package_esp8266com_index.json`  
   then *Tools → Board → Boards Manager → install "esp8266"*.
3. Select **Tools → Board → NodeMCU 1.0 (ESP-12E Module)**, upload speed 115200 (or 921600).
4. Open `WiFi_Robot_Car.ino` and edit the settings at the top:

   ```cpp
   const char* STA_SSID     = "YourWiFi";
   const char* STA_PASSWORD = "YourPassword";
   ```
   Leave the SSID empty to always use the car's own hotspot.
5. Upload, open the Serial Monitor at **115200 baud**, and note the IP address.

## Connecting

| Mode | How |
|------|-----|
| **STA** (joined your WiFi) | Phone on the same WiFi. Use the IP printed in the Serial Monitor. Status LED is ON. |
| **AP** (fallback) | Connect the phone to the WiFi network `wificar-XXXX`. IP is `192.168.4.1`. Status LED is OFF. |

Use an RC-car WiFi app that sends `?State=` commands (e.g. "WiFi RC Car" / "Wi-Fi Robot Car" apps)
and enter the IP address, or test from a browser:

```
http://192.168.4.1/?State=F     # forward
http://192.168.4.1/?State=S     # stop
```

## Command Reference

| Command | Action | Command | Action |
|---------|--------|---------|--------|
| `F` | Forward | `G` | Forward-left |
| `B` | Backward | `I` | Forward-right |
| `L` | Turn left (spin) | `H` | Backward-left |
| `R` | Turn right (spin) | `J` | Backward-right |
| `S` | Stop | `V` | Horn |
| `W` | Lights ON | `w` | Lights OFF |
| `0`–`9` | Speed 330 → 960 | `q` | Full speed (1023) |

Speed steps: `0`=330, `1`=400, `2`=470, `3`=540, `4`=610, `5`=680, `6`=750, `7`=820, `8`=890, `9`=960.

## OTA Updates

After the first USB upload, the car appears under *Tools → Port* as a network port named
`wificar-XXXX` (computer and car must be on the same network). Optionally set `OTA_PASSWORD`.

## Failsafe

Set `FAILSAFE_MS` (e.g. `1500`) to stop the motors automatically when no command has been
received for that long. Note that some apps only send a command on button press and release; with
those apps keep it at `0` (disabled).

## Testing Checklist

- [ ] Serial Monitor shows the hostname and IP / AP address
- [ ] All wheels spin forward on `F`, backward on `B`
- [ ] `L` / `R` rotate the car in place in the correct direction
- [ ] Speed digits visibly change motor speed
- [ ] `V` sounds the buzzer, `W` / `w` toggle the LEDs
- [ ] Command-to-motor response feels instant (< 100 ms)
- [ ] Battery lasts > 1 hour; no overheating of the motor shield

## Troubleshooting

| Problem | Fix |
|---------|-----|
| Car won't connect to WiFi | Check SSID/password, use a 2.4 GHz network; it falls back to AP mode anyway |
| Board won't boot with motor shield | D3/D4 (GPIO0/2) are boot pins – disconnect the shield or motors during upload |
| Motors twitch / ESP resets | Low battery or motor noise; charge cells, add a 100 µF capacitor near the motors |
| One side runs backwards | Swap that side's motor wires |
| Wheels don't move at low speed | Use speeds ≥ 330; raise the lowest step if the car is heavy |

## Future Work

Ultrasonic obstacle avoidance (HC-SR04), IR line following, MPU6050 drift correction,
light-sensor automatic headlights, and ESP32 BLE control (as outlined in the project report).

## Project Timeline

Classify modules (6–14 Feb) → test sensors (15–22 Feb) → test actuators (23 Feb–2 Mar) →
prepare platform (3–9 Mar) → install components (10–30 Mar) → upload code (31 Mar–13 Apr) →
calibrate & debug (14–21 Apr).

## License

Educational project – free to use and modify with credit.
