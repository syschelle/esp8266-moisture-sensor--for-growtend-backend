# Build / validation status — ESP8266 Moisture Sensor v0.1.1

## Completed in the artifact environment

- project structure validation: PASS
- release version consistency: PASS
- required API route presence: PASS
- `A0` signal-pin validation guard: PASS
- calibration span guard: PASS
- Wi-Fi fallback AP configuration presence: PASS
- OTA repository / manifest configuration presence: PASS
- canonical release-document policy: PASS
- JavaScript syntax check: PASS
- GitHub Actions firmware-artifact workflow validation: PASS
- ZIP packaging and SHA-256 generation: PASS

## GitHub Actions workflow

The workflow now:

1. runs on `ubuntu-24.04`
2. installs PlatformIO
3. runs `scripts/validate_project.py`
4. builds with `pio run -e d1_mini`
5. creates the OTA package with `scripts/make_ota.py`
6. verifies the generated files
7. uploads `esp8266-moisture-sensor-v0.1.1` as a GitHub Actions artifact

Expected artifact contents:

- `firmware.bin`
- `firmware.bin.sha256`
- `manifest.json`
- `README.md`

## Local firmware build

The authoritative local build command remains:

```powershell
pio run -e d1_mini
```

## Repository release-document policy

Only the current source version keeps these release documents in the repository root:

- `BUILD_STATUS.md`
- `RELEASE_NOTES.md`

Historical `BUILD_STATUS_v*.md` and `RELEASE_NOTES_v*.md` files are not added.
