# ESP8266 Moisture Sensor v0.1.19

This release improves the readability of the OTA status message.

## OTA status readability

The message below the OTA progress bar, for example:

`Keine neuere Version verfügbar.`

previously used the generic muted hint style. On the white card background this resulted in low contrast.

The OTA status message now uses a dedicated style with:

- normal foreground text color
- stronger font weight
- subtle border
- separate background area
- improved padding and line height

This applies to update checks, download/install status, reboot status and errors.

## Version

v0.1.19
