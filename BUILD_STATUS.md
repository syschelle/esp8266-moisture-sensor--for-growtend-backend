# Build / validation status — ESP8266 Moisture Sensor v0.1.12

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- mixed-type `min()` OTA buffer expression removed: PASS
- warning regressions from v0.1.11 remain fixed: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local checks

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```

The final ESP8266 compile/link result is still confirmed by PlatformIO locally or in GitHub Actions.
