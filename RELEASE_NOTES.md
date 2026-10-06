# ESP8266 Moisture Sensor v0.1.6

This release fixes OTA update checks by switching completely from the unused `ota` branch to GitHub Release assets.

## OTA fix

The firmware no longer requests:

`https://raw.githubusercontent.com/syschelle/esp8266-moisture-sensor--for-growtend-backend/ota/manifest.json`

That URL returned HTTP 404 because no dedicated OTA branch is required by the current release workflow.

The firmware now uses the latest GitHub Release directly:

`https://github.com/syschelle/esp8266-moisture-sensor--for-growtend-backend/releases/latest/download/manifest.json`

The generated manifest points to:

`https://github.com/syschelle/esp8266-moisture-sensor--for-growtend-backend/releases/latest/download/firmware.bin`

## Release workflow

The existing tag workflow already builds and publishes:

- `firmware.bin`
- `firmware.bin.sha256`
- `manifest.json`
- `README.md`

These files are now both published and consumed from the same GitHub Release path.

## Result

The OTA process now has one consistent source of truth:

`Git tag -> GitHub Actions -> GitHub Release -> OTA update`

No separate `ota` branch is needed.

## Version

v0.1.6
