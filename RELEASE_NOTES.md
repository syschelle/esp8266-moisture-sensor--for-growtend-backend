# ESP8266 Moisture Sensor v0.1.12

This release fixes an IntelliSense / C++ type mismatch in the OTA download buffer calculation.

## Build fix

The previous code used:

`min(available, sizeof(buffer))`

Depending on the host/compiler type definitions, the arguments can be interpreted as different unsigned integer types (`size_t` vs. `unsigned long long`), which causes overload resolution to fail.

The code now uses an explicit type-safe conditional expression:

`size_t want = (available < sizeof(buffer)) ? available : sizeof(buffer);`

This removes the overload ambiguity without changing behavior.

## Tests

The internal regression test suite now checks that the mixed-type `min()` expression is not reintroduced.

## Version

v0.1.12
