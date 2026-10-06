# ESP8266 Moisture Sensor v0.1.1

This release improves the GitHub Actions build pipeline so every successful CI build provides a downloadable firmware package.

## GitHub Actions

- pins the hosted runner to `ubuntu-24.04`
- runs the existing static validation
- builds the ESP8266 firmware with PlatformIO
- creates the OTA package with `scripts/make_ota.py`
- verifies that all expected OTA files were generated
- uploads the generated files as a GitHub Actions artifact
- names the artifact `esp8266-moisture-sensor-v0.1.1`
- keeps the artifact for 30 days

## Firmware artifact contents

The downloadable GitHub Actions artifact contains:

- `firmware.bin`
- `firmware.bin.sha256`
- `manifest.json`
- `README.md`

The artifact is available from the workflow run summary after a successful build.

## Firmware functionality

The sensor firmware itself is unchanged from v0.1.0:

- ESP8266 / Wemos D1 mini target
- capacitive analog soil-moisture sensor on A0
- configurable signal-pin setting in the web interface
- averaged and smoothed ADC measurements
- dry/wet calibration
- 0 to 100 % moisture calculation
- `/api/current-values`
- `/api/health`
- Wi-Fi fallback AP
- NTP
- German/English web interface
- light/dark theme
- browser-assisted OTA
- factory reset

## Version

v0.1.1
