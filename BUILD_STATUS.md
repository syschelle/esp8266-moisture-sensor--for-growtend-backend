# Build / validation status — ESP8266 Moisture Sensor v0.1.27

## Completed in the artifact environment

- static project validation: PASS
- internal regression tests: PASS
- `HTTPClient::getString()` removed from manifest path: PASS
- streamed manifest body reader: PASS
- expected Content-Length body read: PASS
- manifest body read independent of HTTP connected state: PASS
- 3-second manifest body idle timeout: PASS
- resolved asset URL retained for retries: PASS
- firmware stream no longer gated by `http.connected()`: PASS
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
