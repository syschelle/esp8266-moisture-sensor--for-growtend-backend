# ESP8266 Moisture Sensor v0.1.25

This release adds detailed System Log entries after every successful NTP/SNTP time synchronization.

## NTP synchronization logging

The ESP8266 time-set callback is now monitored by the main loop.

After every successful NTP update, the System Log records:

- determined local date and time
- configured NTP server
- configured POSIX timezone
- current Wi-Fi RSSI

Example:

`NTP: synchronized local=2026-10-10 18:55:03 | server=de.pool.ntp.org | TZ=CET-1CEST,M3.5.0,M10.5.0/3 | RSSI=-58 dBm`

This applies to the initial successful synchronization after boot as well as the automatic daily refresh.

## Daily NTP interval

The existing 24-hour automatic SNTP refresh interval is unchanged.

## Version

v0.1.25
