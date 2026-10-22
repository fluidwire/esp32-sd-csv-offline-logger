/*
 * ESP32 Offline CSV Data Logger with RTC Timestamps
 * -------------------------------------------------
 * Logs sensor readings to a daily CSV file on a microSD card, stamped by
 * a DS3231 RTC. No WiFi needed - ideal for field deployments.
 * A new file (YYYYMMDD.csv) is started each day; data is flushed to the
 * card on a fixed interval to survive power loss.
 *
 * Wiring:
 *   microSD (SPI/VSPI): CS=GPIO5, SCK=18, MISO=19, MOSI=23, VCC=5V, GND
 *   DS3231 (I2C):       SDA=GPIO21, SCL=GPIO22, VCC=3V3, GND
 *   Analog sensor:      GPIO34 (ADC1) - e.g. a potentiometer or LDR divider
 *
 * Required library (install via Library Manager):
 *   - "RTClib" by Adafruit
 *   (SD, SPI, Wire are built into the ESP32 core)
 *
 * Setup:
 *   1. Flash. The RTC is set to the compile time if it lost power.
 *   2. Open Serial Monitor at 115200.
 *
 * (c) 2026 Fluidwire. Released under the MIT License.
 * https://github.com/fluidwire
 */

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>

const uint8_t  SD_CS_PIN       = 5;
const uint8_t  SENSOR_PIN      = 34;
const uint32_t LOG_INTERVAL_MS = 5 * 1000UL;    // one row every 5 s
const uint32_t FLUSH_INTERVAL_MS = 30 * 1000UL; // flush to card every 30 s

RTC_DS3231 rtc;
File logFile;
char currentName[16] = "";
uint32_t lastLog = 0;
uint32_t lastFlush = 0;
uint32_t rowCount = 0;

// Open (or rotate to) today's file; write header if the file is new.
bool openLogFor(const DateTime &now) {
  char name[16];
  snprintf(name, sizeof(name), "/%04d%02d%02d.csv", now.year(), now.month(), now.day());
  if (strcmp(name, currentName) == 0 && logFile) return true;

  if (logFile) { logFile.flush(); logFile.close(); }
  bool isNew = !SD.exists(name);
  logFile = SD.open(name, FILE_APPEND);
  if (!logFile) {
    Serial.printf("[sd] cannot open %s\n", name);
    currentName[0] = '\0';
    return false;
  }
  if (isNew) logFile.println("timestamp,raw_adc,millivolts");
  strlcpy(currentName, name, sizeof(currentName));
  Serial.printf("[sd] logging to %s\n", name);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n[boot] ESP32 offline CSV logger");

  Wire.begin(21, 22);
  if (!rtc.begin()) {
    Serial.println("[rtc] DS3231 not found - check wiring");
    while (true) delay(1000);
  }
  if (rtc.lostPower()) {
    Serial.println("[rtc] lost power - setting to compile time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("[sd] mount failed - check card and wiring");
    while (true) delay(1000);
  }
  Serial.println("[sd] card mounted");
  analogReadResolution(12);
}

void loop() {
  uint32_t ms = millis();
  if (ms - lastLog < LOG_INTERVAL_MS) return;
  lastLog = ms;

  DateTime now = rtc.now();
  if (!openLogFor(now)) return;

  int raw = analogRead(SENSOR_PIN);
  int mv  = analogReadMilliVolts(SENSOR_PIN);

  char row[64];
  snprintf(row, sizeof(row), "%04d-%02d-%02d %02d:%02d:%02d,%d,%d",
           now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second(), raw, mv);
  logFile.println(row);
  rowCount++;
  Serial.printf("[log] %s\n", row);

  if (ms - lastFlush >= FLUSH_INTERVAL_MS) {
    logFile.flush();
    lastFlush = ms;
    Serial.printf("[sd] flushed (%lu rows)\n", (unsigned long)rowCount);
  }
}
