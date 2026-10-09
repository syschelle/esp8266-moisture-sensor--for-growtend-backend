# Build / validation status — ESP8266 Moisture Sensor v0.1.16

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- ADC 0..50 disconnected detection: PASS
- ADC upper plausibility limit 1000: PASS
- factory calibration disabled: PASS
- implausible sensor blocks moisture percentage: PASS
- implausible sensor blocks calibration: PASS
- health endpoint requires plausible sensor: PASS
- disconnected status exposed in API/UI: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local checks

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```
