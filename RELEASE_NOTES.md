# ESP8266 Moisture Sensor v0.1.17

This release adds manual editing of the dry and wet calibration values.

## Manual calibration values

The Sensor page now includes two additional numeric fields:

- manual dry value
- manual wet value

The existing buttons for capturing the current ADC value as dry or wet remain available.

Manual values are validated both in the browser and by the ESP8266 firmware.

Allowed range:

- `50` to `1000`

Minimum difference between dry and wet:

- `40` ADC points

The values are stored persistently in EEPROM using the existing configuration storage.

New endpoint:

`POST /api/calibration/manual`

Parameters:

- `dry_adc`
- `wet_adc`

## Version

v0.1.17
