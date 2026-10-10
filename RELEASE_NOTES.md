# ESP8266 Moisture Sensor v0.1.30

This release redesigns the Internet OTA check so version detection and release-note loading are faster and no longer depend on cached manifest state.

## Faster version detection

The old **Check update** flow downloaded the GitHub Release `manifest.json` through the complete release-asset redirect chain before the UI could display the available version.

The ESP8266 now requests only:

`/releases/latest`

with redirects disabled.

GitHub's first redirect contains the latest release tag. The firmware extracts the version directly from that redirect.

The version check therefore normally uses a single GitHub TLS request and does not download a release asset.

The System Log now records:

- version-check attempt
- HTTP status
- request duration
- Location header length
- parsed latest version
- Wi-Fi RSSI
- free heap

## Release-text error fixed

The error:

`{"error":"No newer firmware selected"}`

came from `/api/ota/readme` depending on `cachedOtaManifestValid`.

That dependency is removed.

The browser now explicitly requests:

`/api/ota/readme?version=<VERSION>`

The ESP8266 loads the matching tagged `RELEASE_NOTES.md` directly from `raw.githubusercontent.com`.

This avoids the GitHub Release asset redirect chain for the release text.

The System Log records the requested version, host, HTTP result, duration and received size.

## Manifest only when installation starts

The compact `manifest.json` is no longer downloaded during **Check update**.

It is fetched only when **Install update** is pressed.

The selected version is sent explicitly to the ESP8266. The downloaded manifest must match that version before the update is scheduled.

## Existing functionality retained

- compact metadata-only manifest
- version-specific firmware URL
- SHA-256 verification
- Internet OTA diagnostics
- manual Wi-Fi firmware update
- daily NTP synchronization and NTP logging
- System Log copy-to-clipboard button

## Version

v0.1.30
