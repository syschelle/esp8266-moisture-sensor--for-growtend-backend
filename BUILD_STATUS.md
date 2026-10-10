# Build / validation status — ESP8266 Moisture Sensor v0.1.28

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- compact metadata-only manifest restored: PASS
- embedded release notes removed from manifest: PASS
- separate release README retained: PASS
- fixed 768-byte manifest buffer: PASS
- large manifest String allocation removed: PASS
- install button waits for separate release text: PASS
- GitHub redirect diagnostics retained: PASS
- version-specific firmware URL retained: PASS
- manual OTA implementation retained: PASS
- NTP synchronization logging retained: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local / CI verification

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```

The final ESP8266 compile/link result is confirmed by PlatformIO locally or in GitHub Actions.
