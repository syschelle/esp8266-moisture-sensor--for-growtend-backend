# ESP8266 Moisture Sensor v0.1.26

This release hardens the Internet OTA path against GitHub Release redirect and empty-response problems observed in the System Log.

## GitHub redirect handling

The previous implementation relied on `HTTPClient` automatic redirect following.

The observed failures included:

- final HTTP `302` responses instead of the release asset
- HTTP `200` with the expected Content-Length but an empty manifest body

The firmware now follows GitHub redirects explicitly.

For each redirect it logs:

- redirect HTTP code
- redirect hop number
- Location header length
- destination host
- current free heap

The full signed GitHub asset URL is intentionally not written to the System Log.

## Manifest retries

Manifest retrieval now performs up to three attempts.

A retry is triggered when:

- a redirect is invalid or incomplete
- HTTP returns an unexpected response
- HTTP `200` contains an empty body
- the received body length does not match Content-Length
- JSON parsing fails

## Firmware download redirects

The firmware download uses the same explicit redirect strategy.

The generated manifest now points to the version-specific release asset:

`/releases/download/v<VERSION>/firmware.bin`

instead of:

`/releases/latest/download/firmware.bin`

This removes one unnecessary redirect from the actual firmware download.

## Manual OTA

The manual Wi-Fi firmware upload is unchanged.

## Version

v0.1.26
