# ESP8266 Moisture Sensor v0.1.14

This release changes the browser tab title to use the configured sensor name.

## Browser tab title

The web interface now sets `document.title` from the current sensor configuration.

Priority:

1. sensor name
2. device name
3. `ESP8266 Moisture Sensor` fallback

Example:

If the configured sensor name is:

`Topf 1`

the browser tab will display:

`Topf 1`

instead of:

`ESP8266 Moisture Sensor`

## Tests

The internal regression suite now verifies the dynamic browser title behavior and fallback title.

## Version

v0.1.14
