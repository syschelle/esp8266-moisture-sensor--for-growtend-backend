#!/usr/bin/env python3
from pathlib import Path
import re, sys

root = Path(__file__).resolve().parents[1]
checks = []

def require(cond, name):
    checks.append((name, bool(cond)))

version_h = (root / "include" / "version.h").read_text(encoding="utf-8")
main = (root / "src" / "main.cpp").read_text(encoding="utf-8")
ui = (root / "src" / "web_ui.h").read_text(encoding="utf-8")
readme = (root / "README.md").read_text(encoding="utf-8")
release = (root / "RELEASE_NOTES.md").read_text(encoding="utf-8")

require('#define APP_VERSION "0.1.0"' in version_h, "version header")
require("v0.1.0" in readme and "v0.1.0" in release, "release version consistency")
require('"/api/current-values"' in main, "current-values API")
require('"/api/health"' in main, "health API")
require('pin != "A0"' in main, "A0 pin validation")
require("MIN_CALIBRATION_SPAN" in main, "calibration span guard")
require("MS-Setup-8266" in main and "MS-Setup-8266" in readme, "fallback AP password documented")
require("esp8266-moisture-sensor--for-growtend-backend" in version_h, "OTA repository")
require("TM1637" not in main and "TM1637" not in ui, "no display runtime code")
require(not list(root.glob("BUILD_STATUS_v*.md")), "no historical build status files")
require(not list(root.glob("RELEASE_NOTES_v*.md")), "no historical release notes files")

failed = [name for name, ok in checks if not ok]
for name, ok in checks:
    print(("PASS" if ok else "FAIL") + " - " + name)
if failed:
    sys.exit(1)
