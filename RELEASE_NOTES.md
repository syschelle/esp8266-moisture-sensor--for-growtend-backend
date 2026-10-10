# ESP8266 Moisture Sensor v0.1.28

This release rolls back the manifest design change that introduced Internet OTA instability.

## Root cause

Up to v0.1.20, `manifest.json` contained only compact firmware metadata.

Starting with v0.1.21, the complete release text was embedded into `manifest.json` to save a second HTTPS request.

The v0.1.27 System Log identified the resulting memory problem:

`manifest String reserve failed for 2237 B`

The ESP8266 still reported more than 25 KB of total free heap, but TLS processing, long GitHub redirect URLs and log Strings had fragmented the heap. A contiguous 2237-byte String allocation was therefore no longer available.

## Compact manifest restored

The manifest contains only:

- version
- firmware URL
- firmware size
- SHA-256

Release notes are again stored separately in the release `README.md` asset.

The OTA page waits for the release text to load successfully before showing **Install update**.

## Fixed manifest buffer

The manifest body no longer requires a dynamically allocated String.

It is read into a fixed 768-byte buffer, avoiding the large contiguous heap allocation that failed in v0.1.27.

The existing GitHub redirect diagnostics and retry handling are retained.

## Firmware download

The version-specific firmware URL and streamed firmware download remain in place.

## Manual OTA and NTP

The working manual Wi-Fi firmware upload is unchanged.

Daily NTP synchronization and NTP synchronization logging are unchanged.

## Version

v0.1.28
