# Build / validation status — ESP8266 Moisture Sensor v0.1.21

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- release notes embedded in OTA manifest: PASS
- second blocking README request removed from current manifests: PASS
- README endpoint retained as compatibility fallback: PASS
- broken `uploadBuf()` call removed: PASS
- manual OTA multipart upload: PASS
- manual OTA progress reporting: PASS
- manual OTA backend diagnostics: PASS
- JavaScript syntax: PASS
- ZIP packaging and SHA-256 generation: PASS

## Local checks

```powershell
python scripts/validate_project.py
python tests/internal_tests.py
pio run -e d1_mini
```

The final ESP8266 compile/link result is confirmed by PlatformIO locally or in GitHub Actions.
