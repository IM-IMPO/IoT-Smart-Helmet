# Power Requirements and Safety

## Power Supply
The ESP32, GPS, and GSM modules have different power requirements. Select the supply and regulator according to the specifications of the exact modules used.

## SIM800L Power
The SIM800L may require a supply around 3.4–4.4 V and can draw current bursts approaching 2 A. Do not power it directly from the ESP32 3.3 V output. Verify the requirements of your specific breakout board and provide a suitable supply.

## Battery and Charging
Use a battery with appropriate protection and a charging circuit designed for that battery chemistry. A TP4056 charging module alone does not provide every required regulated output for the complete system.

## Physical Safety
- Do not drill, cut, or weaken a certified helmet shell.
- Secure wiring and electronics so they cannot become loose.
- Keep batteries protected from impact, heat, short circuits, and moisture.
- Bench-test the system before mounting it.
- Do not rely on this prototype as the sole means of emergency notification.

## Network Limitations
The SIM800L uses 2G GSM. Confirm that a compatible network is available from the intended carrier before relying on SMS functionality.

## Prototype Disclaimer
This project is an academic prototype and is not a certified crash-detection or emergency-response system.
