# ESP8266 Moisture Sensor for GrowTent Backend

This ESP8266-based sensor reads one capacitive analog soil-moisture sensor and exposes the current moisture value via HTTP for direct integration with the `syschelle/growtent-backend` project.

**Current version: v0.1.6**

## Scope

The firmware is intentionally focused on one task: measuring soil moisture and making that value available to the GrowTent backend.

There is:

- no segment display
- no local measurement display hardware
- no external temperature polling
- no external measurement server configuration
- no remote temperature cache

The ESP8266 itself is the measurement source.

## Hardware

- ESP8266, Wemos/Lolin D1 mini compatible target
- one capacitive analog soil-moisture sensor

Typical wiring:

- VCC -> 3.3 V
- GND -> GND
- AOUT -> A0

Verify the accepted ADC input voltage for the exact ESP8266 board before connecting the sensor.

## Signal pin

The signal pin is configurable in the web interface.

On ESP8266 v0.1.6 the supported analog input is:

`A0`

The configuration model remains generic so a future ESP32 variant can expose multiple ADC pins.

## Measurement

Defaults:

- measurement interval: 5 seconds
- 10 ADC samples per measurement
- sample averaging
- additional smoothing
- calibrated result limited to 0..100 %

Calibration:

- dry ADC value -> 0 %
- wet ADC value -> 100 %


## User interface behavior

The web interface keeps the established layout of the predecessor firmware: dark top bar, fixed left navigation, light content area and compact white status cards.

Live status polling runs automatically only while the **Status** page is active. Configuration fields are not periodically overwritten while the user is editing them. This prevents Wi-Fi credentials and other settings from being reset in the browser before they are saved.

## Web interface

The user interface contains only functions needed by this project:

- Status
- Sensor
- System settings
- System log
- OTA Update
- Factory reset

The Status page shows:

- current soil moisture
- raw ADC value
- signal pin
- dry/wet calibration values
- last measurement
- measurement interval
- local time / NTP
- IP address
- Wi-Fi RSSI
- uptime
- free heap
- firmware version

## GrowTent backend API

### Current values

`GET /api/current-values`

Example:

```json
{
  "device": "SoilSensor-1",
  "sensor": "Topf 1",
  "signal_pin": "A0",
  "firmware_version": "0.1.6",
  "raw_adc": 487,
  "moisture_percent": 63.4,
  "calibrated": true,
  "last_measurement_at": "2026-10-06 12:30:00",
  "wifi_rssi": -57,
  "uptime_seconds": 86423
}
```

### Health

`GET /api/health`

## Wi-Fi fallback AP

If no normal Wi-Fi connection can be established, the device starts its setup access point.

```text
SSID:     MoistureSensor-<CHIPID>
Password: MS-Setup-8266
```

Default device name:

```text
SoilSensor-1
```

The default AP address is normally:

`192.168.4.1`

## NTP

Defaults:

```text
Server:   de.pool.ntp.org
Timezone: CET-1CEST,M3.5.0,M10.5.0/3
```

## Build

```powershell
pio run -e d1_mini
```

## GitHub firmware release assets

Pushing a version tag triggers a firmware build. The GitHub Release receives:

- `firmware.bin`
- `firmware.bin.sha256`
- `manifest.json`
- `README.md`

## OTA

OTA uses the latest published GitHub Release of this repository. No separate `ota` branch is required.

Manifest:

`https://github.com/syschelle/esp8266-moisture-sensor--for-growtend-backend/releases/latest/download/manifest.json`

Firmware:

`https://github.com/syschelle/esp8266-moisture-sensor--for-growtend-backend/releases/latest/download/firmware.bin`

The browser-assisted update process is:

1. load `manifest.json` from the latest GitHub Release
2. compare the release version with the installed firmware
3. download `firmware.bin`
4. verify file size and SHA-256 from the manifest
5. upload the verified firmware to the ESP8266 over the local HTTP connection
6. reboot after a successful flash

The release workflow publishes these files directly under **GitHub Releases -> Assets**:

- `firmware.bin`
- `firmware.bin.sha256`
- `manifest.json`
- `README.md`

## License

Apache License 2.0
