# ESP8266 Moisture Sensor v0.1.10

This release fixes an ESP8266 build error in the OTA failure handling.

## Build fix

The previous OTA implementation used:

`Update.abort()`

`UpdaterClass` in the ESP8266 Arduino core does not provide an `abort()` method, which caused compilation to fail.

The unsupported calls have been removed.

If an OTA download, flash write, timeout or SHA-256 verification fails, the firmware now records the error and schedules a restart without calling `Update.end()`. This discards the unfinished OTA session while keeping the currently installed firmware active.

## OTA behavior retained

- update availability check
- conditional `Update installieren` button
- no confirmation alert
- OTA process logging
- SHA-256 verification
- automatic reboot/reconnect handling
- persistent OTA result reporting

## Version

v0.1.10
