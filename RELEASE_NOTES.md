# ESP8266 Moisture Sensor v0.1.24

This release adds detailed diagnostics for the Internet OTA update path.

The manual Wi-Fi firmware upload is unchanged.

## Internet OTA diagnostics

The System Log now records additional information for the GitHub OTA process, including:

- Wi-Fi RSSI before Internet requests
- free heap before TLS / firmware download
- manifest HTTP result
- manifest request duration
- manifest response size
- release-note size
- expected firmware size
- available sketch space
- firmware HTTP result
- time until firmware response headers arrive
- firmware Content-Length
- download progress with elapsed time and average KiB/s
- 5 / 10 / 15 second data-stall messages
- number of bytes received before a timeout or incomplete download
- final download duration and average transfer rate
- more detailed messages for size mismatch, Update.begin, flash-write and finalize failures

## Persistent diagnostics

A small separate OTA diagnostic record is stored in EEPROM.

After a reboot, the System Log can report the previous Internet OTA request timings, HTTP codes, transferred bytes, Wi-Fi RSSI and free heap.

The existing OTA result record layout is retained unchanged.

## No OTA behavior change

This release is intended for diagnosis. It does not change:

- firmware URL handling
- TLS mode
- redirect handling
- SHA-256 verification
- firmware flashing
- reboot workflow
- manual firmware upload

## Version

v0.1.24
