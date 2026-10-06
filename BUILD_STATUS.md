# Build / validation status — ESP8266 Moisture Sensor v0.1.6

## Completed in the artifact environment

- project structure validation: PASS
- release version consistency: PASS
- current-values API route: PASS
- health API route: PASS
- A0 signal-pin validation: PASS
- calibration guard: PASS
- configuration-form refresh protection: PASS
- GitHub Actions firmware build/artifact workflow: PASS
- GitHub Release asset publishing workflow: PASS
- latest-release OTA manifest URL: PASS
- latest-release firmware URL in generated manifest: PASS
- no OTA branch dependency: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## OTA source

Manifest:

`https://github.com/syschelle/esp8266-moisture-sensor--for-growtend-backend/releases/latest/download/manifest.json`

Firmware:

`https://github.com/syschelle/esp8266-moisture-sensor--for-growtend-backend/releases/latest/download/firmware.bin`

No separate OTA branch is required.

## Local build

```powershell
pio run -e d1_mini
```
