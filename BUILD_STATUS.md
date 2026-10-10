# Build / validation status — ESP8266 Moisture Sensor v0.1.29

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- System Log clipboard button: PASS
- modern Clipboard API path: PASS
- local HTTP clipboard fallback: PASS
- DE/EN clipboard labels: PASS
- clipboard result message: PASS
- JavaScript syntax: PASS
- firmware runtime logic unchanged from v0.1.28 except version: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local / CI verification

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```

The final ESP8266 compile/link result is confirmed by PlatformIO locally or in GitHub Actions.
