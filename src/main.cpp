#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
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
static constexpr uint16_t SENSOR_ADC_DISCONNECTED_MAX = 50;
static constexpr uint16_t SENSOR_ADC_PLAUSIBLE_MAX = 1000;
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
static constexpr uint32_t WIFI_RETRY_MS = 15000;
static constexpr uint32_t WIFI_AP_AFTER_DISCONNECT_MS = 30000;
static constexpr uint32_t WIFI_AP_DISABLE_STABLE_MS = 10000;
static constexpr uint32_t SAMPLE_SPACING_MS = 10;
static constexpr size_t LOG_CAPACITY = 60;
static constexpr size_t OTA_RECORD_OFFSET = 512;
static constexpr uint32_t OTA_RECORD_MAGIC = 0x4F544131UL; // OTA1

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

struct OtaRecord {
  uint32_t magic = OTA_RECORD_MAGIC;
  char fromVersion[16] = {0};
  char targetVersion[16] = {0};
  uint8_t phase = 0;
  char error[64] = {0};
  uint32_t crc = 0;
};


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
bool otaUpdateRequested = false;
uint32_t otaUpdateRunAt = 0;
String otaJobState = "idle";
String otaJobMessage;
String otaJobTargetVersion;

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

static uint32_t otaRecordCrc(const OtaRecord& r) {
  return crc32Bytes(reinterpret_cast<const uint8_t*>(&r), offsetof(OtaRecord, crc));
}

static void saveOtaRecord(uint8_t phase, const String& targetVersion, const String& error = "") {
  OtaRecord r;
  r.magic = OTA_RECORD_MAGIC;
  copyText(r.fromVersion, sizeof(r.fromVersion), APP_VERSION);
  copyText(r.targetVersion, sizeof(r.targetVersion), targetVersion);
  r.phase = phase;
  copyText(r.error, sizeof(r.error), error);
  r.crc = otaRecordCrc(r);
  EEPROM.put(OTA_RECORD_OFFSET, r);
  EEPROM.commit();
}

static bool loadOtaRecord(OtaRecord& r) {
  EEPROM.get(OTA_RECORD_OFFSET, r);
  return r.magic == OTA_RECORD_MAGIC && r.crc == otaRecordCrc(r);
}

static void clearOtaRecord() {
  OtaRecord r{};
  r.magic = 0;
  r.phase = 0;
  r.fromVersion[0] = '\0';
  r.targetVersion[0] = '\0';
  r.error[0] = '\0';
  r.crc = 0;
  EEPROM.put(OTA_RECORD_OFFSET, r);
  EEPROM.commit();
}

static void defaults() {
  memset(&cfg, 0, sizeof(cfg));
  cfg.magic = CFG_MAGIC;
  cfg.schema = CFG_SCHEMA;
  copyText(cfg.deviceName, sizeof(cfg.deviceName), "SoilSensor-1");
  copyText(cfg.sensorName, sizeof(cfg.sensorName), "SoilSensor-1"); // legacy compatibility field
  copyText(cfg.signalPin, sizeof(cfg.signalPin), "A0");
  cfg.dryAdc = 0;
  cfg.wetAdc = 0;
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
  } else if (String(cfg.sensorName) != String(cfg.deviceName)) {
    // v0.1.16: deviceName is the single authoritative identity.
    copyText(cfg.sensorName, sizeof(cfg.sensorName), cfg.deviceName);
    cfg.crc = configCrc(cfg);
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

static void reportPreviousOta() {
  OtaRecord r;
  if (!loadOtaRecord(r)) return;
  String from = String(r.fromVersion), target = String(r.targetVersion);
  if (r.phase == 4 && target == APP_VERSION) {
    addLog("OTA previous update: " + from + " -> " + target);
    addLog("OTA previous update: firmware downloaded");
    addLog("OTA previous update: SHA-256 verified");
    addLog("OTA previous update: installation successful");
  } else if (r.phase == 5) {
    addLog("OTA previous update failed: " + String(r.error));
  } else if (r.phase > 0) {
    addLog("OTA previous update was interrupted at phase " + String(r.phase));
  }
  clearOtaRecord();
}

static bool calibrated() {
  return abs((int)cfg.dryAdc - (int)cfg.wetAdc) >= MIN_CALIBRATION_SPAN;
}

static bool sensorAdcPlausible(float raw) {
  return raw > (float)SENSOR_ADC_DISCONNECTED_MAX && raw <= (float)SENSOR_ADC_PLAUSIBLE_MAX;
}

static float moistureFromAdc(float adc) {
  if (!calibrated() || !sensorAdcPlausible(adc)) return NAN;
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
  // ESP8266: use the POSIX-TZ configTime overload directly.
  // This applies CET/CEST including automatic daylight-saving changes.
  configTime(cfg.timezone, cfg.ntpServer);
  setenv("TZ", cfg.timezone, 1);
  tzset();
  addLog("NTP configured: " + String(cfg.ntpServer) + " TZ=" + String(cfg.timezone));
}


static void sendJson(JsonDocument& doc, int status = 200);
static void sendError(int status, const String& msg);
static void scheduleRestart(uint32_t delayMs = 800);

class Sha256Tiny {
public:
  Sha256Tiny() { reset(); }

  void reset() {
    totalLen = 0;
    bufferLen = 0;
    h[0]=0x6a09e667UL; h[1]=0xbb67ae85UL; h[2]=0x3c6ef372UL; h[3]=0xa54ff53aUL;
    h[4]=0x510e527fUL; h[5]=0x9b05688cUL; h[6]=0x1f83d9abUL; h[7]=0x5be0cd19UL;
  }

  void update(const uint8_t* data, size_t len) {
    totalLen += len;
    while (len) {
      size_t take = min((size_t)64 - bufferLen, len);
      memcpy(buffer + bufferLen, data, take);
      bufferLen += take;
      data += take;
      len -= take;
      if (bufferLen == 64) {
        transform(buffer);
        bufferLen = 0;
      }
    }
  }

  String finalHex() {
    const uint64_t bitLen = totalLen * 8ULL;
    buffer[bufferLen++] = 0x80;
    if (bufferLen > 56) {
      while (bufferLen < 64) buffer[bufferLen++] = 0;
      transform(buffer);
      bufferLen = 0;
    }
    while (bufferLen < 56) buffer[bufferLen++] = 0;
    for (int i = 7; i >= 0; --i) buffer[bufferLen++] = (uint8_t)(bitLen >> (i * 8));
    transform(buffer);

    char out[65];
    for (int i = 0; i < 8; ++i) snprintf(out + i * 8, 9, "%08lx", (unsigned long)h[i]);
    out[64] = '\0';
    return String(out);
  }

private:
  uint32_t h[8];
  uint8_t buffer[64];
  size_t bufferLen = 0;
  uint64_t totalLen = 0;

  static uint32_t rotr(uint32_t x, uint8_t n) { return (x >> n) | (x << (32 - n)); }

  void transform(const uint8_t block[64]) {
    static const uint32_t k[64] = {
      0x428a2f98UL,0x71374491UL,0xb5c0fbcfUL,0xe9b5dba5UL,0x3956c25bUL,0x59f111f1UL,0x923f82a4UL,0xab1c5ed5UL,
      0xd807aa98UL,0x12835b01UL,0x243185beUL,0x550c7dc3UL,0x72be5d74UL,0x80deb1feUL,0x9bdc06a7UL,0xc19bf174UL,
      0xe49b69c1UL,0xefbe4786UL,0x0fc19dc6UL,0x240ca1ccUL,0x2de92c6fUL,0x4a7484aaUL,0x5cb0a9dcUL,0x76f988daUL,
      0x983e5152UL,0xa831c66dUL,0xb00327c8UL,0xbf597fc7UL,0xc6e00bf3UL,0xd5a79147UL,0x06ca6351UL,0x14292967UL,
      0x27b70a85UL,0x2e1b2138UL,0x4d2c6dfcUL,0x53380d13UL,0x650a7354UL,0x766a0abbUL,0x81c2c92eUL,0x92722c85UL,
      0xa2bfe8a1UL,0xa81a664bUL,0xc24b8b70UL,0xc76c51a3UL,0xd192e819UL,0xd6990624UL,0xf40e3585UL,0x106aa070UL,
      0x19a4c116UL,0x1e376c08UL,0x2748774cUL,0x34b0bcb5UL,0x391c0cb3UL,0x4ed8aa4aUL,0x5b9cca4fUL,0x682e6ff3UL,
      0x748f82eeUL,0x78a5636fUL,0x84c87814UL,0x8cc70208UL,0x90befffaUL,0xa4506cebUL,0xbef9a3f7UL,0xc67178f2UL
    };
    uint32_t w[64];
    for (int i=0;i<16;++i) {
      w[i] = ((uint32_t)block[i*4] << 24) | ((uint32_t)block[i*4+1] << 16) | ((uint32_t)block[i*4+2] << 8) | block[i*4+3];
    }
    for (int i=16;i<64;++i) {
      uint32_t s0 = rotr(w[i-15],7) ^ rotr(w[i-15],18) ^ (w[i-15] >> 3);
      uint32_t s1 = rotr(w[i-2],17) ^ rotr(w[i-2],19) ^ (w[i-2] >> 10);
      w[i] = w[i-16] + s0 + w[i-7] + s1;
    }

    uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7];
    for (int i=0;i<64;++i) {
      uint32_t S1 = rotr(e,6) ^ rotr(e,11) ^ rotr(e,25);
      uint32_t ch = (e & f) ^ ((~e) & g);
      uint32_t t1 = hh + S1 + ch + k[i] + w[i];
      uint32_t S0 = rotr(a,2) ^ rotr(a,13) ^ rotr(a,22);
      uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
      uint32_t t2 = S0 + maj;
      hh=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
  }
};

struct OtaManifest {
  String version;
  String url;
  String sha256;
  size_t size = 0;
};

OtaManifest cachedOtaManifest;
bool cachedOtaManifestValid = false;

static bool parseVersionPart(const String& v, int part, int& out) {
  String s = v;
  if (s.startsWith("v")) s.remove(0, 1);
  int start = 0;
  for (int i = 0; i <= part; ++i) {
    int dot = s.indexOf('.', start);
    String token = dot >= 0 ? s.substring(start, dot) : s.substring(start);
    if (!token.length()) return false;
    for (size_t j=0;j<token.length();++j) if (!isDigit(token[j])) return false;
    if (i == part) { out = token.toInt(); return true; }
    if (dot < 0) return false;
    start = dot + 1;
  }
  return false;
}

static bool isNewerVersion(const String& candidate, const String& current) {
  for (int i=0;i<3;++i) {
    int a=0,b=0;
    if (!parseVersionPart(candidate, i, a) || !parseVersionPart(current, i, b)) return false;
    if (a > b) return true;
    if (a < b) return false;
  }
  return false;
}

static bool fetchOtaManifest(OtaManifest& out, String& error) {
  addLog("OTA: checking latest release manifest");
  if (WiFi.status() != WL_CONNECTED) {
    error = "Wi-Fi not connected";
    return false;
  }

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(15000);

  HTTPClient http;
  http.setTimeout(15000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  if (!http.begin(client, OTA_MANIFEST_URL)) {
    error = "Manifest connection failed";
    return false;
  }

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    error = "Manifest HTTP " + String(code);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError jsonError = deserializeJson(doc, payload);
  if (jsonError) {
    error = "Manifest JSON invalid";
    return false;
  }

  out.version = String(doc["version"] | "");
  out.url = String(doc["url"] | "");
  out.sha256 = String(doc["sha256"] | "");
  out.size = doc["size"] | 0;
  out.sha256.toLowerCase();

  if (!out.version.length() || !out.url.startsWith("https://") || out.sha256.length() != 64 || out.size < 1024) {
    error = "Manifest fields invalid";
    addLog("OTA: manifest invalid");
    return false;
  }
  addLog("OTA: manifest OK, latest version " + out.version);
  return true;
}

static bool downloadAndFlashOta(const OtaManifest& manifest, String& error) {
  if (WiFi.status() != WL_CONNECTED) { error = "Wi-Fi not connected"; return false; }
  size_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  if (manifest.size > maxSketchSpace) { error = "Firmware too large"; return false; }
  otaJobState = "downloading"; otaJobMessage = "Downloading firmware";
  addLog("OTA: firmware download started (" + String(manifest.size) + " bytes)");
  saveOtaRecord(1, manifest.version);
  BearSSL::WiFiClientSecure client; client.setInsecure(); client.setTimeout(20000);
  HTTPClient http; http.setTimeout(20000); http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!http.begin(client, manifest.url)) { error = "Firmware connection failed"; saveOtaRecord(5, manifest.version, error); return false; }
  int code = http.GET();
  if (code != HTTP_CODE_OK) { error = "Firmware HTTP " + String(code); http.end(); saveOtaRecord(5, manifest.version, error); return false; }
  int contentLength = http.getSize();
  if (contentLength > 0 && (size_t)contentLength != manifest.size) { error = "Firmware size mismatch"; http.end(); saveOtaRecord(5, manifest.version, error); return false; }
  if (!Update.begin(manifest.size)) { error = "Update begin failed"; http.end(); saveOtaRecord(5, manifest.version, error); return false; }
  WiFiClient* stream = http.getStreamPtr(); Sha256Tiny sha; uint8_t buffer[1024]; size_t total = 0; uint32_t lastDataMs = millis(); uint8_t nextLogPercent = 25;
  while (http.connected() && total < manifest.size) {
    size_t available = stream->available();
    if (available) {
      size_t want = (available < sizeof(buffer)) ? available : sizeof(buffer); size_t got = stream->readBytes(buffer, want);
      if (got) {
        lastDataMs = millis(); sha.update(buffer, got);
        if (Update.write(buffer, got) != got) { error = "Firmware flash write failed"; scheduleRestart(1200); http.end(); saveOtaRecord(5, manifest.version, error); return false; }
        total += got; uint8_t pct = (uint8_t)((total * 100UL) / manifest.size);
        if (pct >= nextLogPercent && nextLogPercent <= 100) { addLog("OTA: download " + String(nextLogPercent) + "%"); nextLogPercent += 25; }
      }
    } else {
      if ((uint32_t)(millis() - lastDataMs) > 20000UL) { error = "Firmware download timeout"; scheduleRestart(1200); http.end(); saveOtaRecord(5, manifest.version, error); return false; }
      delay(1); yield();
    }
  }
  http.end();
  if (total != manifest.size) { error = "Firmware incomplete"; scheduleRestart(1200); saveOtaRecord(5, manifest.version, error); return false; }
  addLog("OTA: firmware download complete"); saveOtaRecord(2, manifest.version);
  otaJobState = "verifying"; otaJobMessage = "Verifying SHA-256"; addLog("OTA: verifying SHA-256");
  String actualSha = sha.finalHex();
  if (!actualSha.equalsIgnoreCase(manifest.sha256)) { error = "Firmware SHA-256 mismatch"; scheduleRestart(1200); addLog("OTA: SHA-256 verification failed"); saveOtaRecord(5, manifest.version, error); return false; }
  addLog("OTA: SHA-256 verified"); saveOtaRecord(3, manifest.version);
  otaJobState = "installing"; otaJobMessage = "Finalizing firmware"; addLog("OTA: finalizing firmware installation");
  if (!Update.end(true)) { error = "Firmware finalize failed"; saveOtaRecord(5, manifest.version, error); return false; }
  addLog("OTA: firmware installation successful"); saveOtaRecord(4, manifest.version); return true;
}


static bool fetchOtaReadme(String& content, String& error) {
  if (WiFi.status() != WL_CONNECTED) {
    error = "Wi-Fi not connected";
    return false;
  }

  addLog("OTA: loading release README");

  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(15000);

  HTTPClient http;
  http.setTimeout(15000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  if (!http.begin(client, OTA_README_URL)) {
    error = "README connection failed";
    return false;
  }

  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    error = "README HTTP " + String(code);
    http.end();
    return false;
  }

  const int contentLength = http.getSize();
  if (contentLength > 16384) {
    error = "README too large";
    http.end();
    return false;
  }

  content = http.getString();
  http.end();

  if (!content.length()) {
    error = "README empty";
    return false;
  }

  addLog("OTA: release README loaded");
  return true;
}

static void apiOtaReadme() {
  if (!cachedOtaManifestValid || !isNewerVersion(cachedOtaManifest.version, APP_VERSION)) {
    sendError(409, "No newer firmware selected");
    return;
  }

  String content;
  String error;
  if (!fetchOtaReadme(content, error)) {
    addLog("OTA: README load failed: " + error);
    sendError(502, error);
    return;
  }

  server.send(200, "text/plain; charset=utf-8", content);
}

static void apiOtaCheck() {
  OtaManifest manifest; String error;
  if (!fetchOtaManifest(manifest, error)) { addLog("OTA: update check failed: " + error); cachedOtaManifestValid = false; sendError(502, error); return; }
  cachedOtaManifest = manifest; cachedOtaManifestValid = true;
  bool available = isNewerVersion(manifest.version, APP_VERSION);
  addLog(available ? ("OTA: update available " + manifest.version) : "OTA: firmware is up to date");
  JsonDocument doc; doc["current_version"] = APP_VERSION; doc["available_version"] = manifest.version; doc["update_available"] = available; doc["size"] = manifest.size; sendJson(doc);
}

static void apiOtaUpdate() {
  if (otaUpdateRequested || otaJobState == "downloading" || otaJobState == "verifying" || otaJobState == "installing") { sendError(409, "OTA update already running"); return; }
  if (!cachedOtaManifestValid || !isNewerVersion(cachedOtaManifest.version, APP_VERSION)) { sendError(409, "Run update check first"); return; }
  otaJobTargetVersion = cachedOtaManifest.version; otaJobState = "scheduled"; otaJobMessage = "Update scheduled";
  otaUpdateRequested = true; otaUpdateRunAt = millis() + 750;
  addLog("OTA: update scheduled for " + otaJobTargetVersion);
  JsonDocument doc; doc["ok"] = true; doc["started"] = true; doc["target_version"] = otaJobTargetVersion; sendJson(doc);
}

static void apiOtaStatus() {
  JsonDocument doc; doc["state"] = otaJobState; doc["message"] = otaJobMessage; doc["target_version"] = otaJobTargetVersion;
  doc["running"] = otaUpdateRequested || otaJobState == "downloading" || otaJobState == "verifying" || otaJobState == "installing"; sendJson(doc);
}

static void otaLoop() {
  if (!otaUpdateRequested || (int32_t)(millis() - otaUpdateRunAt) < 0) return;
  otaUpdateRequested = false; OtaManifest manifest = cachedOtaManifest; cachedOtaManifestValid = false; String error;
  addLog("OTA: update process started");
  if (!downloadAndFlashOta(manifest, error)) { otaJobState = "failed"; otaJobMessage = error; addLog("OTA: update failed: " + error); return; }
  otaJobState = "rebooting"; otaJobMessage = "Update installed, rebooting"; addLog("OTA: reboot scheduled"); scheduleRestart(1500);
}

static void sendJson(JsonDocument& doc, int status) {
  String out;
  serializeJson(doc, out);
  server.send(status, "application/json; charset=utf-8", out);
}

static void sendError(int status, const String& msg) {
  JsonDocument doc;
  doc["error"] = msg;
  sendJson(doc, status);
}

static bool validDeviceName(const String& v) {
  if (v.length() < 1 || v.length() > 32) return false;
  if (v[0] == '-' || v[v.length() - 1] == '-') return false;
  for (size_t i = 0; i < v.length(); ++i) {
    const char c = v[i];
    const bool allowed = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '-';
    if (!allowed) return false;
  }
  return true;
}

static void apiCurrentValues() {
  JsonDocument doc;
  doc["device"] = cfg.deviceName;
  doc["sensor"] = cfg.deviceName;
  doc["signal_pin"] = cfg.signalPin;
  doc["firmware_version"] = APP_VERSION;
  doc["raw_adc"] = sensorState.valid ? sensorState.rawAdc : -1;
  if (sensorState.valid && !isnan(sensorState.moisturePercent)) doc["moisture_percent"] = roundf(sensorState.moisturePercent * 10.0f) / 10.0f;
  else doc["moisture_percent"] = nullptr;
  doc["calibrated"] = calibrated();
  doc["sensor_plausible"] = sensorState.valid && sensorAdcPlausible(sensorState.rawAdc);
  doc["sensor_status"] = !sensorState.valid ? "no_measurement" : (!sensorAdcPlausible(sensorState.rawAdc) ? "not_connected" : (!calibrated() ? "not_calibrated" : "ok"));
  doc["last_measurement_at"] = sensorState.measurementEpoch ? localTimestamp(sensorState.measurementEpoch) : "";
  doc["wifi_rssi"] = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
  doc["uptime_seconds"] = millis() / 1000UL;
  sendJson(doc);
}

static void apiHealth() {
  JsonDocument doc;
  const bool healthy = sensorState.valid && sensorAdcPlausible(sensorState.rawAdc) && calibrated();
  doc["status"] = healthy ? "ok" : "degraded";
  doc["sensor_valid"] = sensorState.valid;
  doc["calibrated"] = calibrated();
  doc["sensor_plausible"] = sensorState.valid && sensorAdcPlausible(sensorState.rawAdc);
  doc["sensor_status"] = !sensorState.valid ? "no_measurement" : (!sensorAdcPlausible(sensorState.rawAdc) ? "not_connected" : (!calibrated() ? "not_calibrated" : "ok"));
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
  settings["sensor_name"] = cfg.deviceName; // compatibility alias
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
  sensor["plausible"] = sensorState.valid && sensorAdcPlausible(sensorState.rawAdc);
  sensor["status"] = !sensorState.valid ? "no_measurement" : (!sensorAdcPlausible(sensorState.rawAdc) ? "not_connected" : (!calibrated() ? "not_calibrated" : "ok"));
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
  const String pin = server.arg("signal_pin");
  const int interval = server.arg("interval").toInt();
  const int samples = server.arg("samples").toInt();

  if (pin != "A0") return sendError(400, "ESP8266 supports only A0 as analog signal pin");
  if (interval < 1 || interval > 300) return sendError(400, "Interval must be 1..300 seconds");
  if (samples < 1 || samples > 50) return sendError(400, "Sample count must be 1..50");

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

  if (!validDeviceName(device)) return sendError(400, "Device name: only A-Z, a-z, 0-9 and hyphen; no spaces or umlauts");
  if (ssid.length() > 32 || pass.length() > 64) return sendError(400, "Invalid Wi-Fi field length");
  if (ntp.length() < 1 || ntp.length() > 63 || tz.length() < 1 || tz.length() > 63) return sendError(400, "Invalid NTP/timezone");
  if (language != "de" && language != "en") return sendError(400, "Invalid language");
  if (theme != "light" && theme != "dark") return sendError(400, "Invalid theme");

  const bool wifiChanged = ssid != cfg.ssid || pass.length() > 0 || device != cfg.deviceName;
  copyText(cfg.deviceName, sizeof(cfg.deviceName), device);
  copyText(cfg.sensorName, sizeof(cfg.sensorName), device); // legacy field kept synchronized
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
  if (!sensorAdcPlausible(sensorState.rawAdc)) return sendError(409, "Sensor value is implausible / sensor not connected");
  if (dry) cfg.dryAdc = sensorState.rawAdc;
  else cfg.wetAdc = sensorState.rawAdc;
  saveConfig();
  sensorState.moisturePercent = moistureFromAdc(sensorState.filteredAdc);
  addLog(String("Calibration ") + (dry ? "dry" : "wet") + " = " + sensorState.rawAdc);
  JsonDocument doc; doc["ok"] = true; doc["dry_adc"] = cfg.dryAdc; doc["wet_adc"] = cfg.wetAdc; doc["calibrated"] = calibrated(); sendJson(doc);
}

static void scheduleRestart(uint32_t delayMs) {
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
  server.on("/api/ota/check", HTTP_GET, apiOtaCheck);
  server.on("/api/ota/update", HTTP_POST, apiOtaUpdate);
  server.on("/api/ota/status", HTTP_GET, apiOtaStatus);
  server.on("/api/ota/readme", HTTP_GET, apiOtaReadme);
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
  reportPreviousOta();
  setupRoutes();
  server.begin();
  addLog("HTTP server started");
}

void loop() {
  server.handleClient();
  wifiLoop();
  sensorLoop();
  otaLoop();

  if (restartPending && (int32_t)(millis() - restartAt) >= 0) {
    delay(50);
    ESP.restart();
  }
  yield();
}
