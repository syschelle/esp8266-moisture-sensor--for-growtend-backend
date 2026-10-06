# Build / validation status — ESP8266 Moisture Sensor v0.1.8

## Completed in the artifact environment

- project structure validation: PASS
- release version consistency: PASS
- ESP8266 POSIX-TZ `configTime()` overload: PASS
- explicit `TZ` environment application: PASS
- `tzset()` activation: PASS
- `localtime_r()` timestamp conversion: PASS
- CET/CEST default timezone: PASS
- System-settings timezone reapply path retained: PASS
- OTA architecture from v0.1.7 retained: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Default timezone

`CET-1CEST,M3.5.0,M10.5.0/3`

The real firmware compile/link build must still be confirmed by PlatformIO/GitHub Actions.

## Local build

```powershell
pio run -e d1_mini
```
