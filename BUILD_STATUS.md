# Build / validation status — ESP8266 Moisture Sensor v0.1.3

## Completed in the artifact environment

- project structure validation: PASS
- release version consistency: PASS
- required API route presence: PASS
- `A0` signal-pin validation guard: PASS
- calibration span guard: PASS
- Wi-Fi fallback AP configuration presence: PASS
- default AP SSID naming guard: PASS
- default device name guard: PASS
- default AP password guard: PASS
- no legacy display-project naming guard: PASS
- OTA repository / manifest configuration presence: PASS
- canonical release-document policy: PASS
- JavaScript syntax check: PASS
- GitHub Actions firmware artifact workflow validation: PASS
- GitHub Release asset publishing workflow validation: PASS
- ZIP packaging and SHA-256 generation: PASS

## Runtime identity

Expected defaults:

- AP SSID: `MoistureSensor-<CHIPID>`
- device name: `SoilSensor-1`
- AP password: `MS-Setup-8266`

## Local firmware build

```powershell
pio run -e d1_mini
```
