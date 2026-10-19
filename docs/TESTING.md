# Testing and Results

## Test Cases

| Test | Procedure | Expected Result | Actual Result |
|---|---|---|---|
| ESP32 startup | Restart the ESP32 | Startup messages appear | Not tested |
| MPU6050 detection | Start the system with the sensor connected | Sensor initialization succeeds | Not tested |
| SOS button | Press SOS | Emergency countdown starts | Not tested |
| Cancel button | Press Cancel during countdown | Countdown stops | Not tested |
| Motion detection | Safely move the sensor through controlled motions | Detection responds according to configured thresholds | Not tested |
| GPS location | Wait for a valid GPS fix | Coordinates are printed | Not tested |
| GSM initialization | Start with a compatible SIM and power supply | Modem responds to AT commands | Not tested |
| SMS test | Send a test alert to an authorized test recipient | SMS delivery can be checked on the receiving phone | Not tested |

## Test Procedure
1. Inspect all connections and power supplies.
2. Power the ESP32 and open Serial Monitor.
3. Test the SOS and cancel buttons.
4. Check sensor initialization and GPS status.
5. Verify cellular registration and SMS behavior.
6. Record the actual result and any error messages.

## Limitations
Testing must be performed in a controlled environment. Do not strike a helmet or conduct dangerous riding tests to trigger the sensor. Record simulation results separately from physical hardware results.
