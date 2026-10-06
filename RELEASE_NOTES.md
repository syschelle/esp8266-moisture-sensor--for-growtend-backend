# ESP8266 Moisture Sensor v0.1.8

This release fixes the two-hour local-time offset observed during daylight-saving time.

## Timezone fix

The previous implementation configured NTP with the integer-offset `configTime()` overload after setting the POSIX timezone separately. On ESP8266 this could result in UTC being shown even though the stored timezone was correct.

The firmware now uses the ESP8266 POSIX timezone overload directly:

`configTime(cfg.timezone, cfg.ntpServer)`

The configured timezone is still applied to the C runtime with `TZ` / `tzset()` and timestamps are formatted using `localtime_r()`.

Default timezone:

`CET-1CEST,M3.5.0,M10.5.0/3`

This automatically provides:

- CET / UTC+1 in winter
- CEST / UTC+2 in summer

Saving System settings immediately reapplies NTP and the timezone; a reboot is not required.

## Version

v0.1.8
