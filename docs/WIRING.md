# Wiring Guide

## ESP32 Pin Connections

| Component | Component Pin | ESP32 Pin |
|---|---|---|
| MPU6050 | SDA | GPIO 21 |
| MPU6050 | SCL | GPIO 22 |
| MPU6050 | VCC | 3.3V, if supported by the breakout |
| MPU6050 | GND | GND |
| NEO-6M | TX | GPIO 16 |
| NEO-6M | RX | GPIO 17 |
| SIM800L | TXD | GPIO 26 |
| SIM800L | RXD | GPIO 27 |
| SOS button | Signal | GPIO 32 |
| Cancel button | Signal | GPIO 33 |
| Buzzer | Positive | GPIO 25, if electrically compatible |
| LED | Anode | GPIO 13 through a resistor |

The push buttons use the ESP32 internal pull-up resistors. Each button connects its GPIO pin to GND when pressed.

## Wiring Notes
- Connect module grounds to a common ground where required.
- Confirm the voltage requirements of each module before connecting power.
- Check UART voltage compatibility between the ESP32 and GSM module.
- Consult the documentation for the exact breakout boards being used.

See the repository's `diagram.json` for the Wokwi circuit configuration.
