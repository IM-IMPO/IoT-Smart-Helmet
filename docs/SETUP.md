# Installation and Setup

## Requirements
- Arduino IDE
- ESP32 board support package by Espressif Systems
- TinyGPSPlus library
- ESP32 Dev Board and USB data cable
- Project hardware or a compatible Wokwi simulation

## Installation
1. Install Arduino IDE.
2. Open Boards Manager and install the ESP32 platform.
3. Open Library Manager and install TinyGPSPlus.
4. Open `SmartHelmet.ino` from the project root.
5. Select the board that matches the physical ESP32.
6. Select the correct COM port.
7. Configure the emergency contact number in the sketch.
8. Click Verify to compile the sketch.
9. Click Upload to flash the firmware.
10. Open Serial Monitor at 115200 baud.

## Emergency Contact Configuration
Replace the placeholder emergency number in the sketch with the intended recipient's number in international format. Keep private contact details out of public screenshots and documentation.

## Initial Verification
Check the startup messages in Serial Monitor. Verify that the MPU6050 is detected, the GPS provides a valid fix when possible, and the GSM module responds. Test the SOS and cancel buttons before testing emergency SMS behavior.
