# Build / validation status — ESP8266 Moisture Sensor v0.1.13

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- fixed-header desktop clearance: PASS
- fixed-header tablet clearance: PASS
- mobile static-header layout retained: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local checks

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```
