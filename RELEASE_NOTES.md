# ESP8266 Moisture Sensor v0.1.20

This release reverts the broader OTA status styling change and keeps only the requested text-color adjustment.

## OTA status text color

The OTA page keeps the original layout and original `hint` styling.

Only the text color of the OTA status line is changed to the normal foreground color so messages such as:

`Keine neuere Version verfügbar.`

are easier to read on the white card background.

No OTA workflow, update endpoint, download logic, install logic, reboot handling, release-note loading or button behavior has been changed from v0.1.18.

## Version

v0.1.20
