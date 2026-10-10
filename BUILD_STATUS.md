# Build / validation status — ESP8266 Moisture Sensor v0.1.17

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- manual dry/wet calibration endpoint: PASS
- server-side ADC range validation: PASS
- server-side minimum-span validation: PASS
- manual calibration UI: PASS
- client-side validation: PASS
- EEPROM persistence path retained: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local checks

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```
