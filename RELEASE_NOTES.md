# ESP8266 Moisture Sensor v0.1.7

This release fixes the OTA `Failed to fetch` error.

## OTA architecture

The browser no longer downloads GitHub release assets directly.

New local endpoints:

- `GET /api/ota/check`
- `POST /api/ota/update`

The ESP8266 now performs the complete GitHub OTA transaction itself.

## Automatic update flow

1. browser asks the local ESP for an update check
2. ESP downloads `manifest.json` from the latest GitHub Release
3. ESP compares firmware versions
4. browser starts the update through the local endpoint
5. ESP downloads `firmware.bin`
6. firmware size is checked
7. SHA-256 is calculated while streaming
8. SHA-256 is compared with the manifest
9. the update is finalized only after successful verification
10. ESP reboots

This removes browser CORS and GitHub redirect issues from the OTA process.

## Manual OTA

Manual local `firmware.bin` upload remains available as a fallback.

## Version

v0.1.7
