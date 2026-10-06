# Build / validation status — ESP8266 Moisture Sensor v0.1.7

## Completed in the artifact environment

- project structure validation: PASS
- release version consistency: PASS
- local OTA check endpoint: PASS
- local OTA update endpoint: PASS
- ESP-side HTTPS manifest fetch: PASS
- ESP-side HTTPS firmware download: PASS
- HTTP redirect following: PASS
- streamed SHA-256 implementation: PASS
- browser no longer fetches GitHub OTA assets directly: PASS
- manual OTA upload retained: PASS
- JavaScript syntax: PASS
- GitHub Actions firmware build/artifact workflow: PASS
- GitHub Release asset workflow: PASS
- ZIP packaging and SHA-256 generation: PASS

## Important

The real ESP8266 compile/link build must still be confirmed by PlatformIO/GitHub Actions.

## Local build

```powershell
pio run -e d1_mini
```
