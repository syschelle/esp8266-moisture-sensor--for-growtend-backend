# ESP8266 Moisture Sensor v0.1.15

This release adds OTA release-change information and merges the duplicate sensor/device naming.

## OTA release changes

When a newer firmware version is available, the OTA page now displays a new card below **Manual firmware update** containing the changes for that release.

The GitHub Release asset `README.md` is now generated directly from `RELEASE_NOTES.md`.

The browser does not fetch GitHub directly. The ESP8266 provides the release text through:

`GET /api/ota/readme`

The card is only shown when a newer firmware version is available.

## Unified device name

The separate sensor name and device name have been merged.

The device name is now the single identity used for:

- web interface header
- browser tab
- sensor label on the Status page
- `/api/current-values` device field
- `/api/current-values` sensor compatibility field
- network hostname

The old `sensorName` EEPROM field remains only for configuration-layout compatibility and is automatically synchronized with the device name.

## Hostname-safe naming

Because the device name is also used as the network hostname, only the following characters are accepted:

- `A-Z`
- `a-z`
- `0-9`
- `-`

Spaces and umlauts are rejected. A hyphen cannot be the first or last character.

Valid example:

`Topf-1`

Invalid examples:

`Topf 1`

`Töpf-1`

## Version

v0.1.15
