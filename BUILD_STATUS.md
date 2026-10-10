# Build / validation status — ESP8266 Moisture Sensor v0.1.22

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- install button hidden while release notes load: PASS
- embedded release notes path: PASS
- compatibility fallback waits for release notes: PASS
- install button enabled only after release notes are ready: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local checks

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```
