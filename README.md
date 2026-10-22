# esp32-sd-csv-offline-logger — log sensor readings with RTC timestamps to a CSV file on an SD card with no network needed

IoT example by [Fluidwire](https://fluidwire.com/).

Sample sketch for an [ESP32 dev board](https://fluidwire.com/) that logs readings to a microSD card as CSV, timestamped by a DS3231 RTC. A new file is started every day and data is flushed on a fixed interval — built for field deployments where WiFi is unavailable.

## What you need

- 1x ESP32 dev board
- 1x microSD card module (SPI) + FAT32-formatted card
- 1x DS3231 RTC module (with coin cell)
- 1x analog sensor (potentiometer or LDR divider) on GPIO34

## Required library

- **RTClib** by Adafruit (Library Manager)

SD, SPI and Wire are built into the ESP32 core.

## Wiring

```
microSD:  CS -> GPIO5   SCK -> GPIO18   MISO -> GPIO19   MOSI -> GPIO23   VCC -> 5V   GND -> GND
DS3231:   SDA -> GPIO21  SCL -> GPIO22  VCC -> 3V3  GND -> GND
Sensor:   signal -> GPIO34 (ADC1)
```

## Setup

1. Format the card as FAT32.
2. Flash the sketch. If the RTC lost power it is set to the compile time.
3. Open Serial Monitor at **115200 baud**. Pull the card later and open the `.csv` files in Excel or Google Sheets.

## Expected serial output

```
[rtc] lost power - setting to compile time
[sd] card mounted
[sd] logging to /20261022.csv
[log] 2026-10-22 09:15:05,2048,1650
[sd] flushed (6 rows)
```

## Troubleshooting

- **"mount failed"**: check CS pin and that the module gets 5V; try another card (<=32 GB, FAT32).
- **"DS3231 not found"**: check SDA/SCL and run an I2C scanner.
- **Wrong time**: replace the RTC coin cell and re-flash to reset from compile time.
- **Lost last rows after power cut**: lower `FLUSH_INTERVAL_MS` (more writes, less loss).

## Notes

- Frequent flushes wear SD cards slightly; 30 s is a good balance for most builds.
- Replace the analog read with any sensor (DHT, DS18B20, etc.) and extend the CSV header.

## License

MIT — see [LICENSE](LICENSE).

## Got stuck?

Drop by [fluidwire.com](https://fluidwire.com/) — happy to help.
