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

require('#define APP_VERSION "0.1.15"' in version_h, "version header")
require("v0.1.15" in readme and "v0.1.15" in release, "release version consistency")
require('"/api/current-values"' in main, "current-values API")
require('"/api/health"' in main, "health API")
require('pin != "A0"' in main, "A0 pin validation")
require("MIN_CALIBRATION_SPAN" in main, "calibration span guard")
require("MS-Setup-8266" in main and "MS-Setup-8266" in readme, "fallback AP password documented")
require("esp8266-moisture-sensor--for-growtend-backend" in version_h, "OTA repository")
forbidden_display = "TM" + "1637"
require(forbidden_display not in main and forbidden_display not in ui, "no segment-display runtime/UI code")
require(not list(root.glob("BUILD_STATUS_v*.md")), "no historical build status files")
require(not list(root.glob("RELEASE_NOTES_v*.md")), "no historical release notes files")

workflow = (root / ".github" / "workflows" / "build.yml").read_text(encoding="utf-8")
require("runs-on: ubuntu-24.04" in workflow, "fixed Ubuntu runner")
require("pio run -e d1_mini" in workflow, "PlatformIO firmware build")
require("python scripts/make_ota.py" in workflow, "OTA package creation")
require("actions/upload-artifact@v4" in workflow, "firmware artifact upload")
require("esp8266-moisture-sensor-v0.1.15" in workflow, "versioned firmware artifact name")
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
runtime_ui = (main + "\n" + ui).lower()
require("temperature" not in runtime_ui, "no temperature runtime/UI concept")
require("api host" not in runtime_ui, "no external API host concept")
require("external api" not in runtime_ui, "no external API polling concept")
require("anzeige" not in ui.lower(), "no display page")
require("formDirty" in ui, "configuration dirty-state guard")
require("saveInProgress" in ui, "save-in-progress guard")
require("activePage==='status'" in ui, "Status-only polling guard")
require("setInterval(()=>{if(activePage==='status'" in ui, "safe periodic polling")

version_h_text = (root / "include" / "version.h").read_text(encoding="utf-8")
ota_script = (root / "scripts" / "make_ota.py").read_text(encoding="utf-8")
require("https://github.com/syschelle/esp8266-moisture-sensor--for-growtend-backend/releases/latest/download/manifest.json" in version_h_text, "latest-release OTA manifest URL")
require("releases/latest/download/firmware.bin" in ota_script, "latest-release firmware URL")
require("/ota/manifest.json" not in version_h_text, "no OTA branch manifest dependency")
require("raw.githubusercontent.com/{REPO}/ota/firmware.bin" not in ota_script, "no OTA branch firmware dependency")

require('"/api/ota/check"' in main, "local OTA check endpoint")
require('"/api/ota/update"' in main, "local OTA update endpoint")
require("WiFiClientSecureBearSSL.h" in main, "ESP-side secure OTA client")
require("HTTPC_STRICT_FOLLOW_REDIRECTS" in main, "OTA redirect following")
require("class Sha256Tiny" in main, "streamed SHA-256 implementation")
require("Firmware SHA-256 mismatch" in main, "firmware SHA-256 verification")
require("api('/api/ota/check')" in ui, "browser local OTA check")
require("api('/api/ota/update',{method:'POST'})" in ui, "browser local OTA update")
require("fetch(c.manifest_url" not in ui, "no browser GitHub manifest fetch")

require("configTime(cfg.timezone, cfg.ntpServer)" in main, "POSIX-TZ configTime overload")
require('setenv("TZ", cfg.timezone' in main, "timezone environment")
require("tzset();" in main, "timezone activation")
require("localtime_r(" in main, "local time conversion")
require("CET-1CEST,M3.5.0,M10.5.0/3" in main, "CET/CEST default timezone")
require("configTime(0, 0, cfg.ntpServer)" not in main, "no UTC-style configTime overload")

require('"/api/ota/status"' in main, "OTA status endpoint")
require("otaUpdateRequested" in main and "otaUpdateRunAt" in main, "scheduled OTA execution")
require("otaLoop();" in main, "OTA main-loop execution")
require("nextLogPercent = 25" in main and "OTA: download " in main and "nextLogPercent += 25" in main, "OTA progress logging")
require("reportPreviousOta();" in main, "post-reboot OTA report")
require("OTA_RECORD_OFFSET" in main, "persistent OTA result record")
require('id="otaInstallBtn"' in ui, "conditional OTA install button")
require("otaInstallBtn.style.display='none'" in ui, "OTA install button hidden by default")
require("watchOtaRestart()" in ui, "OTA reconnect watcher")
require("confirm(" not in ui[ui.find("async function checkOta"):ui.find("async function manualUpload")], "no OTA confirmation alert")
require("if(activePage==='log')loadLog()" in ui, "System log live refresh")

require("Update.abort()" not in main, "no unsupported Update.abort")
require("scheduleRestart(1200)" in main, "failed OTA restart cleanup")

require("memset(&r, 0, sizeof(r))" not in main, "no OtaRecord memset warning")
require("rawOtaUpload()" not in main, "no unused rawOtaUpload")
require((root / "tests" / "internal_tests.py").exists(), "internal regression tests present")
require("python tests/internal_tests.py" in workflow, "internal regression tests in CI")

require("min(available, sizeof(buffer))" not in main, "no mixed-type min in OTA buffer sizing")

require(".content{margin-left:205px;padding:87px 32px 40px" in ui, "fixed header content clearance")
require(".content{margin-left:175px;padding:75px 18px 26px" in ui, "tablet header content clearance")

require("document.title=S.settings.device_name||'ESP8266 Moisture Sensor'" in ui, "browser tab device name")
require("<title>SoilSensor</title>" in ui, "browser title fallback")

require('doc["sensor"] = cfg.deviceName;' in main, "API uses unified device name")
require('settings["sensor_name"] = cfg.deviceName;' in main, "sensor compatibility alias unified")
require("validDeviceName" in main, "hostname-safe device validation")
require('id="sensorName"' not in ui, "sensor name UI removed")
require('"/api/ota/readme"' in main, "OTA README endpoint")
require('id="otaReadmeCard"' in ui, "OTA README card")
require("await loadOtaReadme()" in ui, "OTA README load on update")
require('release_notes = (ROOT / "RELEASE_NOTES.md").read_text' in ota_script, "OTA README from release notes")

failed = [name for name, ok in checks if not ok]
for name, ok in checks:
    print(("PASS" if ok else "FAIL") + " - " + name)
if failed:
    sys.exit(1)
