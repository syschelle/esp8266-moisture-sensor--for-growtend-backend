# Build / validation status — ESP8266 Moisture Sensor v0.1.0

## Completed in the artifact environment

- project structure validation: PASS
- release version consistency: PASS
- required API route presence: PASS
- `A0` signal-pin validation guard: PASS
- calibration span guard: PASS
- Wi-Fi fallback AP configuration presence: PASS
- OTA repository / manifest configuration presence: PASS
- canonical release-document policy: PASS
- JavaScript extraction and syntax check: PASS
- ZIP packaging and SHA-256 generation: PASS

## Not available in this environment

PlatformIO (`pio`) is not installed in the artifact environment, so the real ESP8266 compile/link build was not executed here.

Before tagging the release, run:

```powershell
pio run -e d1_mini
```

A successful PlatformIO build is the authoritative firmware compile/link validation.

## Repository release-document policy

Only the current files are kept in the repository root:

- `BUILD_STATUS.md`
- `RELEASE_NOTES.md`

Do not add historical `BUILD_STATUS_v*.md` or `RELEASE_NOTES_v*.md` files.
