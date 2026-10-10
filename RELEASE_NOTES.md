# ESP8266 Moisture Sensor v0.1.22

This release changes the OTA page so the **Install update** button is only shown after the release notes have been loaded successfully.

## OTA install button behavior

When a newer firmware version is found, the page now follows this sequence:

1. detect the available firmware
2. load and display the release notes
3. enable and show **Install update**

While the release notes are loading, the install button remains hidden and disabled.

If the release notes cannot be loaded, the firmware is still reported as available, but the install button remains hidden.

For current releases the notes are embedded directly in `manifest.json`, so no additional GitHub request is normally required.

Older manifests still use `/api/ota/readme` as a compatibility fallback. In that fallback case the UI waits for the release text before offering installation.

## Version

v0.1.22
