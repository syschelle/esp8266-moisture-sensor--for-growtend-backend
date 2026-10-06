# ESP8266 Moisture Sensor v0.1.2

This release completes the GitHub firmware publishing workflow.

## GitHub Release assets

When a version tag is pushed, GitHub Actions now:

- builds the ESP8266 firmware with PlatformIO
- creates the OTA package
- keeps the normal GitHub Actions artifact
- creates the matching GitHub Release when necessary
- uploads the compiled firmware directly to the GitHub Release assets

The GitHub Release now contains:

- `firmware.bin`
- `firmware.bin.sha256`
- `manifest.json`
- `README.md`

The standard GitHub-generated source archives remain available as well.

## Build pipeline

The workflow continues to:

- use `ubuntu-24.04`
- run static project validation
- build with `pio run -e d1_mini`
- create the OTA package with `scripts/make_ota.py`
- verify all generated files
- upload `esp8266-moisture-sensor-v0.1.2` as a GitHub Actions artifact

## Firmware functionality

Sensor functionality is unchanged from v0.1.1:

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
- factory reset

## Version

v0.1.2
