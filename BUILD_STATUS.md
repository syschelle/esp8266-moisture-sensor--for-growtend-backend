# Build / validation status — ESP8266 Moisture Sensor v0.1.20

## Completed in the artifact environment

- based directly on v0.1.18: PASS
- OTA workflow code unchanged: PASS
- original OTA status markup retained: PASS
- only OTA status text color overridden: PASS
- no custom OTA status box styling: PASS
- static project validation: PASS
- internal regression tests: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local checks

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```

The final ESP8266 compile/link result is confirmed by PlatformIO locally or in GitHub Actions.
