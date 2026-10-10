#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
MAIN = (ROOT / "src" / "main.cpp").read_text(encoding="utf-8")
UI = (ROOT / "src" / "web_ui.h").read_text(encoding="utf-8")
VERSION_H = (ROOT / "include" / "version.h").read_text(encoding="utf-8")
WORKFLOW = (ROOT / ".github" / "workflows" / "build.yml").read_text(encoding="utf-8")
OTA_SCRIPT = (ROOT / "scripts" / "make_ota.py").read_text(encoding="utf-8")

failures = []
count = 0

def check(name, condition):
    global count
    count += 1
    if condition:
        print(f"PASS - {name}")
    else:
        print(f"FAIL - {name}")
        failures.append(name)

def version_tuple(v):
    parts = v.lstrip("v").split(".")
    if len(parts) != 3 or not all(x.isdigit() for x in parts):
        raise ValueError(v)
    return tuple(int(x) for x in parts)

def is_newer(candidate, current):
    return version_tuple(candidate) > version_tuple(current)

def moisture_percent(adc, dry, wet):
    if dry == wet:
        raise ValueError("invalid calibration")
    value = (adc - dry) / (wet - dry) * 100.0
    return max(0.0, min(100.0, value))

# Version comparison
check("version newer patch", is_newer("0.1.30", "0.1.10"))
check("version equal not newer", not is_newer("v0.1.30", "0.1.30"))
check("version older not newer", not is_newer("0.1.9", "0.1.10"))
check("version minor comparison", is_newer("0.2.0", "0.1.99"))

# Soil-moisture conversion
check("moisture dry endpoint", moisture_percent(800, 800, 400) == 0.0)
check("moisture wet endpoint", moisture_percent(400, 800, 400) == 100.0)
check("moisture midpoint", moisture_percent(600, 800, 400) == 50.0)
check("moisture clamp below zero", moisture_percent(900, 800, 400) == 0.0)
check("moisture clamp above hundred", moisture_percent(300, 800, 400) == 100.0)
check("moisture inverse ADC direction", moisture_percent(600, 400, 800) == 50.0)

# Sensor/runtime invariants
check("A0 analog input", "analogRead(A0)" in MAIN)
check("calibration minimum span constant", "MIN_CALIBRATION_SPAN = 40" in MAIN)
check("calibration span enforced", ">= MIN_CALIBRATION_SPAN" in MAIN)
check("POSIX timezone configTime", "configTime(cfg.timezone, cfg.ntpServer)" in MAIN)
check("localtime conversion", "localtime_r(" in MAIN)

check("ADC disconnected threshold constant", "SENSOR_ADC_DISCONNECTED_MAX = 50" in MAIN)
check("ADC 50 treated disconnected", "raw > (float)SENSOR_ADC_DISCONNECTED_MAX" in MAIN)
check("ADC plausible upper bound", "SENSOR_ADC_PLAUSIBLE_MAX = 1000" in MAIN)
check("factory calibration disabled", "cfg.dryAdc = 0;" in MAIN and "cfg.wetAdc = 0;" in MAIN)
check("plausibility blocks moisture calculation", "!calibrated() || !sensorAdcPlausible(adc)" in MAIN)
check("health requires plausible sensor", "sensorState.valid && sensorAdcPlausible(sensorState.rawAdc) && calibrated()" in MAIN)
check("calibration rejects implausible sensor", "Sensor value is implausible / sensor not connected" in MAIN)
check("sensor status API", '"sensor_status"' in MAIN and 'sensor["status"]' in MAIN)
check("sensor disconnected UI", "sensorDisconnected" in UI and "sensorStateCode==='not_connected'" in UI)

check("manual calibration endpoint", '"/api/calibration/manual"' in MAIN)
check("manual calibration range validation", "Calibration values must be within the plausible ADC range" in MAIN)
check("manual calibration span validation", "Dry and wet calibration values are too close" in MAIN)
check("manual calibration persisted", "cfg.dryAdc = static_cast<uint16_t>(dry);" in MAIN and "cfg.wetAdc = static_cast<uint16_t>(wet);" in MAIN)
check("manual calibration UI", 'id="manualDry"' in UI and 'id="manualWet"' in UI)
check("manual calibration client validation", "Math.abs(dry-wet)<40" in UI)
check("manual calibration save action", "saveManualCalibration()" in UI and "'/api/calibration/manual'" in UI)

check("manual calibration uses canonical ADC constants", "SENSOR_ADC_DISCONNECTED_MAX" in MAIN and "SENSOR_ADC_PLAUSIBLE_MAX" in MAIN)
check("manual calibration lower bound matches disconnected threshold", "dry <= SENSOR_ADC_DISCONNECTED_MAX" in MAIN and "wet <= SENSOR_ADC_DISCONNECTED_MAX" in MAIN)
check("obsolete manual calibration constants absent", "SENSOR_ADC_MIN_PLAUSIBLE" not in MAIN and "SENSOR_ADC_MAX_PLAUSIBLE" not in MAIN and "SENSOR_ADC_PLAUSIBLE_MIN" not in MAIN)

check("manifest stays compact without release notes", '"release_notes": release_notes' not in OTA_SCRIPT)
check("OTA check excludes embedded release notes", 'doc["release_notes"] = manifest.releaseNotes' not in MAIN)
check("OTA UI loads release notes separately", "const releaseNotesReady=await loadOtaReadme(otaExpectedVersion)" in UI)
check("OTA release notes request is awaited", "const releaseNotesReady=await loadOtaReadme(otaExpectedVersion)" in UI)
check("manual OTA uses multipart FormData", "const form=new FormData()" in UI and "form.append('firmware',f,f.name)" in UI)
check("manual OTA has upload progress", "xhr.upload.onprogress" in UI)
check("manual OTA posts to local endpoint", "xhr.open('POST','/api/ota/upload',true)" in UI)
check("broken uploadBuf removed", "uploadBuf(" not in UI)
check("manual OTA backend diagnostics", "Manual OTA: upload started" in MAIN and "Manual OTA: firmware installed" in MAIN)

check("daily SNTP update interval override", "uint32_t sntp_update_delay_MS_rfc_not_less_than_15000()" in MAIN)
check("daily SNTP interval is 24 hours", "return 24UL * 60UL * 60UL * 1000UL;" in MAIN)
check("NTP configuration still uses configured timezone and server", "configTime(cfg.timezone, cfg.ntpServer);" in MAIN)

check("OTA network manifest timing logs", "OTA NET: manifest HTTP " in MAIN and "otaDiag.manifestMs" in MAIN)
check("OTA compact manifest diagnostics", "OTA NET: manifest payload " in MAIN and "OTA NET: compact manifest " in MAIN)
check("OTA firmware header timing logs", "OTA NET: firmware HTTP " in MAIN and "otaDiag.firmwareHeaderMs" in MAIN)
check("OTA firmware preflight diagnostics", "OTA NET: firmware preflight size " in MAIN and "free sketch " in MAIN)
check("OTA firmware progress throughput logs", "otaRateKbs(total, otaDiag.downloadMs)" in MAIN and "KiB/s" in MAIN)
check("OTA firmware stall diagnostics", "OTA NET: waiting for firmware data " in MAIN)
check("OTA firmware timeout diagnostics", "OTA NET: firmware timeout after " in MAIN)
check("OTA diagnostics persisted separately", "struct OtaDiagRecord" in MAIN and "OTA_DIAG_OFFSET = 640" in MAIN)
check("OTA diagnostics reported after reboot", "OTA diag: manifest HTTP " in MAIN and "OTA diag: download " in MAIN)
check("existing OTA record layout retained", 'char error[64] = {0};' in MAIN and "struct OtaDiagRecord" in MAIN)
check("manual OTA implementation retained", "Manual OTA: upload started" in MAIN and "form.append('firmware',f,f.name)" in UI)

check("NTP core callback header", "#include <coredecls.h>" in MAIN)
check("NTP synchronization callback registered", "settimeofday_cb(onNtpTimeSet);" in MAIN)
check("NTP callback remains lightweight", "static void onNtpTimeSet()" in MAIN and "ntpTimeUpdatePending = true;" in MAIN)
check("NTP synchronization log processed in loop", "ntpUpdateLogLoop();" in MAIN)
check("NTP synchronization log contains local date time", 'NTP: synchronized local=' in MAIN and "localTimestamp(now)" in MAIN)
check("NTP synchronization log contains server", '" | server=" + String(cfg.ntpServer)' in MAIN)
check("NTP synchronization log contains timezone", '" | TZ=" + String(cfg.timezone)' in MAIN)
check("NTP synchronization log contains RSSI", '" | RSSI=" + String(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0)' in MAIN)
check("daily NTP interval retained after logging change", "return 24UL * 60UL * 60UL * 1000UL;" in MAIN)

check("manifest manual redirect handling", "HTTPC_DISABLE_FOLLOW_REDIRECTS" in MAIN and "OTA NET: manifest redirect " in MAIN)
check("manifest redirect location header", 'const char* headerKeys[] = {"Location"};' in MAIN and "http.collectHeaders(headerKeys, 1);" in MAIN)
check("manifest fixed reader rejects empty HTTP 200 body", "manifest body reader returned 0 B" in MAIN or "Manifest payload empty" in MAIN)
check("manifest body length validation", "manifest body length mismatch HTTP=" in MAIN)
check("manifest three attempts", "MAX_MANIFEST_ATTEMPTS = 3" in MAIN)
check("manifest redirect limit", "MAX_REDIRECTS = 6" in MAIN)
check("firmware manual redirect handling", "MAX_FIRMWARE_REDIRECTS = 6" in MAIN and "OTA NET: firmware redirect " in MAIN)
check("redirect log includes location length", "locationLen=" in MAIN)
check("versioned firmware URL removes latest redirect", "releases/download/v{VERSION}/firmware.bin" in OTA_SCRIPT)
check("manual OTA remains unchanged", "form.append('firmware',f,f.name)" in UI and "Manual OTA: upload started" in MAIN)

check("manifest uses fixed-buffer body reader", "static bool otaReadManifestBody(" in MAIN and "otaReadManifestBody(" in MAIN)
check("manifest no longer uses getString", "payload = http.getString();" not in MAIN)
check("manifest waits for expected body despite disconnected socket", "while (contentLength <= 0 || bodyLen < (size_t)contentLength)" in MAIN)
check("manifest body idle timeout", "MANIFEST_BODY_IDLE_TIMEOUT_MS = 3000" in MAIN)
check("manifest retry reuses resolved URL", "String currentUrl = otaManifestUrlForVersion(targetVersion);" in MAIN and "retryHost=" in MAIN)
check("manifest requests identity encoding", 'http.addHeader("Accept-Encoding", "identity");' in MAIN)
check("firmware download not gated by http connected state", "while (total < manifest.size)" in MAIN and "while (http.connected() && total < manifest.size)" not in MAIN)
check("manual OTA still retained", "form.append('firmware',f,f.name)" in UI and "Manual OTA: upload started" in MAIN)
check("NTP sync logging still retained", "NTP: synchronized local=" in MAIN)

check("manifest fixed buffer avoids large String allocation", "MANIFEST_BUFFER_SIZE = 768" in MAIN and "char payload[MANIFEST_BUFFER_SIZE]" in MAIN)
check("manifest releaseNotes String removed", "String releaseNotes;" not in MAIN)
check("manifest asset excludes release notes", '"release_notes": release_notes' not in OTA_SCRIPT)
check("README remains separate release asset", '(OUT / "README.md").write_text(release_notes' in OTA_SCRIPT)
check("release notes loaded separately before install", "const releaseNotesReady=await loadOtaReadme(otaExpectedVersion)" in UI)
check("README diagnostics retained", "OTA README: HTTP " in MAIN and "OTA README: received " in MAIN)
check("manual OTA retained after manifest rollback", "form.append('firmware',f,f.name)" in UI and "Manual OTA: upload started" in MAIN)
check("NTP logging retained after manifest rollback", "NTP: synchronized local=" in MAIN)

check("OTA fast latest release check", "fetchLatestReleaseVersion" in MAIN and "/releases/latest" in MAIN)
check("OTA latest version parsed from redirect", "otaVersionFromLatestLocation" in MAIN and "/releases/tag/" in MAIN)
check("OTA latest version check logs timing", "OTA CHECK: HTTP " in MAIN and "OTA CHECK: latest version " in MAIN)
check("OTA README version is explicit", 'server.arg("version")' in MAIN and "/api/ota/readme?version=" in UI)
check("OTA README no cached manifest dependency", "No newer firmware selected" not in MAIN)
check("OTA README uses raw tagged file", "raw.githubusercontent.com" in MAIN and "RELEASE_NOTES.md" in MAIN)
check("OTA README detailed logging", "OTA README: browser requested version " in MAIN and "OTA README: HTTP " in MAIN)
check("OTA install version is explicit", "postForm('/api/ota/update',{version:otaExpectedVersion})" in UI)
check("OTA install fetches manifest on demand", "fetchOtaManifestForVersion(targetVersion, manifest, error)" in MAIN)
check("OTA install manifest target checked", "Manifest version mismatch" in MAIN)
check("System log copy button retained in v0.1.30", 'onclick="copyLog()"' in UI)
check("manual OTA retained in v0.1.30", "form.append('firmware',f,f.name)" in UI and "Manual OTA: upload started" in MAIN)
check("NTP logging retained in v0.1.30", "NTP: synchronized local=" in MAIN)

# OTA invariants
check("OTA check endpoint", '"/api/ota/check"' in MAIN and "fetchLatestReleaseVersion" in MAIN)
check("OTA update endpoint", '"/api/ota/update"' in MAIN)
check("OTA status endpoint", '"/api/ota/status"' in MAIN)
check("SHA-256 verification", "Firmware SHA-256 mismatch" in MAIN)
check("OTA progress milestones", 'nextLogPercent = 25' in MAIN and 'nextLogPercent += 25' in MAIN)
check("OTA System log progress", 'addLog("OTA: download " + String(nextLogPercent) + "%" + ' in MAIN or 'addLog("OTA: download " + String(nextLogPercent) + "%, "' in MAIN)
check("no unsupported Update.abort", "Update.abort()" not in MAIN)
check("no mixed-type min in OTA buffer sizing", "min(available, sizeof(buffer))" not in MAIN)

# Warning regressions
check("no memset on OtaRecord", "memset(&r, 0, sizeof(r))" not in MAIN)
check("rawOtaUpload removed", "rawOtaUpload()" not in MAIN)

# UI behavior
check("conditional install button", 'id="otaInstallBtn"' in UI)
check("install button initially hidden", "otaInstallBtn.style.display='none'" in UI)
ota_segment = UI[UI.find("async function checkOta"):UI.find("async function manualUpload")]
check("no OTA confirmation dialog", "confirm(" not in ota_segment)
check("OTA reboot watcher", "watchOtaRestart()" in UI)
check("settings dirty-state guard", "formDirty" in UI)
check("status-only normal polling", "activePage==='status'" in UI)
check("System log live refresh", "if(activePage==='log')loadLog()" in UI)

check("fixed header content clearance", ".content{margin-left:205px;padding:87px 32px 40px" in UI)
check("tablet header content clearance", ".content{margin-left:175px;padding:75px 18px 26px" in UI)

check("browser tab uses device name", "document.title=S.settings.device_name||\'ESP8266 Moisture Sensor\'" in UI)
check("browser title fallback present", "<title>SoilSensor</title>" in UI)

check("single authoritative device name", 'doc["sensor"] = cfg.deviceName;' in MAIN and 'settings["sensor_name"] = cfg.deviceName;' in MAIN)
check("legacy sensor name synchronized", "copyText(cfg.sensorName, sizeof(cfg.sensorName), device)" in MAIN)
check("hostname-safe server validation", "validDeviceName" in MAIN and "no spaces or umlauts" in MAIN)
check("sensor name input removed", 'id="sensorName"' not in UI)
check("hostname-safe client validation", "const devicePattern=/^[A-Za-z0-9]" in UI)
check("OTA README endpoint", '"/api/ota/readme"' in MAIN)
check("OTA README card", 'id="otaReadmeCard"' in UI)
check("OTA README fallback retained", "loadOtaReadme(otaExpectedVersion)" in UI)
check("OTA README asset contains release notes", 'release_notes = (ROOT / "RELEASE_NOTES.md").read_text' in OTA_SCRIPT)

check("OTA status keeps original hint markup", '<div class="hint" id="otaMsg"></div>' in UI)
check("OTA status text color override", "#otaMsg{color:var(--text)}" in UI)
check("no custom OTA status box style", ".otaStatus{" not in UI)

check("install button waits for release notes", "const releaseNotesReady=await loadOtaReadme(otaExpectedVersion)" in UI and "if(releaseNotesReady)" in UI)
check("install button hidden while release notes load", "otaInstallBtn.style.display='none'" in UI and "otaInstallBtn.disabled=true" in UI)
check("separate release notes unlock install button", "const releaseNotesReady=await loadOtaReadme(otaExpectedVersion)" in UI and "if(releaseNotesReady)" in UI)
check("release notes awaited before install", "const releaseNotesReady=await loadOtaReadme(otaExpectedVersion)" in UI)
check("release note loader reports success", "return true;" in UI and "return false;" in UI)
check("install button shown inside ready block", "if(releaseNotesReady)" in UI and "otaInstallBtn.style.display='inline-block'" in UI)

check("System log clipboard button", 'onclick="copyLog()"' in UI and 'data-i18n="copyLog"' in UI)
check("System log clipboard modern API", "navigator.clipboard" in UI and "navigator.clipboard.writeText(text)" in UI)
check("System log clipboard HTTP fallback", "document.execCommand('copy')" in UI and "document.createElement('textarea')" in UI)
check("System log clipboard German label", "copyLog:'In Zwischenablage kopieren'" in UI)
check("System log clipboard English label", "copyLog:'Copy to clipboard'" in UI)
check("System log clipboard result state", 'id="logCopyState"' in UI and "copiedLog" in UI and "copyLogFailed" in UI)

# Release pipeline
check("tag workflow", 'tags:' in WORKFLOW and '"v*"' in WORKFLOW)
check("release asset upload", "gh release upload" in WORKFLOW)
check("firmware release asset", "firmware.bin" in WORKFLOW)
check("manifest release asset", "manifest.json" in WORKFLOW)
check("versioned firmware release URL", "releases/download/v{VERSION}/firmware.bin" in OTA_SCRIPT)
check("internal tests in CI", "python tests/internal_tests.py" in WORKFLOW)

# Version consistency
m = re.search(r'#define APP_VERSION "([^"]+)"', VERSION_H)
check("firmware version 0.1.30", bool(m) and m.group(1) == "0.1.30")

if failures:
    print("\nInternal regression tests failed:")
    for item in failures:
        print(f" - {item}")
    sys.exit(1)

print(f"\nAll {count} internal regression tests passed.")
