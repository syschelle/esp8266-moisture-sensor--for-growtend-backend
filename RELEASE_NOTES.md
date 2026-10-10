# ESP8266 Moisture Sensor v0.1.29

This release adds a clipboard button to the System Log page.

## Copy System Log

The **Systemprotokoll / System log** page now contains a **Copy to clipboard** button next to **Refresh**.

The button copies the complete currently displayed System Log.

For browsers that support the modern Clipboard API in the current context, `navigator.clipboard` is used.

Because the ESP8266 web interface is normally opened through a local HTTP address, the UI also contains a fallback based on a temporary textarea and `execCommand('copy')`.

After the action, the page reports whether the log was copied successfully.

## No firmware logic changes

Internet OTA, manual OTA, NTP synchronization, sensor measurement and calibration logic are unchanged from v0.1.28.

## Version

v0.1.29
