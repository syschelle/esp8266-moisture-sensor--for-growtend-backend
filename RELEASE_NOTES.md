# ESP8266 Moisture Sensor v0.1.27

This release fixes the next Internet OTA failure identified by the v0.1.26 diagnostics.

## Root cause identified

The GitHub redirect chain itself now completes successfully.

The System Log showed the final release asset returning:

- HTTP `200`
- valid `Content-Length`
- but `HTTPClient::getString()` returned an empty payload

Example from the field log:

`manifest HTTP 200 ... length 1771`

followed by:

`manifest payload 0 B`

The manifest JSON was therefore never received by the parser.

## Streamed manifest body reader

The manifest is no longer read with `HTTPClient::getString()`.

The firmware now reads the response body directly from the HTTP stream until the expected `Content-Length` has been received.

This body reader does not stop merely because the remote HTTP connection has already been marked as closed. That is important for GitHub release assets, where the server may close the connection immediately after sending the response.

The manifest body reader:

- waits for the expected number of bytes
- logs current body progress while waiting
- uses a 3-second idle timeout
- validates the final body length
- requests identity encoding
- retries on an empty or incomplete body

## Faster retries

After GitHub has already redirected the manifest request to the final release-assets host, a failed body read now retries that resolved URL directly instead of repeating the complete GitHub redirect chain.

The retry log shows the host that will be retried.

## Firmware download robustness

The firmware download loop had the same dependency on `http.connected()`.

It now continues reading until the complete expected firmware size has arrived or the existing data timeout is reached.

SHA-256 verification and flash handling are unchanged.

## Manual OTA

The working manual Wi-Fi firmware upload remains unchanged.

## Version

v0.1.27
