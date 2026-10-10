# Build / validation status — ESP8266 Moisture Sensor v0.1.18

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- canonical ADC plausibility constants: PASS
- obsolete manual-calibration constant names removed: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local checks

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```

The final ESP8266 compile/link result is confirmed by PlatformIO locally or in GitHub Actions.
