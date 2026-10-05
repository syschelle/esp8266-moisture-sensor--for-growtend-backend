# Security Policy

## Supported version

The current source version is supported for security fixes.

## Network scope

This firmware is intended for a trusted local network. The built-in HTTP web interface and REST API do not provide user authentication in v0.1.0.

Do not expose the device web interface directly to the public Internet.

## OTA

Browser-assisted OTA uses a GitHub-hosted manifest and firmware binary. When the manifest provides a SHA-256 value, the browser verifies the downloaded firmware before uploading it to the ESP8266.

The ESP8266 itself accepts local OTA uploads from clients that can access its HTTP server. Keep the device on a trusted network.

## Wi-Fi setup AP

The fallback AP uses a fixed project default password documented in the README. Configure the normal Wi-Fi connection promptly and avoid leaving the fallback AP active unnecessarily.

## Reporting

Please report security issues privately to the repository owner rather than opening a public issue with exploit details.
