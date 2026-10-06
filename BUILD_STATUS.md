# Build / validation status — ESP8266 Moisture Sensor v0.1.15

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- unified device/sensor identity: PASS
- hostname-safe naming validation: PASS
- duplicate sensor-name input removed: PASS
- browser title uses device name: PASS
- OTA README endpoint: PASS
- OTA release-change card: PASS
- OTA README generated from release notes: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local checks

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```
