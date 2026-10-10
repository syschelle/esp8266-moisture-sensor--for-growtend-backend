# ESP8266 Moisture Sensor v0.1.23

This release changes the automatic NTP synchronization interval from the ESP8266 core default of one hour to once per day.

## Daily NTP synchronization

The ESP8266 Arduino core normally refreshes SNTP time every hour.

The firmware now overrides the SNTP update interval to:

`24 hours`

The initial NTP synchronization after boot is unchanged. The configured NTP server and timezone are also unchanged.

Saving the NTP server or timezone in System Settings still immediately reconfigures the time service.

## Version

v0.1.23
