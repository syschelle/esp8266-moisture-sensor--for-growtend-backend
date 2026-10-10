# ESP8266 Moisture Sensor for GrowTent Backend

This ESP8266-based sensor reads one capacitive analog soil-moisture sensor and exposes the current moisture value via HTTP for direct integration with the `syschelle/growtent-backend` project.

**Current version: v0.1.29**

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

On ESP8266 v0.1.29 the supported analog input is:

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
  "firmware_version": "0.1.29",
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

## Device name and hostname

The firmware uses one device name for both the sensor identity and the network hostname.

Allowed characters:

- `A-Z`
- `a-z`
- `0-9`
- `-`

Spaces and umlauts are not accepted. The name cannot start or end with a hyphen.

The same value is shown in the web interface and browser tab and is returned through the API.

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


## Local time and timezone

The ESP8266 uses the POSIX timezone overload of `configTime()` so that daylight-saving time is handled by the firmware itself.

Default for Germany:

`CET-1CEST,M3.5.0,M10.5.0/3`

This means:

- winter: CET / UTC+1
- summer: CEST / UTC+2

The configured timezone is applied both during boot and immediately after saving System settings.

## NTP

Defaults:

```text
Server:   de.pool.ntp.org
Timezone: CET-1CEST,M3.5.0,M10.5.0/3
```

## Internal regression tests

The project contains a host-side regression suite:

```powershell
python tests/internal_tests.py
```

It verifies moisture conversion, calibration guards, timezone handling, OTA behavior, UI safeguards and release-pipeline assumptions.

GitHub Actions executes these tests automatically before the PlatformIO firmware build.

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



### NTP synchronization interval

After the initial synchronization at boot, SNTP refreshes the clock every 24 hours. The NTP server and POSIX timezone remain configurable in System Settings.

## OTA release changes

When a newer firmware is available, the OTA page shows the release changes in a separate card below the manual firmware-update card.

The release `README.md` asset is generated from `RELEASE_NOTES.md` and is retrieved by the ESP8266 through `/api/ota/readme`.

## OTA user experience


The OTA page first checks for a newer release. The **Install update** button is only shown when a newer firmware version is available. Starting an update is acknowledged immediately by the ESP; the browser then waits for the device to reboot and verifies the installed version. Temporary connection loss during flashing/reboot is expected and is not shown as `Failed to fetch`.

The System log records the manifest check, available release, update scheduling, download progress (25/50/75/100 %), SHA-256 verification, firmware finalization and reboot scheduling. A compact EEPROM record allows successful installation to be reported after reboot.

## OTA


OTA checks and firmware downloads are performed by the ESP8266 itself. The browser only communicates with the local device, so browser CORS and GitHub redirect behavior do not affect the update flow.

Local OTA endpoints:

- `GET /api/ota/check`
- `POST /api/ota/update`

The ESP8266 loads the manifest from the latest GitHub Release:

`https://github.com/syschelle/esp8266-moisture-sensor--for-growtend-backend/releases/latest/download/manifest.json`

Automatic OTA flow:

1. ESP downloads `manifest.json`
2. ESP compares versions
3. ESP downloads `firmware.bin` over HTTPS
4. ESP verifies the expected size
5. ESP calculates SHA-256 while streaming the firmware
6. ESP compares SHA-256 with the manifest
7. ESP finalizes the update only after successful verification
8. ESP reboots

Manual local `firmware.bin` upload remains available as a fallback.

## License

Apache License 2.0


## Manual OTA over Wi-Fi

The manual firmware-update form uploads a local `firmware.bin` through the ESP8266 web interface over the current WLAN connection.

The browser sends the file as `multipart/form-data`, displays upload progress and waits for the ESP8266 to reboot and reconnect.

## OTA update-check performance

Release changes are embedded directly in `manifest.json`. This avoids a second blocking GitHub HTTPS request during normal update checks. The separate README endpoint remains available only as a compatibility fallback for older release manifests.


### Internet OTA diagnostics

The Internet OTA path writes detailed network diagnostics to the System Log: HTTP timings/codes, Wi-Fi RSSI, free heap, response sizes, download throughput and data-stall information.

A compact diagnostic record is also persisted in EEPROM so relevant Internet OTA metrics can be reported after a reboot.


### NTP synchronization logging

Every successful SNTP synchronization is reported in the System Log. The entry includes the resulting local date/time, configured NTP server, configured POSIX timezone and Wi-Fi RSSI.

The existing automatic synchronization interval remains 24 hours.


### Robust GitHub OTA redirects

Internet OTA follows GitHub Release redirects explicitly instead of relying on automatic `HTTPClient` redirect handling. Manifest requests are retried when a successful HTTP response contains an empty or incomplete body.

The firmware asset URL stored in new manifests is version-specific, reducing the number of redirects required for firmware download.


### Streamed GitHub manifest body

The Internet OTA manifest body is read directly from the HTTP stream until the expected Content-Length has been received. This avoids relying on `HTTPClient::getString()` and on the remote connection remaining marked as connected after the server has sent and closed the response.

Failed body reads retry the already-resolved GitHub release asset URL where possible.


### Compact OTA manifest

The OTA manifest contains firmware metadata only. Release notes are delivered separately through the release `README.md` asset.

The ESP8266 reads the compact manifest into a fixed-size buffer instead of allocating a large dynamic String, reducing heap-fragmentation sensitivity during TLS and GitHub redirect processing.


### Copy System Log

The System Log page includes a **Copy to clipboard** button. The complete currently displayed log can be copied directly for diagnostics and support. A fallback is included for browsers opening the ESP8266 interface over local HTTP.
