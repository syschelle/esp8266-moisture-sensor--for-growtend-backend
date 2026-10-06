#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Updater.h>
#include <time.h>

#include "version.h"
#include "web_ui.h"

static constexpr uint32_t CFG_MAGIC = 0x4D4F4953UL; // "MOIS"
static constexpr uint16_t CFG_SCHEMA = 1;
static constexpr size_t EEPROM_SIZE = 1024;
static constexpr uint16_t ADC_MAX_VALUE = 1023;
static constexpr uint16_t MIN_CALIBRATION_SPAN = 40;
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
static constexpr uint32_t WIFI_RETRY_MS = 15000;
static constexpr uint32_t WIFI_AP_AFTER_DISCONNECT_MS = 30000;
static constexpr uint32_t WIFI_AP_DISABLE_STABLE_MS = 10000;
static constexpr uint32_t SAMPLE_SPACING_MS = 10;
static constexpr size_t LOG_CAPACITY = 60;

ESP8266WebServer server(80);

struct Config {
  uint32_t magic;
  uint16_t schema;
  char ssid[33];
  char wifiPassword[65];
  char deviceName[33];
  char sensorName[33];
  char signalPin[8];
  uint16_t dryAdc;
  uint16_t wetAdc;
  uint16_t measureIntervalSeconds;
  uint8_t sampleCount;
  char ntpServer[64];
  char timezone[64];
  char language[3];
  char theme[8];
  uint32_t crc;
};

Config cfg;

struct SensorState {
  bool valid = false;
  bool measuring = false;
  uint32_t sampleSum = 0;
  uint8_t samplesTaken = 0;
  uint32_t lastSampleMs = 0;
  uint32_t lastMeasurementMs = 0;
  uint16_t rawAdc = 0;
  float filteredAdc = NAN;
  float moisturePercent = NAN;
  time_t measurementEpoch = 0;
} sensorState;

String logLines[LOG_CAPACITY];
size_t logHead = 0;
size_t logCount = 0;

bool apActive = false;
uint32_t wifiDisconnectedSince = 0;
uint32_t wifiConnectedSince = 0;
uint32_t lastWifiAttempt = 0;
bool restartPending = false;
uint32_t restartAt = 0;

static void copyText(char* dst, size_t size, const String& src) {
  if (size == 0) return;
  strncpy(dst, src.c_str(), size - 1);
  dst[size - 1] = '\0';
}

static uint32_t crc32Bytes(const uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFFUL;
  while (len--) {
    crc ^= *data++;
    for (uint8_t k = 0; k < 8; k++) crc = (crc >> 1) ^ (0xEDB88320UL & (-(int32_t)(crc & 1)));
  }
  return ~crc;
}

static uint32_t configCrc(const Config& c) {
  return crc32Bytes(reinterpret_cast<const uint8_t*>(&c), offsetof(Config, crc));
}

static void defaults() {
  memset(&cfg, 0, sizeof(cfg));
  cfg.magic = CFG_MAGIC;
  cfg.schema = CFG_SCHEMA;
  copyText(cfg.deviceName, sizeof(cfg.deviceName), "SoilSensor-1");
  copyText(cfg.sensorName, sizeof(cfg.sensorName), "Topf 1");
  copyText(cfg.signalPin, sizeof(cfg.signalPin), "A0");
  cfg.dryAdc = 800;
  cfg.wetAdc = 400;
  cfg.measureIntervalSeconds = 5;
  cfg.sampleCount = 10;
  copyText(cfg.ntpServer, sizeof(cfg.ntpServer), "de.pool.ntp.org");
  copyText(cfg.timezone, sizeof(cfg.timezone), "CET-1CEST,M3.5.0,M10.5.0/3");
  copyText(cfg.language, sizeof(cfg.language), "de");
  copyText(cfg.theme, sizeof(cfg.theme), "light");
  cfg.crc = configCrc(cfg);
}

static bool loadConfig() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, cfg);
  const bool ok = cfg.magic == CFG_MAGIC && cfg.schema == CFG_SCHEMA && cfg.crc == configCrc(cfg);
  if (!ok) {
    defaults();
    EEPROM.put(0, cfg);
    EEPROM.commit();
  }
  return ok;
}

static bool saveConfig() {
  cfg.magic = CFG_MAGIC;
  cfg.schema = CFG_SCHEMA;
  cfg.crc = configCrc(cfg);
  EEPROM.put(0, cfg);
  return EEPROM.commit();
}

static String localTimestamp(time_t t) {
  if (t < 1700000000) return "";
  struct tm tmv;
  localtime_r(&t, &tmv);
  char out[32];
  strftime(out, sizeof(out), "%Y-%m-%d %H:%M:%S", &tmv);
  return String(out);
}

static bool timeValid() {
  return time(nullptr) > 1700000000;
}

static void addLog(const String& message) {
  String line;
  if (timeValid()) line = localTimestamp(time(nullptr)) + " ";
  else line = "[" + String(millis() / 1000) + "s] ";
  line += message;
  logLines[logHead] = line;
  logHead = (logHead + 1) % LOG_CAPACITY;
  if (logCount < LOG_CAPACITY) logCount++;
  Serial.println(line);
}

static String allLogs() {
  String out;
  const size_t start = (logHead + LOG_CAPACITY - logCount) % LOG_CAPACITY;
  for (size_t i = 0; i < logCount; ++i) {
    out += logLines[(start + i) % LOG_CAPACITY];
    out += '\n';
  }
  return out;
}

static bool calibrated() {
  return abs((int)cfg.dryAdc - (int)cfg.wetAdc) >= MIN_CALIBRATION_SPAN;
}

static float moistureFromAdc(float adc) {
  if (!calibrated()) return NAN;
  const float span = (float)cfg.wetAdc - (float)cfg.dryAdc;
  float pct = ((adc - (float)cfg.dryAdc) / span) * 100.0f;
  if (pct < 0.0f) pct = 0.0f;
  if (pct > 100.0f) pct = 100.0f;
  return pct;
}

static uint16_t readSensorRaw() {
  // ESP8266 has one ADC channel. The configurable signal-pin setting currently resolves to A0.
  return analogRead(A0);
}

static void sensorLoop() {
  const uint32_t now = millis();
  const uint32_t intervalMs = (uint32_t)cfg.measureIntervalSeconds * 1000UL;
  if (!sensorState.measuring) {
    if (sensorState.valid && (uint32_t)(now - sensorState.lastMeasurementMs) < intervalMs) return;
    sensorState.measuring = true;
    sensorState.sampleSum = 0;
    sensorState.samplesTaken = 0;
    sensorState.lastSampleMs = now - SAMPLE_SPACING_MS;
  }
  if ((uint32_t)(now - sensorState.lastSampleMs) < SAMPLE_SPACING_MS) return;
  sensorState.lastSampleMs = now;
  sensorState.sampleSum += readSensorRaw();
  sensorState.samplesTaken++;
  if (sensorState.samplesTaken < cfg.sampleCount) return;

  const float avg = (float)sensorState.sampleSum / (float)sensorState.samplesTaken;
  if (isnan(sensorState.filteredAdc)) sensorState.filteredAdc = avg;
  else sensorState.filteredAdc = sensorState.filteredAdc * 0.70f + avg * 0.30f;

  sensorState.rawAdc = (uint16_t)lroundf(sensorState.filteredAdc);
  sensorState.moisturePercent = moistureFromAdc(sensorState.filteredAdc);
  sensorState.valid = true;
  sensorState.measurementEpoch = timeValid() ? time(nullptr) : 0;
  sensorState.lastMeasurementMs = now;
  sensorState.measuring = false;
}

static String apSsid() {
  String suffix = String(ESP.getChipId(), HEX);
  suffix.toUpperCase();
  return "MoistureSensor-" + suffix;
}

static void startAp() {
  if (apActive) return;
  WiFi.mode(WIFI_AP_STA);
  const String ssid = apSsid();
  if (WiFi.softAP(ssid.c_str(), "MS-Setup-8266")) {
    apActive = true;
    addLog("Fallback AP enabled: " + ssid + " / " + WiFi.softAPIP().toString());
  } else addLog("Fallback AP start failed");
}

static void stopAp() {
  if (!apActive) return;
  WiFi.softAPdisconnect(true);
  apActive = false;
  WiFi.mode(WIFI_STA);
  addLog("Fallback AP disabled");
}

static void beginStation() {
  if (strlen(cfg.ssid) == 0) {
    startAp();
    return;
  }
  WiFi.mode(apActive ? WIFI_AP_STA : WIFI_STA);
  WiFi.hostname(cfg.deviceName);
  WiFi.begin(cfg.ssid, cfg.wifiPassword);
  lastWifiAttempt = millis();
  addLog("Connecting Wi-Fi SSID: " + String(cfg.ssid));
}

static void wifiLoop() {
  const uint32_t now = millis();
  if (WiFi.status() == WL_CONNECTED) {
    if (wifiConnectedSince == 0) {
      wifiConnectedSince = now;
      wifiDisconnectedSince = 0;
      addLog("Wi-Fi connected: " + WiFi.localIP().toString());
    }
    if (apActive && (uint32_t)(now - wifiConnectedSince) >= WIFI_AP_DISABLE_STABLE_MS) stopAp();
    return;
  }

  wifiConnectedSince = 0;
  if (wifiDisconnectedSince == 0) wifiDisconnectedSince = now;
  if (strlen(cfg.ssid) == 0 || (uint32_t)(now - wifiDisconnectedSince) >= WIFI_AP_AFTER_DISCONNECT_MS) startAp();
  if (strlen(cfg.ssid) && (uint32_t)(now - lastWifiAttempt) >= WIFI_RETRY_MS) {
    WiFi.disconnect();
    beginStation();
  }
}

static void configureTime() {
  setenv("TZ", cfg.timezone, 1);
  tzset();
  configTime(0, 0, cfg.ntpServer);
  addLog("NTP configured: " + String(cfg.ntpServer));
}

static void sendJson(JsonDocument& doc, int status = 200) {
  String out;
  serializeJson(doc, out);
  server.send(status, "application/json; charset=utf-8", out);
}

static void sendError(int status, const String& msg) {
  JsonDocument doc;
  doc["error"] = msg;
  sendJson(doc, status);
}

static bool validName(const String& v) {
  return v.length() >= 1 && v.length() <= 32;
}

static void apiCurrentValues() {
  JsonDocument doc;
  doc["device"] = cfg.deviceName;
  doc["sensor"] = cfg.sensorName;
  doc["signal_pin"] = cfg.signalPin;
  doc["firmware_version"] = APP_VERSION;
  doc["raw_adc"] = sensorState.valid ? sensorState.rawAdc : -1;
  if (sensorState.valid && !isnan(sensorState.moisturePercent)) doc["moisture_percent"] = roundf(sensorState.moisturePercent * 10.0f) / 10.0f;
  else doc["moisture_percent"] = nullptr;
  doc["calibrated"] = calibrated();
  doc["last_measurement_at"] = sensorState.measurementEpoch ? localTimestamp(sensorState.measurementEpoch) : "";
  doc["wifi_rssi"] = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
  doc["uptime_seconds"] = millis() / 1000UL;
  sendJson(doc);
}

static void apiHealth() {
  JsonDocument doc;
  const bool healthy = sensorState.valid && calibrated();
  doc["status"] = healthy ? "ok" : "degraded";
  doc["sensor_valid"] = sensorState.valid;
  doc["calibrated"] = calibrated();
  doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
  doc["time_valid"] = timeValid();
  doc["uptime_seconds"] = millis() / 1000UL;
  sendJson(doc, healthy ? 200 : 200);
}

static void apiState() {
  JsonDocument doc;
  doc["app"] = APP_NAME;
  doc["version"] = APP_VERSION;
  doc["uptime_seconds"] = millis() / 1000UL;
  doc["free_heap"] = ESP.getFreeHeap();

  JsonObject settings = doc["settings"].to<JsonObject>();
  settings["device_name"] = cfg.deviceName;
  settings["sensor_name"] = cfg.sensorName;
  settings["signal_pin"] = cfg.signalPin;
  settings["measure_interval_seconds"] = cfg.measureIntervalSeconds;
  settings["sample_count"] = cfg.sampleCount;
  settings["dry_adc"] = cfg.dryAdc;
  settings["wet_adc"] = cfg.wetAdc;
  settings["ssid"] = cfg.ssid;
  settings["ntp_server"] = cfg.ntpServer;
  settings["timezone"] = cfg.timezone;
  settings["language"] = cfg.language;
  settings["theme"] = cfg.theme;

  JsonObject sensor = doc["sensor"].to<JsonObject>();
  sensor["valid"] = sensorState.valid;
  sensor["calibrated"] = calibrated();
  sensor["raw_adc"] = sensorState.valid ? sensorState.rawAdc : -1;
  if (sensorState.valid && !isnan(sensorState.moisturePercent)) sensor["moisture_percent"] = roundf(sensorState.moisturePercent * 10.0f) / 10.0f;
  else sensor["moisture_percent"] = nullptr;
  sensor["last_measurement_local"] = sensorState.measurementEpoch ? localTimestamp(sensorState.measurementEpoch) : "";

  JsonObject wifi = doc["wifi"].to<JsonObject>();
  wifi["connected"] = WiFi.status() == WL_CONNECTED;
  wifi["rssi"] = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
  wifi["ip"] = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "";
  wifi["ap_active"] = apActive;
  wifi["ap_ip"] = apActive ? WiFi.softAPIP().toString() : "";
  wifi["mode"] = WiFi.status() == WL_CONNECTED ? "STA" : (apActive ? "AP" : "OFFLINE");

  JsonObject tim = doc["time"].to<JsonObject>();
  tim["valid"] = timeValid();
  tim["local"] = timeValid() ? localTimestamp(time(nullptr)) : "";

  sendJson(doc);
}

static void saveSensorSettings() {
  const String name = server.arg("sensor_name");
  const String pin = server.arg("signal_pin");
  const int interval = server.arg("interval").toInt();
  const int samples = server.arg("samples").toInt();

  if (!validName(name)) return sendError(400, "Invalid sensor name");
  if (pin != "A0") return sendError(400, "ESP8266 supports only A0 as analog signal pin");
  if (interval < 1 || interval > 300) return sendError(400, "Interval must be 1..300 seconds");
  if (samples < 1 || samples > 50) return sendError(400, "Sample count must be 1..50");

  copyText(cfg.sensorName, sizeof(cfg.sensorName), name);
  copyText(cfg.signalPin, sizeof(cfg.signalPin), pin);
  cfg.measureIntervalSeconds = interval;
  cfg.sampleCount = samples;
  saveConfig();
  sensorState.valid = false;
  sensorState.measuring = false;
  addLog("Sensor settings saved");
  JsonDocument doc; doc["ok"] = true; sendJson(doc);
}

static void saveSystemSettings() {
  const String device = server.arg("device_name");
  const String ssid = server.arg("ssid");
  const String pass = server.arg("wifi_password");
  const String ntp = server.arg("ntp_server");
  const String tz = server.arg("timezone");
  const String language = server.arg("language");
  const String theme = server.arg("theme");

  if (!validName(device)) return sendError(400, "Invalid device name");
  if (ssid.length() > 32 || pass.length() > 64) return sendError(400, "Invalid Wi-Fi field length");
  if (ntp.length() < 1 || ntp.length() > 63 || tz.length() < 1 || tz.length() > 63) return sendError(400, "Invalid NTP/timezone");
  if (language != "de" && language != "en") return sendError(400, "Invalid language");
  if (theme != "light" && theme != "dark") return sendError(400, "Invalid theme");

  const bool wifiChanged = ssid != cfg.ssid || pass.length() > 0;
  copyText(cfg.deviceName, sizeof(cfg.deviceName), device);
  copyText(cfg.ssid, sizeof(cfg.ssid), ssid);
  if (pass.length() > 0) copyText(cfg.wifiPassword, sizeof(cfg.wifiPassword), pass);
  copyText(cfg.ntpServer, sizeof(cfg.ntpServer), ntp);
  copyText(cfg.timezone, sizeof(cfg.timezone), tz);
  copyText(cfg.language, sizeof(cfg.language), language);
  copyText(cfg.theme, sizeof(cfg.theme), theme);
  saveConfig();
  configureTime();
  addLog("System settings saved");
  JsonDocument doc; doc["ok"] = true; doc["wifi_changed"] = wifiChanged; sendJson(doc);
}

static void calibratePoint(bool dry) {
  if (!sensorState.valid) return sendError(409, "No sensor measurement available");
  if (dry) cfg.dryAdc = sensorState.rawAdc;
  else cfg.wetAdc = sensorState.rawAdc;
  saveConfig();
  sensorState.moisturePercent = moistureFromAdc(sensorState.filteredAdc);
  addLog(String("Calibration ") + (dry ? "dry" : "wet") + " = " + sensorState.rawAdc);
  JsonDocument doc; doc["ok"] = true; doc["dry_adc"] = cfg.dryAdc; doc["wet_adc"] = cfg.wetAdc; doc["calibrated"] = calibrated(); sendJson(doc);
}

static void scheduleRestart(uint32_t delayMs = 800) {
  restartPending = true;
  restartAt = millis() + delayMs;
}

static void otaUploadHandler() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    addLog("OTA upload started");
    const size_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (!Update.begin(maxSketchSpace)) Update.printError(Serial);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) Update.printError(Serial);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) addLog("OTA upload finished: " + String(upload.totalSize) + " bytes");
    else Update.printError(Serial);
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.end();
    addLog("OTA upload aborted");
  }
  yield();
}

static void otaUploadFinished() {
  JsonDocument doc;
  if (Update.hasError()) {
    doc["ok"] = false;
    doc["error"] = "OTA update failed";
    sendJson(doc, 500);
  } else {
    doc["ok"] = true;
    sendJson(doc);
    scheduleRestart(1000);
  }
}

static void rawOtaUpload() {
  // This handler is used when the browser sends application/octet-stream.
  if (!server.hasArg("plain")) {
    sendError(400, "No firmware body");
    return;
  }
  sendError(415, "Use multipart upload");
}

static void setupRoutes() {
  server.on("/", HTTP_GET, [](){ server.send_P(200, "text/html; charset=utf-8", WEB_UI); });
  server.on("/api/current-values", HTTP_GET, apiCurrentValues);
  server.on("/api/health", HTTP_GET, apiHealth);
  server.on("/api/state", HTTP_GET, apiState);
  server.on("/api/log", HTTP_GET, [](){ server.send(200, "text/plain; charset=utf-8", allLogs()); });
  server.on("/api/settings/sensor", HTTP_POST, saveSensorSettings);
  server.on("/api/settings/system", HTTP_POST, saveSystemSettings);
  server.on("/api/calibration/dry", HTTP_POST, [](){ calibratePoint(true); });
  server.on("/api/calibration/wet", HTTP_POST, [](){ calibratePoint(false); });
  server.on("/api/ota/config", HTTP_GET, [](){
    JsonDocument doc;
    doc["manifest_url"] = OTA_MANIFEST_URL;
    doc["current_version"] = APP_VERSION;
    doc["repository"] = APP_REPOSITORY;
    sendJson(doc);
  });

  server.on("/api/ota/upload", HTTP_POST, otaUploadFinished, otaUploadHandler);

  server.on("/api/reboot", HTTP_POST, [](){
    JsonDocument doc; doc["ok"] = true; sendJson(doc); addLog("Reboot requested"); scheduleRestart();
  });
  server.on("/api/factory-reset", HTTP_POST, [](){
    defaults();
    saveConfig();
    JsonDocument doc; doc["ok"] = true; sendJson(doc);
    addLog("Factory reset requested");
    scheduleRestart();
  });

  server.onNotFound([](){
    if (server.method() == HTTP_GET) server.sendHeader("Location", "/", true), server.send(302, "text/plain", "");
    else sendError(404, "Not found");
  });
}

void setup() {
  Serial.begin(115200);
  delay(20);
  Serial.println();
  const bool configWasValid = loadConfig();
  addLog(String(APP_NAME) + " " + APP_VERSION + " boot");
  addLog(String("Reset reason: ") + ESP.getResetReason());
  if (!configWasValid) addLog("Default settings initialized");

  pinMode(A0, INPUT);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);

  beginStation();
  const uint32_t waitStart = millis();
  while (strlen(cfg.ssid) && WiFi.status() != WL_CONNECTED && (uint32_t)(millis() - waitStart) < WIFI_CONNECT_TIMEOUT_MS) {
    delay(50);
    yield();
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnectedSince = millis();
    addLog("Wi-Fi connected: " + WiFi.localIP().toString());
  } else {
    startAp();
  }

  configureTime();
  setupRoutes();
  server.begin();
  addLog("HTTP server started");
}

void loop() {
  server.handleClient();
  wifiLoop();
  sensorLoop();

  if (restartPending && (int32_t)(millis() - restartAt) >= 0) {
    delay(50);
    ESP.restart();
  }
  yield();
}
