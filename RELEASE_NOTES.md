# ESP8266 Moisture Sensor v0.1.13

This release fixes the main content being hidden underneath the fixed top header.

## UI layout fix

The top header is fixed at 49 px height. The main content previously started too close to the top of the viewport, causing the page title and the upper-right status badge to be partially covered by the header.

The desktop content area now includes the header height in its top spacing.

Desktop:

- fixed header: 49 px
- content top padding: 87 px

Tablet layout receives the same correction.

The mobile layout is unchanged because the top header becomes static there.

## Tests

The internal regression suite now checks that the desktop and tablet layouts keep the required clearance below the fixed header.

## Version

v0.1.13
