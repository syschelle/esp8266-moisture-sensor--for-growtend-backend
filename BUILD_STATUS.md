# Build / validation status — ESP8266 Moisture Sensor v0.1.30

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- latest version determined from `/releases/latest` redirect: PASS
- update check no longer downloads release manifest: PASS
- detailed version-check logging: PASS
- release-note endpoint uses explicit version: PASS
- cached-manifest dependency removed from release notes: PASS
- release notes use tagged `raw.githubusercontent.com`: PASS
- install request uses explicit target version: PASS
- compact manifest fetched only when install starts: PASS
- manifest target-version validation: PASS
- System Log copy button retained: PASS
- manual OTA retained: PASS
- NTP logging retained: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local / CI verification

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```

The final ESP8266 compile/link result is confirmed by PlatformIO locally or in GitHub Actions.
