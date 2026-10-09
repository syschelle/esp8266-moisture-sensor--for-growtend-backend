# ESP8266 Moisture Sensor v0.1.16

This release adds sensor plausibility detection so an ESP8266 without a connected moisture sensor is no longer reported as 100% soil moisture.

## Disconnected sensor detection

ADC values from `0` through `50` are now treated as an implausible / disconnected sensor signal.

The upper plausibility limit is `1000`.

Examples observed during testing:

- ADC `6` without sensor -> sensor not detected
- ADC `9` without sensor -> sensor not detected
- ADC `622` with a sensor hanging in dry air -> plausible measurement

If the ADC value is implausible:

- soil moisture is shown as `--`
- the sensor badge shows `Sensor nicht erkannt`
- `moisture_percent` is `null`
- `/api/health` is `degraded`
- dry/wet calibration is blocked

## Factory calibration

Fresh/factory-reset devices no longer use the old example calibration values `800 / 400`.

The defaults are now `0 / 0`, so a real dry/wet calibration is required before a valid percentage can be produced.

Existing saved calibration values are retained after a normal firmware update, but the new plausibility check still prevents ADC values up to 50 from producing a percentage.

## Version

v0.1.16
