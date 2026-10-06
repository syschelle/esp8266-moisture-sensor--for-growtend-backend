# ESP8266 Moisture Sensor for GrowTent Backend

This ESP8266-based sensor reads one capacitive analog soil-moisture sensor and exposes the current moisture value via HTTP for direct integration with the `syschelle/growtent-backend` project.

**Current version: v0.1.3**

## Hardware

- ESP8266, tested target: Wemos/Lolin D1 mini compatible
- Capacitive soil moisture sensor with analog output
- Sensor wiring:
  - VCC -> 3.3 V
  - GND -> GND
  - AOUT -> A0

> Important: ESP8266 boards differ in the voltage accepted on A0. A Wemos/Lolin D1 mini typically includes an onboard divider, while a bare ESP8266 ADC input is not equivalent. Verify the electrical limits of the exact board before connecting the sensor output.

## Signal pin

The signal pin is stored as a normal runtime setting and can be selected in the web interface.

For ESP8266 v0.1.0 the only valid analog selection is:

`A0`

The setting is intentionally modeled as a selectable pin so a later ESP32 variant can expose multiple ADC pins without changing the configuration concept.

## Measurement

Default behavior:

- measurement interval: 5 s
- 10 ADC samples per measurement
- samples are averaged
- an exponential filter smooths the result
- calibrated result is limited to 0..100 %

Calibration uses two stored ADC reference points:

- `dry_adc` -> 0 %
- `wet_adc` -> 100 %

The values may be in either numerical order. A minimum span is required to avoid an invalid calibration.

## HTTP API

### Current value

`GET /api/current-values`

Example:

```json
{
  "device": "SoilSensor-1",
  "sensor": "Topf 1",
  "signal_pin": "A0",
  "firmware_version": "0.1.3",
  "raw_adc": 487,
  "moisture_percent": 63.4,
  "calibrated": true,
  "last_measurement_at": "2026-10-05 21:10:00",
  "wifi_rssi": -57,
  "uptime_seconds": 86423
}
```

This is the endpoint intended for the GrowTent backend.

### Health

`GET /api/health`

Returns sensor, calibration, Wi-Fi and time status.

### UI state

`GET /api/state`

Used by the local web interface.

## Web interface

Pages/tabs:

- Status
- Sensor
- System
- System log
- OTA
- Factory reset

Features:

- German / English
- light / dark theme
- sensor name
- configurable signal pin
- measurement interval
- sample count
- dry/wet calibration buttons
- Wi-Fi settings
- NTP settings
- OTA
- factory reset

## Wi-Fi fallback AP

If no Wi-Fi is configured or station connection fails, the ESP opens a fallback AP.

```text
SSID:     MoistureSensor-<CHIPID>
Password: MS-Setup-8266
```

Open the ESP address shown by the client network information, normally `192.168.4.1`.

The fallback AP is disabled after the station connection has been stable for 10 seconds and is re-enabled after a longer disconnect.

## NTP

Defaults:

```text
Server:   de.pool.ntp.org
Timezone: CET-1CEST,M3.5.0,M10.5.0/3
```

NTP setup is initiated at each boot. The firmware does not block indefinitely waiting for time synchronization.

## Persistent settings

EEPROM emulation stores:

- Wi-Fi SSID/password
- device name
- sensor name
- signal pin
- dry/wet ADC calibration
- measurement interval
- sample count
- NTP server
- POSIX timezone
- language
- theme

Measurements themselves are RAM-only.

## OTA

OTA follows the same browser-assisted concept used by ESP8266 Moisture Sensor:

1. the browser requests the OTA manifest from the repository `ota` branch
2. the browser downloads `firmware.bin`
3. SHA-256 and size are checked in the browser when supplied by the manifest
4. the browser uploads the verified binary to the ESP8266
5. the ESP flashes the firmware and reboots

Repository:

`syschelle/esp8266-moisture-sensor--for-growtend-backend`

OTA branch:

`ota`

Manifest URL:

`https://raw.githubusercontent.com/syschelle/esp8266-moisture-sensor--for-growtend-backend/ota/manifest.json`


## GitHub Actions firmware artifact

Every build on `main`, every pull request, and every manually started workflow performs:

1. static project validation
2. PlatformIO build for `d1_mini`
3. creation of the OTA package
4. upload of the generated firmware package as a GitHub Actions artifact

After a successful workflow run, open the run summary and download:

`esp8266-moisture-sensor-v0.1.3`

The artifact contains:

- `firmware.bin`
- `firmware.bin.sha256`
- `manifest.json`
- `README.md`

The workflow uses a fixed `ubuntu-24.04` runner image rather than `ubuntu-latest`.


## GitHub Release assets

When a version tag such as `v0.1.3` is pushed, GitHub Actions now also creates or updates the matching GitHub Release and uploads the built firmware directly to **Releases -> Assets**.

Release assets:

- `firmware.bin`
- `firmware.bin.sha256`
- `manifest.json`
- `README.md`

This is in addition to the downloadable GitHub Actions artifact from the workflow run.

## Build

PlatformIO:

```powershell
pio run -e d1_mini
```

USB upload:

```powershell
pio run -e d1_mini -t upload
```

Serial monitor:

```powershell
pio device monitor
```

## First commissioning

1. Wire the sensor.
2. Flash firmware over USB.
3. Connect to `MoistureSensor-<CHIPID>` using password `MS-Setup-8266`.
4. Open `http://192.168.4.1`.
5. Configure Wi-Fi and reboot.
6. Open the sensor's normal LAN IP.
7. Calibrate dry.
8. Put the sensor into representative well-watered soil and calibrate wet.
9. Verify `/api/current-values`.

## Repository release documents

Only the current source version is kept in:

- `BUILD_STATUS.md`
- `RELEASE_NOTES.md`

Historical version-specific copies are intentionally not created. GitHub Releases are the release history.

## License

Apache License 2.0
