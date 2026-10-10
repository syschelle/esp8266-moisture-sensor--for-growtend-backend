# ESP8266 Moisture Sensor v0.1.21

This release fixes manual firmware upload over Wi-Fi and removes an unnecessary delay from the normal OTA update check.

## Faster OTA update check

The OTA page previously caused two separate HTTPS requests through the ESP8266 when a newer version was available:

1. `manifest.json`
2. release `README.md`

Because the ESP8266 web server is single-threaded, the second GitHub TLS request temporarily blocked the local web interface and made the update process feel slower.

Release notes are now included directly in `manifest.json`, so version information, firmware metadata and release changes are obtained in one request.

The existing `/api/ota/readme` endpoint remains as a compatibility fallback for older manifests, but the UI no longer waits for that second request.

The actual firmware download, SHA-256 verification, flash write and normal reboot logic remain unchanged.

## Manual firmware update over Wi-Fi

Manual firmware upload is designed to work over the normal WLAN connection.

The previous web UI called a non-existent JavaScript function named `uploadBuf()`. Therefore the manual upload could not work.

The manual updater now:

- uploads `firmware.bin` using `multipart/form-data`
- sends it to `POST /api/ota/upload`
- shows real upload progress
- waits for the ESP8266 to reboot
- reconnects to the device afterwards
- logs manual OTA start, install, errors and reboot in the System log

## Version

v0.1.21
