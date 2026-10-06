# ESP8266 Moisture Sensor v0.1.3

This release fixes the remaining device naming inherited from the original project and makes the setup/AP identity consistent.

## Naming cleanup

- removes remaining legacy project-name references
- keeps the fallback AP SSID as `MoistureSensor-<CHIPID>`
- keeps the default device name as `SoilSensor-1`
- keeps the fallback AP password as `MS-Setup-8266`
- ensures the web UI, logs, documentation and firmware metadata consistently use the ESP8266 Moisture Sensor naming

## README

The project description now explicitly states that the sensor is intended for direct integration with:

`syschelle/growtent-backend`

## Validation

Project checks now fail if the legacy display-project name is reintroduced.

## Existing functionality retained

- ESP8266 / Wemos D1 mini target
- capacitive analog soil-moisture sensor on A0
- configurable signal-pin setting
- averaged and smoothed ADC measurements
- dry/wet calibration
- 0 to 100 % soil-moisture calculation
- `/api/current-values`
- `/api/health`
- Wi-Fi fallback AP
- NTP
- German/English web interface
- light/dark theme
- browser-assisted OTA
- GitHub Actions firmware artifact
- GitHub Release firmware assets
- factory reset

## Version

v0.1.3
