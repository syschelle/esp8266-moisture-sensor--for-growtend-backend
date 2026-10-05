# ESP8266 Moisture Sensor v0.1.0

Initial release of the ESP8266 soil-moisture sensor for integration with the GrowTent backend.

## Highlights

- ESP8266 / Wemos D1 mini PlatformIO project
- one capacitive analog soil-moisture sensor
- no local TM1637 or other display
- configurable signal-pin setting in the web interface
- ESP8266 analog input currently restricted to `A0`
- adjustable measurement interval
- configurable ADC sample count
- averaged and smoothed ADC measurements
- dry/wet two-point calibration
- calculated soil moisture from 0 to 100 %
- REST endpoint `GET /api/current-values`
- health endpoint `GET /api/health`
- responsive German/English web interface
- light/dark theme
- Wi-Fi fallback AP provisioning
- NTP synchronization at boot
- EEPROM-backed persistent settings with CRC32
- system log
- browser-assisted OTA update flow
- manual firmware upload
- factory reset

## GrowTent backend API

The intended integration endpoint is:

`http://<sensor-ip>/api/current-values`

The response includes:

- device name
- sensor name
- signal pin
- firmware version
- raw ADC value
- moisture percentage
- calibration state
- last measurement timestamp
- Wi-Fi RSSI
- uptime

## Default access point

- SSID: `MoistureSensor-<CHIPID>`
- password: `MS-Setup-8266`

## OTA channel

Repository:

`syschelle/esp8266-moisture-sensor--for-growtend-backend`

Branch:

`ota`

Expected OTA branch files:

- `manifest.json`
- `firmware.bin`
- `firmware.bin.sha256`
- `README.md`

## Platform

- ESP8266
- Arduino framework
- PlatformIO
- Wemos/Lolin D1 mini compatible target

## License

Apache License 2.0
