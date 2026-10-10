# ESP8266 Moisture Sensor v0.1.18

This release fixes the GitHub / PlatformIO build failure introduced with the manual calibration feature.

## Build fix

The manual calibration handler in v0.1.17 referenced non-existent constant names:

- `SENSOR_ADC_MIN_PLAUSIBLE`
- `SENSOR_ADC_MAX_PLAUSIBLE`

The existing firmware constants are:

- `SENSOR_ADC_DISCONNECTED_MAX`
- `SENSOR_ADC_PLAUSIBLE_MAX`

The manual calibration validation now uses the existing canonical constants. ADC values up to and including `50` are treated as disconnected, so manual calibration values must be `51..1000`.

No behavior changes are made to the manual calibration limits.

## Tests

The regression suite now explicitly verifies that:

- manual calibration uses the canonical ADC plausibility constants
- the obsolete constant names cannot be reintroduced

## Version

v0.1.18
