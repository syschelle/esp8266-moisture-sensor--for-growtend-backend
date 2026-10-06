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

require('#define APP_VERSION "0.1.3"' in version_h, "version header")
require("v0.1.3" in readme and "v0.1.3" in release, "release version consistency")
require('"/api/current-values"' in main, "current-values API")
require('"/api/health"' in main, "health API")
require('pin != "A0"' in main, "A0 pin validation")
require("MIN_CALIBRATION_SPAN" in main, "calibration span guard")
require("MS-Setup-8266" in main and "MS-Setup-8266" in readme, "fallback AP password documented")
require("esp8266-moisture-sensor--for-growtend-backend" in version_h, "OTA repository")
require("TM1637" not in main and "TM1637" not in ui, "no display runtime code")
require(not list(root.glob("BUILD_STATUS_v*.md")), "no historical build status files")
require(not list(root.glob("RELEASE_NOTES_v*.md")), "no historical release notes files")

workflow = (root / ".github" / "workflows" / "build.yml").read_text(encoding="utf-8")
require("runs-on: ubuntu-24.04" in workflow, "fixed Ubuntu runner")
require("pio run -e d1_mini" in workflow, "PlatformIO firmware build")
require("python scripts/make_ota.py" in workflow, "OTA package creation")
require("actions/upload-artifact@v4" in workflow, "firmware artifact upload")
require("esp8266-moisture-sensor-v0.1.3" in workflow, "versioned firmware artifact name")
require("ota-dist/firmware.bin" in workflow, "firmware.bin artifact")
require('tags:' in workflow and '"v*"' in workflow, "tag-triggered release workflow")
require("permissions:" in workflow and "contents: write" in workflow, "release write permission")
require("gh release create" in workflow, "GitHub Release creation")
require("gh release upload" in workflow, "GitHub Release asset upload")
require("ota-dist/firmware.bin.sha256" in workflow, "release SHA-256 asset")
require("ota-dist/manifest.json" in workflow, "release manifest asset")
project_text = "\n".join(
    p.read_text(encoding="utf-8", errors="ignore")
    for p in root.rglob("*")
    if p.is_file()
    and p.name != "validate_project.py"
    and p.suffix.lower() in {".h", ".cpp", ".md", ".py", ".yml", ".yaml", ".ini", ".txt"}
)
legacy_name = "esp" + "display"
require(legacy_name not in project_text.lower(), "no legacy display-project naming")
require("MoistureSensor-" in main, "moisture sensor AP SSID")
require("SoilSensor-1" in main, "default device name")
require("MS-Setup-8266" in main, "moisture sensor AP password")

failed = [name for name, ok in checks if not ok]
for name, ok in checks:
    print(("PASS" if ok else "FAIL") + " - " + name)
if failed:
    sys.exit(1)
