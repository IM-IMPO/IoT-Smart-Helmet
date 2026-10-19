
#include <Wire.h>
#include <TinyGPSPlus.h>

// ---------------- PIN CONFIGURATION ----------------
const int SOS_BUTTON_PIN    = 32;
const int CANCEL_BUTTON_PIN = 33;
const int BUZZER_PIN        = 25;
const int WARNING_LED_PIN   = 13;

const int GPS_RX_PIN = 16;  // ESP32 RX <- GPS TX
const int GPS_TX_PIN = 17;  // ESP32 TX -> GPS RX

const int GSM_RX_PIN = 26;  // ESP32 RX <- GSM TXD
const int GSM_TX_PIN = 27;  // ESP32 TX -> GSM RXD

// Replace with the recipient's real phone number.
// Philippines example format: +639XXXXXXXXX
const char EMERGENCY_CONTACT[] = "+639XXXXXXXXX";

// ---------------- GPS / GSM ----------------
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
HardwareSerial gsmSerial(1);

// ---------------- MPU6050 ----------------
const uint8_t MPU_ADDR = 0x68;

// These are starting thresholds, NOT validated crash thresholds.
const float IMPACT_THRESHOLD_G = 3.0;
const float ROTATION_THRESHOLD_DPS = 220.0;
const float ROTATION_ACCEL_THRESHOLD_G = 1.8;

float ax, ay, az;
float gx, gy, gz;
float accelerationG, rotationDps;

// ---------------- SOS SETTINGS ----------------
const unsigned long COUNTDOWN_SECONDS = 10;
const unsigned long SENSOR_INTERVAL_MS = 20;
const unsigned long ACCIDENT_COOLDOWN_MS = 15000;

bool countdownActive = false;
bool alertSent = false;
unsigned long countdownStart = 0;
unsigned long lastSensorRead = 0;
unsigned long lastAccidentTime = 0;

// Button debounce state
bool lastSOSReading = HIGH;
bool stableSOSState = HIGH;
unsigned long sosDebounceTime = 0;

bool lastCancelReading = HIGH;
bool stableCancelState = HIGH;
unsigned long cancelDebounceTime = 0;

// --------------------------------------------------
// Read one MPU6050 register
// --------------------------------------------------
bool readMPURegisters(uint8_t startRegister, uint8_t count,
                      uint8_t *buffer) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(startRegister);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  uint8_t received = Wire.requestFrom(MPU_ADDR, count, true);

  if (received != count) {
    while (Wire.available()) Wire.read();
    return false;
  }

  for (uint8_t i = 0; i < count; i++) {
    buffer[i] = Wire.read();
  }

  return true;
}

// --------------------------------------------------
// Initialize MPU6050
// --------------------------------------------------
bool initializeMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1
  Wire.write(0x00);  // Wake up sensor

  if (Wire.endTransmission() != 0) {
    return false;
  }

  delay(100);

  // Accelerometer: +/- 8g (register 0x1C = 0x10)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C);
  Wire.write(0x10);

  if (Wire.endTransmission() != 0) {
    return false;
  }

  // Gyroscope: +/- 1000 degrees/second (register 0x1B = 0x10)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x10);

  if (Wire.endTransmission() != 0) {
    return false;
  }

  return true;
}

// --------------------------------------------------
// Read acceleration and rotation
// --------------------------------------------------
bool readMPU() {
  uint8_t data[14];

  // Starting at ACCEL_XOUT_H (0x3B):
  // acceleration, temperature, gyroscope
  if (!readMPURegisters(0x3B, 14, data)) {
    return false;
  }

  int16_t rawAX = (int16_t)((data[0] << 8) | data[1]);
  int16_t rawAY = (int16_t)((data[2] << 8) | data[3]);
  int16_t rawAZ = (int16_t)((data[4] << 8) | data[5]);

  int16_t rawGX = (int16_t)((data[8] << 8) | data[9]);
  int16_t rawGY = (int16_t)((data[10] << 8) | data[11]);
  int16_t rawGZ = (int16_t)((data[12] << 8) | data[13]);

  // +/- 8g: 4096 LSB/g
  ax = rawAX / 4096.0;
  ay = rawAY / 4096.0;
  az = rawAZ / 4096.0;

  // +/- 1000 degrees/s: 32.8 LSB/(degree/s)
  gx = rawGX / 32.8;
  gy = rawGY / 32.8;
  gz = rawGZ / 32.8;

  accelerationG = sqrt(ax * ax + ay * ay + az * az);
  rotationDps = sqrt(gx * gx + gy * gy + gz * gz);

  return true;
}

// --------------------------------------------------
// Keep processing GPS bytes continuously
// --------------------------------------------------
void updateGPS() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
}

// --------------------------------------------------
// Read a debounced button press
// Buttons use INPUT_PULLUP and connect to GND.
// --------------------------------------------------
bool buttonPressed(int pin, bool &lastReading,
                   bool &stableState,
                   unsigned long &debounceTime) {
  bool reading = digitalRead(pin);

  if (reading != lastReading) {
    lastReading = reading;
    debounceTime = millis();
  }

  if (millis() - debounceTime >= 35) {
    if (reading != stableState) {
      stableState = reading;

      if (stableState == LOW) {
        return true;
      }
    }
  }

  return false;
}

bool sosPressed() {
  return buttonPressed(SOS_BUTTON_PIN, lastSOSReading,
                       stableSOSState, sosDebounceTime);
}

bool cancelPressed() {
  return buttonPressed(CANCEL_BUTTON_PIN, lastCancelReading,
                       stableCancelState, cancelDebounceTime);
}

// --------------------------------------------------
// Read modem response for a specified period
// --------------------------------------------------
String readGSMResponse(unsigned long timeoutMs) {
  String response;
  unsigned long start = millis();

  while (millis() - start < timeoutMs) {
    updateGPS();

    while (gsmSerial.available()) {
      char c = (char)gsmSerial.read();
      response += c;
      Serial.write(c);
    }

    delay(2);
  }

  return response;
}

bool sendAT(const char *command, const char *expected,
            unsigned long timeoutMs = 3000) {
  while (gsmSerial.available()) gsmSerial.read();

  Serial.print("\nGSM > ");
  Serial.println(command);

  gsmSerial.println(command);

  String response = readGSMResponse(timeoutMs);
  return response.indexOf(expected) >= 0;
}

// --------------------------------------------------
// Initialize SIM800L
// --------------------------------------------------
void initializeGSM() {
  Serial.println("\nInitializing SIM800L...");

  bool modemResponded = false;

  for (int attempt = 0; attempt < 5; attempt++) {
    if (sendAT("AT", "OK", 2000)) {
      modemResponded = true;
      break;
    }
    delay(1000);
  }

  if (!modemResponded) {
    Serial.println("WARNING: GSM modem did not respond.");
    Serial.println("Check power, UART wiring, and modem support.");
    return;
  }

  sendAT("ATE0", "OK");
  sendAT("AT+CMGF=1", "OK");
  sendAT("AT+CSCS=\"GSM\"", "OK");
  sendAT("AT+CSQ", "OK");
  sendAT("AT+CREG?", "OK");

  Serial.println("GSM initialization commands completed.");
  Serial.println("Check signal and network registration before relying on SMS.");
}

// --------------------------------------------------
// Obtain current GPS location for the alert
// --------------------------------------------------
String buildEmergencyMessage() {
  String message = "SMART HELMET EMERGENCY ALERT!\n";
  message += "Possible rider accident detected.\n";

  if (gps.location.isValid() &&
      gps.location.age() < 15000) {
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();

    message += "Latitude: " + String(latitude, 6) + "\n";
    message += "Longitude: " + String(longitude, 6) + "\n";
    message += "Map: https://maps.google.com/?q=";
    message += String(latitude, 6) + ",";
    message += String(longitude, 6);
  } else {
    message += "GPS location currently unavailable.";
    message += "\nPlease check on the rider.";
  }

  return message;
}

// --------------------------------------------------
// Send emergency SMS
// --------------------------------------------------
bool sendSMS(const String &message) {
  Serial.println("\nPreparing emergency SMS...");

  if (String(EMERGENCY_CONTACT) == "+639XXXXXXXXX") {
    Serial.println("ERROR: Set EMERGENCY_CONTACT first.");
    return false;
  }

  if (!sendAT("AT+CMGF=1", "OK")) {
    Serial.println("ERROR: Could not enable SMS text mode.");
    return false;
  }

  while (gsmSerial.available()) gsmSerial.read();

  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(EMERGENCY_CONTACT);
  gsmSerial.println("\"");

  String promptResponse = readGSMResponse(5000);

  if (promptResponse.indexOf('>') < 0) {
    Serial.println("\nERROR: SMS prompt not received.");
    return false;
  }

  gsmSerial.print(message);
  gsmSerial.write(26);  // Ctrl+Z submits SMS

  String result = readGSMResponse(20000);

  if (result.indexOf("+CMGS:") >= 0 &&
      result.indexOf("OK") >= 0) {
    Serial.println("\nSMS accepted by modem.");
    Serial.println("Carrier delivery is not guaranteed.");
    return true;
  }

  Serial.println("\nSMS sending failed or was not confirmed.");
  return false;
}

// --------------------------------------------------
// Start the emergency countdown
// --------------------------------------------------
void startCountdown(const char *reason) {
  if (countdownActive) return;

  countdownActive = true;
  countdownStart = millis();

  Serial.println("\n!!! POSSIBLE EMERGENCY !!!");
  Serial.print("Trigger: ");
  Serial.println(reason);
  Serial.println("Press CANCEL within 10 seconds to stop the alert.");
}

// --------------------------------------------------
// Cancel the countdown
// --------------------------------------------------
void cancelCountdown() {
  countdownActive = false;

  noTone(BUZZER_PIN);
  digitalWrite(WARNING_LED_PIN, LOW);

  Serial.println("\nEmergency alert CANCELLED.");
}

// --------------------------------------------------
// Countdown with buzzer and LED
// --------------------------------------------------
void processCountdown() {
  if (!countdownActive) return;

  if (cancelPressed()) {
    cancelCountdown();
    return;
  }

  unsigned long elapsed = (millis() - countdownStart) / 1000;
  long remaining = (long)COUNTDOWN_SECONDS - (long)elapsed;

  if (remaining <= 0) {
    countdownActive = false;

    noTone(BUZZER_PIN);
    digitalWrite(WARNING_LED_PIN, LOW);

    Serial.println("\nCountdown expired. Sending emergency alert.");

    // Capture the freshest available GPS data before sending.
    updateGPS();

    String message = buildEmergencyMessage();
    Serial.println("\nEmergency message:");
    Serial.println(message);

    sendSMS(message);

    alertSent = true;
    lastAccidentTime = millis();

    Serial.println("\nSystem monitoring resumes.");
    return;
  }

  // Print only once per second
  static long previousRemaining = -1;

  if (remaining != previousRemaining) {
    previousRemaining = remaining;

    Serial.print("Emergency SMS in ");
    Serial.print(remaining);
    Serial.println(" seconds...");

    if (remaining <= 5) {
      tone(BUZZER_PIN, 2200, 180);
      digitalWrite(WARNING_LED_PIN, HIGH);
    } else {
      tone(BUZZER_PIN, 1300, 100);
      digitalWrite(WARNING_LED_PIN, elapsed % 2 == 0);
    }
  }
}

// --------------------------------------------------
// Setup
// --------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(SOS_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CANCEL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(WARNING_LED_PIN, OUTPUT);

  digitalWrite(WARNING_LED_PIN, LOW);
  noTone(BUZZER_PIN);

  Wire.begin(21, 22);

  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  gsmSerial.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);

  Serial.println("\nSMART HELMET - LIVE GPS AND ACCIDENT MONITOR");

  if (initializeMPU()) {
    Serial.println("MPU6050 initialized.");
  } else {
    Serial.println("ERROR: MPU6050 not responding at address 0x68.");
    Serial.println("Automatic detection will be disabled.");
  }

  initializeGSM();

  Serial.println("\nWaiting for GPS satellite fix...");
  Serial.println("Monitoring sensor data.");
  Serial.println("SOS button: manual emergency.");
  Serial.println("CANCEL button: cancel active countdown.");
}

// --------------------------------------------------
// Main loop
// --------------------------------------------------
void loop() {
  updateGPS();

  if (sosPressed() && !countdownActive) {
    startCountdown("Manual SOS button");
  }

  if (countdownActive) {
    processCountdown();
    delay(5);
    return;
  }

  if (millis() - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = millis();

    if (readMPU()) {
      bool impactDetected = accelerationG >= IMPACT_THRESHOLD_G;
      bool rotationDetected = rotationDps >= ROTATION_THRESHOLD_DPS;
      bool combinedMovement =
          rotationDetected &&
          accelerationG >= ROTATION_ACCEL_THRESHOLD_G;

      // Conservative starting rule:
      // a strong impact OR substantial rotation combined
      // with elevated acceleration.
      bool possibleAccident = impactDetected || combinedMovement;

      if (possibleAccident &&
          millis() - lastAccidentTime >= ACCIDENT_COOLDOWN_MS) {
        Serial.println("\nAccident-like sensor movement detected.");
        Serial.print("Acceleration magnitude: ");
        Serial.print(accelerationG, 2);
        Serial.println(" g");

        Serial.print("Rotation magnitude: ");
        Serial.print(rotationDps, 1);
        Serial.println(" degrees/second");

        startCountdown("Automatic MPU6050 detection");
      }
    }
  }

  // Periodically report GPS status.
  static unsigned long lastGPSStatus = 0;

  if (millis() - lastGPSStatus >= 5000) {
    lastGPSStatus = millis();

    if (gps.location.isValid()) {
      Serial.print("GPS: ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(", ");
      Serial.print(gps.location.lng(), 6);
      Serial.print(" | Satellites: ");
      Serial.println(gps.satellites.value());
    } else {
      Serial.println("GPS: Waiting for a valid location fix.");
    }
  }

  delay(2);
}
