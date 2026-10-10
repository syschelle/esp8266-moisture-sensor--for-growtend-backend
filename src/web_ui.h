#pragma once
#include <Arduino.h>

static const char WEB_UI[] PROGMEM = R"HTML(
<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SoilSensor</title>
<style>
:root{--top:#2f465c;--side:#38546d;--side-active:#314b63;--bg:#f4f4f4;--card:#fff;--text:#222;--muted:#858585;--line:#dcdcdc;--good:#087b35;--warn:#b66b00;--bad:#b42318;--primary:#2f65d9;--shadow:0 2px 7px rgba(0,0,0,.10)}
:root[data-theme="dark"]{--top:#1f2e3c;--side:#293f52;--side-active:#213547;--bg:#121920;--card:#1a232c;--text:#edf2f7;--muted:#9da8b2;--line:#34414c;--good:#45c976;--warn:#f0b84b;--bad:#ef6a6a;--primary:#6da8ff;--shadow:none}
*{box-sizing:border-box}
html,body{margin:0;min-height:100%;font-family:Segoe UI,Arial,sans-serif;background:var(--bg);color:var(--text)}
.topbar{height:49px;background:var(--top);color:#fff;display:flex;align-items:center;padding:0 12px;font-weight:700;position:fixed;top:0;left:0;right:0;z-index:20}
.brand{font-size:15px}
.sidebar{position:fixed;left:0;top:49px;bottom:0;width:205px;background:var(--side);padding-top:7px;z-index:15}
.navbtn{display:block;width:100%;border:0;background:transparent;color:#fff;text-align:left;padding:14px 13px;font-size:15px;cursor:pointer}
.navbtn:hover,.navbtn.active{background:var(--side-active)}
.content{margin-left:205px;padding:87px 32px 40px;min-height:100vh}
.page{display:none}.page.active{display:block}
.headrow{display:flex;align-items:flex-start;justify-content:space-between;gap:18px;margin-bottom:29px}
h1{font-size:30px;line-height:1.2;margin:0 0 17px;font-weight:700}
h2{font-size:18px;margin:0 0 15px;font-weight:700}
.subtitle{color:var(--muted);font-size:15px}
.badge{border:1px solid var(--line);border-radius:18px;padding:5px 11px;font-size:12px;font-weight:700;white-space:nowrap;background:transparent}
.badge.good{color:var(--good);border-color:var(--good)}.badge.warn{color:var(--warn);border-color:var(--warn)}.badge.bad{color:var(--bad);border-color:var(--bad)}
.card{background:var(--card);border:1px solid var(--line);border-radius:11px;box-shadow:var(--shadow);padding:16px}
.hero{margin-bottom:15px;padding:20px 18px 18px}
.hero-label{font-size:12px;letter-spacing:.04em;color:var(--muted);text-transform:uppercase}
.hero-value{font-size:48px;font-weight:800;letter-spacing:2px;margin:28px 0 15px}
.hero-bottom{display:flex;align-items:center;justify-content:space-between;gap:15px;color:var(--muted)}
.grid2{display:grid;grid-template-columns:1fr 1fr;gap:14px}
.row{display:flex;align-items:flex-start;justify-content:space-between;gap:18px;padding:9px 0;border-bottom:1px solid var(--line);min-height:37px}
.row:last-child{border-bottom:0}.row span:first-child{color:var(--muted)}.row b{font-weight:400;text-align:right;overflow-wrap:anywhere}
label{display:block;margin:14px 0 6px;font-weight:600}
input,select{width:100%;padding:10px 11px;border:1px solid var(--line);border-radius:6px;background:var(--card);color:var(--text);font:inherit}
input:focus,select:focus{outline:2px solid rgba(47,101,217,.18);border-color:var(--primary)}
.actions{display:flex;gap:9px;flex-wrap:wrap;margin-top:16px}
button.action{border:0;border-radius:7px;padding:10px 14px;background:var(--primary);color:#fff;font-weight:700;cursor:pointer}
button.secondary{background:transparent;color:var(--text);border:1px solid var(--line)}button.danger{background:var(--bad)}
.hint{color:var(--muted);font-size:12px;margin-top:7px}.saveState{font-size:12px;margin-top:10px;color:var(--muted)}
#otaMsg{color:var(--text)}
.releaseReadme{white-space:pre-wrap;overflow-wrap:anywhere;line-height:1.45;color:var(--text);font-size:14px}
pre{margin:14px 0 0;background:#0b1118;color:#dce8f2;padding:14px;border-radius:7px;overflow:auto;max-height:520px;white-space:pre-wrap}
.progress{height:9px;background:var(--line);border-radius:999px;overflow:hidden;margin-top:15px}.progress span{display:block;height:100%;width:0;background:var(--primary)}
@media(max-width:850px){.sidebar{width:175px}.content{margin-left:175px;padding:75px 18px 26px}.grid2{grid-template-columns:1fr}}
@media(max-width:620px){.topbar{position:static}.sidebar{position:static;width:auto;display:flex;overflow:auto;padding:0}.navbtn{width:auto;white-space:nowrap;padding:12px}.content{margin-left:0;padding:18px 12px}.headrow{margin-bottom:18px}.hero-value{font-size:40px}}
</style>
</head>
<body>
<div class="topbar"><div class="brand" id="topBrand">ESP8266 Moisture Sensor</div></div>
<div class="sidebar">
  <button class="navbtn active" data-page="status">Status</button>
  <button class="navbtn" data-page="sensor" data-i18n="sensor">Sensor</button>
  <button class="navbtn" data-page="system" data-i18n="systemSettings">Systemeinstellungen</button>
  <button class="navbtn" data-page="log" data-i18n="systemLog">Systemprotokoll</button>
  <button class="navbtn" data-page="ota">OTA Update</button>
  <button class="navbtn" data-page="reset" data-i18n="factory">Werkseinstellungen</button>
</div>

<main class="content">
<section class="page active" id="status">
  <div class="headrow">
    <div><h1>Status</h1><div class="subtitle" data-i18n="statusSub">Lokaler Sensorstatus und aktuelle Bodenfeuchte.</div></div>
    <span id="connBadge" class="badge warn">...</span>
  </div>

  <div class="card hero">
    <div class="hero-label" data-i18n="currentMoisture">AKTUELLE BODENFEUCHTE</div>
    <div class="hero-value" id="heroMoisture">-- %</div>
    <div class="hero-bottom"><span id="heroSensor">--</span><span id="heroCal">--</span></div>
  </div>

  <div class="grid2">
    <div class="card">
      <div class="headrow" style="margin-bottom:8px"><h2 data-i18n="soilSensor">Bodenfeuchtesensor</h2><span id="sensorBadge" class="badge warn">...</span></div>
      <div class="row"><span>ADC Rohwert</span><b id="stRaw">--</b></div>
      <div class="row"><span data-i18n="signalPin">Signalpin</span><b id="stPin">A0</b></div>
      <div class="row"><span data-i18n="dryValue">Trockenwert</span><b id="stDry">--</b></div>
      <div class="row"><span data-i18n="wetValue">Nasswert</span><b id="stWet">--</b></div>
      <div class="row"><span data-i18n="lastMeasurement">Letzte Messung</span><b id="stLast">--</b></div>
      <div class="row"><span data-i18n="interval">Messintervall</span><b id="stInterval">--</b></div>
    </div>

    <div class="card">
      <div class="headrow" style="margin-bottom:8px"><h2>System</h2><span id="sysBadge" class="badge warn">...</span></div>
      <div class="row"><span data-i18n="localTime">Lokale Zeit</span><b id="stTime">--</b></div>
      <div class="row"><span>NTP</span><b id="stNtp">--</b></div>
      <div class="row"><span data-i18n="ipAddress">IP-Adresse</span><b id="stIp">--</b></div>
      <div class="row"><span>WLAN RSSI</span><b id="stRssi">--</b></div>
      <div class="row"><span>Uptime</span><b id="stUptime">--</b></div>
      <div class="row"><span>Free Heap</span><b id="stHeap">--</b></div>
      <div class="row"><span data-i18n="firmwareVersion">Firmware-Version</span><b id="stVersion">--</b></div>
    </div>
  </div>
</section>

<section class="page" id="sensor">
  <div class="headrow"><div><h1 data-i18n="sensor">Sensor</h1><div class="subtitle" data-i18n="sensorSub">Messung und Kalibrierung des kapazitiven Bodenfeuchtesensors.</div></div></div>
  <div class="grid2">
    <div class="card">
      <h2 data-i18n="sensorSettings">Sensoreinstellungen</h2>
      <label data-i18n="signalPin">Signalpin</label>
      <select id="signalPin"><option value="A0">A0 (ADC)</option></select>
      <div class="hint" data-i18n="pinHint">Beim ESP8266 ist A0 der analoge Sensoreingang.</div>
      <label data-i18n="measureInterval">Messintervall (Sekunden)</label><input id="interval" type="number" min="1" max="300">
      <label data-i18n="sampleCount">Messungen pro Mittelwert</label><input id="samples" type="number" min="1" max="50">
      <div class="actions"><button class="action" onclick="saveSensor()" data-i18n="save">Speichern</button></div><div id="sensorSaveState" class="saveState"></div>
    </div>
    <div class="card">
      <h2 data-i18n="calibration">Kalibrierung</h2>
      <div class="row"><span>ADC live</span><b id="liveRaw">--</b></div>
      <div class="row"><span data-i18n="dryValue">Trockenwert</span><b id="dryVal">--</b></div>
      <div class="row"><span data-i18n="wetValue">Nasswert</span><b id="wetVal">--</b></div>
      <div class="row"><span data-i18n="moisture">Bodenfeuchte</span><b id="calMoist">--</b></div>
      <div class="actions">
        <button class="action" onclick="calibrate('dry')" data-i18n="takeDry">Aktuellen Wert als TROCKEN speichern</button>
        <button class="action" onclick="calibrate('wet')" data-i18n="takeWet">Aktuellen Wert als NASS speichern</button>
      </div>
      <div class="hint" data-i18n="calHint">Trocken und nass müssen ausreichend unterschiedliche ADC-Werte liefern.</div>
      <div style="margin-top:18px;padding-top:16px;border-top:1px solid var(--line)">
        <h3 data-i18n="manualCalibration">Kalibrierwerte manuell anpassen</h3>
        <label data-i18n="manualDry">Trockenwert manuell</label><input id="manualDry" type="number" min="51" max="1000" step="1">
        <label data-i18n="manualWet">Nasswert manuell</label><input id="manualWet" type="number" min="51" max="1000" step="1">
        <div class="hint" data-i18n="manualCalHint">Zulässig sind ADC-Werte von 51 bis 1000. Trocken- und Nasswert müssen mindestens 40 Punkte auseinanderliegen.</div>
        <div class="actions"><button class="action" onclick="saveManualCalibration()" data-i18n="saveCalibration">Kalibrierwerte speichern</button></div>
        <div id="manualCalState" class="saveState"></div>
      </div>
    </div>
  </div>
</section>

<section class="page" id="system">
  <div class="headrow"><div><h1 data-i18n="systemSettings">Systemeinstellungen</h1><div class="subtitle" data-i18n="systemSub">WLAN, Gerätename, NTP und Oberfläche konfigurieren.</div></div></div>
  <div class="card">
    <label data-i18n="deviceName">Gerätename</label><input id="deviceName" maxlength="32" pattern="[A-Za-z0-9](?:[A-Za-z0-9-]{0,30}[A-Za-z0-9])?" autocomplete="off">
    <div class="hint" data-i18n="deviceNameHint">Wird auch als Netzwerk-Hostname verwendet. Nur A-Z, a-z, 0-9 und Bindestrich; keine Leerzeichen oder Umlaute.</div>
    <label>Wi-Fi SSID</label><input id="ssid" maxlength="32">
    <label data-i18n="wifiPassword">Wi-Fi Passwort</label><input id="wifiPass" type="password" maxlength="64" placeholder="********">
    <div class="hint" data-i18n="passwordHint">Leer lassen, um das gespeicherte Passwort beizubehalten.</div>
    <label>NTP Server</label><input id="ntpServer" maxlength="63">
    <label data-i18n="timezone">Zeitzone (POSIX TZ)</label><input id="timezone" maxlength="63">
    <label data-i18n="language">Sprache</label><select id="language"><option value="de">Deutsch</option><option value="en">English</option></select>
    <label>Theme</label><select id="theme"><option value="light">Light</option><option value="dark">Dark</option></select>
    <div class="actions"><button class="action" onclick="saveSystem()" data-i18n="save">Speichern</button><button class="action secondary" onclick="reboot()" data-i18n="reboot">Neustart</button></div><div id="systemSaveState" class="saveState"></div>
  </div>
</section>

<section class="page" id="log">
  <div class="headrow"><div><h1 data-i18n="systemLog">Systemprotokoll</h1></div></div>
  <div class="card"><button class="action secondary" onclick="loadLog()" data-i18n="refresh">Aktualisieren</button><pre id="logText">...</pre></div>
</section>

<section class="page" id="ota">
  <div class="headrow"><div><h1>OTA Update</h1><div class="subtitle" data-i18n="otaSub">Firmware aktualisieren.</div></div></div>
  <div class="card">
    <div class="row"><span data-i18n="installed">Installiert</span><b id="otaCurrent">--</b></div>
    <div class="row"><span data-i18n="available">Verfügbar</span><b id="otaAvailable">--</b></div>
    <div class="actions">
      <button id="otaCheckBtn" class="action secondary" onclick="checkOta()" data-i18n="checkUpdate">Update prüfen</button>
      <button id="otaInstallBtn" class="action" onclick="installOta()" style="display:none" data-i18n="installUpdate">Update installieren</button>
    </div>
    <div class="progress"><span id="otaProgress"></span></div>
    <div class="hint" id="otaMsg"></div>
  </div>
  <div class="card" style="margin-top:14px">
    <h2 data-i18n="manualUpdate">Manuelles Firmware-Update</h2>
    <input id="fwFile" type="file" accept=".bin,application/octet-stream">
    <div class="actions"><button class="action" onclick="manualUpload()" data-i18n="uploadFirmware">Firmware hochladen</button></div>
  </div>
  <div id="otaReadmeCard" class="card" style="margin-top:14px;display:none">
    <h2 data-i18n="updateChanges">Änderungen der verfügbaren Firmware</h2>
    <div id="otaReadme" class="releaseReadme">--</div>
  </div>
</section>

<section class="page" id="reset">
  <div class="headrow"><div><h1 data-i18n="factory">Werkseinstellungen</h1></div></div>
  <div class="card">
    <p data-i18n="resetText">Alle gespeicherten Einstellungen einschließlich WLAN und Sensorkalibrierung werden gelöscht.</p>
    <button class="action danger" onclick="factoryReset()" data-i18n="resetButton">Werkseinstellungen laden</button>
  </div>
</section>
</main>

<script>
let S={},lang='de',activePage='status',formDirty=false,saveInProgress=false;
const T={
de:{
 sensor:'Sensor',systemSettings:'Systemeinstellungen',systemLog:'Systemprotokoll',factory:'Werkseinstellungen',
 statusSub:'Lokaler Sensorstatus und aktuelle Bodenfeuchte.',currentMoisture:'AKTUELLE BODENFEUCHTE',
 soilSensor:'Bodenfeuchtesensor',signalPin:'Signalpin',dryValue:'Trockenwert',wetValue:'Nasswert',
 lastMeasurement:'Letzte Messung',interval:'Messintervall',localTime:'Lokale Zeit',ipAddress:'IP-Adresse',
 firmwareVersion:'Firmware-Version',sensorSub:'Messung und Kalibrierung des kapazitiven Bodenfeuchtesensors.',
 sensorSettings:'Sensoreinstellungen',sensorName:'Sensorname',pinHint:'Beim ESP8266 ist A0 der analoge Sensoreingang.',
 measureInterval:'Messintervall (Sekunden)',sampleCount:'Messungen pro Mittelwert',save:'Speichern',
 calibration:'Kalibrierung',moisture:'Bodenfeuchte',takeDry:'Aktuellen Wert als TROCKEN speichern',
 takeWet:'Aktuellen Wert als NASS speichern',calHint:'Trocken und nass müssen ausreichend unterschiedliche ADC-Werte liefern.',manualCalibration:'Kalibrierwerte manuell anpassen',manualDry:'Trockenwert manuell',manualWet:'Nasswert manuell',manualCalHint:'Zulässig sind ADC-Werte von 51 bis 1000. Trocken- und Nasswert müssen mindestens 40 Punkte auseinanderliegen.',saveCalibration:'Kalibrierwerte speichern',
 systemSub:'WLAN, Gerätename, NTP und Oberfläche konfigurieren.',deviceName:'Gerätename',deviceNameHint:'Wird auch als Netzwerk-Hostname verwendet. Nur A-Z, a-z, 0-9 und Bindestrich; keine Leerzeichen oder Umlaute.',wifiPassword:'Wi-Fi Passwort',
 passwordHint:'Leer lassen, um das gespeicherte Passwort beizubehalten.',timezone:'Zeitzone (POSIX TZ)',language:'Sprache',
 reboot:'Neustart',refresh:'Aktualisieren',otaSub:'Firmware aktualisieren.',installed:'Installiert',available:'Verfügbar',
 checkUpdate:'Update prüfen',installUpdate:'Update installieren',updateChanges:'Änderungen der verfügbaren Firmware',manualUpdate:'Manuelles Firmware-Update',uploadFirmware:'Firmware hochladen',
 resetText:'Alle gespeicherten Einstellungen einschließlich WLAN und Sensorkalibrierung werden gelöscht.',
 resetButton:'Werkseinstellungen laden',connected:'Verbunden',apMode:'AP-Modus',offline:'Nicht erreichbar',
 calibrated:'Kalibriert',notCalibrated:'Nicht kalibriert',active:'Aktiv',waiting:'Warte auf Messung',sensorDisconnected:'Sensor nicht erkannt',synchronized:'Synchronisiert',waitingNtp:'Wartet'
},
en:{
 sensor:'Sensor',systemSettings:'System settings',systemLog:'System log',factory:'Factory reset',
 statusSub:'Local sensor status and current soil moisture.',currentMoisture:'CURRENT SOIL MOISTURE',
 soilSensor:'Soil moisture sensor',signalPin:'Signal pin',dryValue:'Dry value',wetValue:'Wet value',
 lastMeasurement:'Last measurement',interval:'Measurement interval',localTime:'Local time',ipAddress:'IP address',
 firmwareVersion:'Firmware version',sensorSub:'Measurement and calibration of the capacitive soil-moisture sensor.',
 sensorSettings:'Sensor settings',sensorName:'Sensor name',pinHint:'On ESP8266, A0 is the analog sensor input.',
 measureInterval:'Measurement interval (seconds)',sampleCount:'Samples per average',save:'Save',
 calibration:'Calibration',moisture:'Soil moisture',takeDry:'Store current value as DRY',
 takeWet:'Store current value as WET',calHint:'Dry and wet must provide sufficiently different ADC values.',manualCalibration:'Adjust calibration values manually',manualDry:'Manual dry value',manualWet:'Manual wet value',manualCalHint:'Allowed ADC range is 51 to 1000. Dry and wet values must differ by at least 40 points.',saveCalibration:'Save calibration values',
 systemSub:'Configure Wi-Fi, device name, NTP and user interface.',deviceName:'Device name',deviceNameHint:'Also used as the network hostname. Use only A-Z, a-z, 0-9 and hyphen; no spaces or umlauts.',wifiPassword:'Wi-Fi password',
 passwordHint:'Leave empty to keep the stored password.',timezone:'Timezone (POSIX TZ)',language:'Language',
 reboot:'Reboot',refresh:'Refresh',otaSub:'Update firmware.',installed:'Installed',available:'Available',
 checkUpdate:'Check for update',installUpdate:'Install update',updateChanges:'Changes in available firmware',manualUpdate:'Manual firmware update',uploadFirmware:'Upload firmware',
 resetText:'All saved settings including Wi-Fi and sensor calibration will be erased.',
 resetButton:'Restore factory settings',connected:'Connected',apMode:'AP mode',offline:'Offline',
 calibrated:'Calibrated',notCalibrated:'Not calibrated',active:'Active',waiting:'Waiting for measurement',sensorDisconnected:'Sensor not detected',synchronized:'Synchronized',waitingNtp:'Waiting'
}};
function tr(k){return (T[lang]&&T[lang][k])||k}
function applyI18n(){document.querySelectorAll('[data-i18n]').forEach(e=>e.textContent=tr(e.dataset.i18n))}
function setTheme(v){document.documentElement.dataset.theme=v||'light'}
document.querySelectorAll('.navbtn').forEach(b=>b.onclick=()=>{
 document.querySelectorAll('.navbtn').forEach(x=>x.classList.remove('active'));
 document.querySelectorAll('.page').forEach(x=>x.classList.remove('active'));
 b.classList.add('active');activePage=b.dataset.page;document.getElementById(activePage).classList.add('active');
 if(activePage==='log')loadLog();
 if(activePage==='status'){formDirty=false;loadState(true)}
});
async function api(url,opt){let r=await fetch(url,opt);let tx=await r.text(),j={};try{j=JSON.parse(tx)}catch(e){}if(!r.ok)throw Error(j.error||tx||('HTTP '+r.status));return j}
async function postForm(url,data){return api(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(data)})}
function uptime(s){s=Number(s||0);let d=Math.floor(s/86400),h=Math.floor((s%86400)/3600),m=Math.floor((s%3600)/60);return(d?d+' d ':'')+h+' h '+m+' min'}
function badge(el,text,cls){el.textContent=text;el.className='badge '+cls}
function pct(v){return(v===null||v===undefined)?'-- %':Number(v).toFixed(1)+' %'}
async function loadState(forceFormFill=false){
 try{
   S=await api('/api/state');lang=S.settings.language||'de';applyI18n();setTheme(S.settings.theme||'light');
   topBrand.textContent=S.settings.device_name||'ESP8266 Moisture Sensor';
   document.title=S.settings.device_name||'ESP8266 Moisture Sensor';

   let connected=!!S.wifi.connected, ap=!!S.wifi.ap_active;
   if(connected)badge(connBadge,tr('connected'),'good');else if(ap)badge(connBadge,tr('apMode'),'warn');else badge(connBadge,tr('offline'),'bad');
   badge(sysBadge,connected?tr('connected'):(ap?tr('apMode'):tr('offline')),connected?'good':(ap?'warn':'bad'));

   const sensorStateCode=S.sensor.status||'no_measurement';
   heroMoisture.textContent=(S.sensor.valid&&S.sensor.plausible)?pct(S.sensor.moisture_percent):'-- %';
   heroSensor.textContent=S.settings.device_name||'--';
   heroCal.textContent=S.sensor.calibrated?tr('calibrated'):tr('notCalibrated');
   if(sensorStateCode==='not_connected')badge(sensorBadge,tr('sensorDisconnected'),'bad');
   else if(sensorStateCode==='not_calibrated')badge(sensorBadge,tr('notCalibrated'),'warn');
   else if(sensorStateCode==='ok')badge(sensorBadge,tr('active'),'good');
   else badge(sensorBadge,tr('waiting'),'warn');

   stRaw.textContent=S.sensor.valid?S.sensor.raw_adc:'--';
   stPin.textContent=S.settings.signal_pin||'A0';
   stDry.textContent=S.settings.dry_adc;
   stWet.textContent=S.settings.wet_adc;
   stLast.textContent=S.sensor.last_measurement_local||'--';
   stInterval.textContent=S.settings.measure_interval_seconds+' s';

   stTime.textContent=(S.time&&S.time.local)||'--';
   stNtp.textContent=(S.time&&S.time.valid)?tr('synchronized'):tr('waitingNtp');
   stIp.textContent=connected?S.wifi.ip:(ap?S.wifi.ap_ip:'--');
   stRssi.textContent=connected?(S.wifi.rssi+' dBm'):'--';
   stUptime.textContent=uptime(S.uptime_seconds);
   stHeap.textContent=(S.free_heap||0)+' B';
   stVersion.textContent='v'+String(S.version||'').replace(/^v/,'');
   liveRaw.textContent=S.sensor.valid?S.sensor.raw_adc:'--';
   dryVal.textContent=S.settings.dry_adc;wetVal.textContent=S.settings.wet_adc;manualDry.value=S.settings.dry_adc;manualWet.value=S.settings.wet_adc;
   calMoist.textContent=(S.sensor.valid&&S.sensor.plausible)?pct(S.sensor.moisture_percent):'--';

   if(forceFormFill || (!formDirty && !saveInProgress)){
     signalPin.value=S.settings.signal_pin||'A0';
     interval.value=S.settings.measure_interval_seconds;samples.value=S.settings.sample_count;
     deviceName.value=S.settings.device_name||'';ssid.value=S.settings.ssid||'';
     ntpServer.value=S.settings.ntp_server||'';timezone.value=S.settings.timezone||'';
     language.value=S.settings.language||'de';theme.value=S.settings.theme||'light';
   }
   otaCurrent.textContent='v'+String(S.version||'').replace(/^v/,'');
 }catch(e){badge(connBadge,tr('offline'),'bad')}
}
async function saveSensor(){
 try{
  saveInProgress=true;sensorSaveState.textContent=lang==='de'?'Speichere…':'Saving…';
  await postForm('/api/settings/sensor',{signal_pin:signalPin.value,interval:interval.value,samples:samples.value});
  formDirty=false;sensorSaveState.textContent=lang==='de'?'Gespeichert.':'Saved.';
  await loadState(true);
 }catch(e){sensorSaveState.textContent=e.message;alert(e.message)}
 finally{saveInProgress=false}
}
async function saveSystem(){
 try{
  const devicePattern=/^[A-Za-z0-9](?:[A-Za-z0-9-]{0,30}[A-Za-z0-9])?$/;
  if(!devicePattern.test(deviceName.value)){
   throw Error(lang==='de'?'Gerätename: nur A-Z, a-z, 0-9 und Bindestrich; keine Leerzeichen oder Umlaute.':'Device name: only A-Z, a-z, 0-9 and hyphen; no spaces or umlauts.');
  }
  saveInProgress=true;systemSaveState.textContent=lang==='de'?'Speichere…':'Saving…';
  let result=await postForm('/api/settings/system',{device_name:deviceName.value,ssid:ssid.value,wifi_password:wifiPass.value,ntp_server:ntpServer.value,timezone:timezone.value,language:language.value,theme:theme.value});
  formDirty=false;wifiPass.value='';
  systemSaveState.textContent=result.wifi_changed?(lang==='de'?'Gespeichert. WLAN-Daten geändert – Neustart empfohlen.':'Saved. Wi-Fi changed – reboot recommended.'):(lang==='de'?'Gespeichert.':'Saved.');
  await loadState(true);
 }catch(e){systemSaveState.textContent=e.message;alert(e.message)}
 finally{saveInProgress=false}
}
async function saveManualCalibration(){
 try{
  const dry=parseInt(manualDry.value,10),wet=parseInt(manualWet.value,10);
  if(!Number.isInteger(dry)||!Number.isInteger(wet)||dry<=50||dry>1000||wet<=50||wet>1000){
   throw Error(lang==='de'?'Trocken- und Nasswert müssen zwischen 51 und 1000 liegen.':'Dry and wet values must be between 51 and 1000.');
  }
  if(Math.abs(dry-wet)<40){
   throw Error(lang==='de'?'Trocken- und Nasswert müssen mindestens 40 ADC-Punkte auseinanderliegen.':'Dry and wet values must differ by at least 40 ADC points.');
  }
  manualCalState.textContent=lang==='de'?'Speichere…':'Saving…';
  await postForm('/api/calibration/manual',{dry_adc:dry,wet_adc:wet});
  manualCalState.textContent=lang==='de'?'Kalibrierwerte gespeichert.':'Calibration values saved.';
  await loadState(true);
 }catch(e){manualCalState.textContent=e.message}
}

async function calibrate(which){try{await postForm('/api/calibration/'+which,{});await loadState()}catch(e){alert(e.message)}}
async function loadLog(){try{let r=await fetch('/api/log');logText.textContent=await r.text()}catch(e){logText.textContent=e.message}}
async function reboot(){if(confirm('Reboot?')){await postForm('/api/reboot',{})}}
async function factoryReset(){if(confirm(lang==='de'?'Wirklich alle Einstellungen löschen?':'Really erase all settings?')){await postForm('/api/factory-reset',{})}}
let otaExpectedVersion='';
let otaWatchTimer=null;
async function loadOtaReadme(){
 try{
  let r=await fetch('/api/ota/readme',{cache:'no-store'}),text=await r.text();
  if(!r.ok)throw Error(text||('HTTP '+r.status));
  otaReadme.textContent=text;
  otaReadmeCard.style.display='block';
  return true;
 }catch(e){
  otaReadme.textContent=lang==='de'?'Änderungen konnten nicht geladen werden: '+e.message:'Could not load changes: '+e.message;
  otaReadmeCard.style.display='block';
  return false;
 }
}
async function checkOta(){
 try{
  otaCheckBtn.disabled=true;
  otaInstallBtn.style.display='none';
  otaInstallBtn.disabled=true;
  otaReadmeCard.style.display='none';
  otaReadme.textContent='--';
  otaMsg.textContent=lang==='de'?'Prüfe Update…':'Checking update…';
  otaProgress.style.width='0';

  let c=await api('/api/ota/check');
  otaAvailable.textContent='v'+String(c.available_version||'').replace(/^v/,'');
  otaExpectedVersion=String(c.available_version||'').replace(/^v/,'');

  if(c.update_available){
   otaMsg.textContent=lang==='de'?'Neue Firmware verfügbar. Lade Änderungen…':'New firmware available. Loading changes…';

   let releaseNotesReady=false;
   if(c.release_notes){
    otaReadme.textContent=c.release_notes;
    otaReadmeCard.style.display='block';
    releaseNotesReady=true;
   }else{
    releaseNotesReady=await loadOtaReadme();
   }

   if(releaseNotesReady){
    otaMsg.textContent=lang==='de'?'Neue Firmware verfügbar.':'New firmware available.';
    otaInstallBtn.disabled=false;
    otaInstallBtn.style.display='inline-block';
   }else{
    otaMsg.textContent=lang==='de'?'Neue Firmware verfügbar, aber die Änderungen konnten nicht geladen werden.':'New firmware is available, but the release notes could not be loaded.';
   }
  }else{
   otaMsg.textContent=lang==='de'?'Keine neuere Version verfügbar.':'No newer version available.';
  }
 }catch(e){
  otaMsg.textContent=e.message;
 }finally{
  otaCheckBtn.disabled=false;
 }
}
async function installOta(){
 try{
  otaInstallBtn.disabled=true; otaCheckBtn.disabled=true; otaMsg.textContent=lang==='de'?'Update wird gestartet…':'Starting update…'; otaProgress.style.width='15%';
  let r=await api('/api/ota/update',{method:'POST'}); otaExpectedVersion=String(r.target_version||otaExpectedVersion).replace(/^v/,'');
  otaMsg.textContent=lang==='de'?'Firmware wird geladen und installiert. Gerät nicht ausschalten.':'Firmware is being downloaded and installed. Do not power off.'; otaProgress.style.width='45%'; watchOtaRestart();
 }catch(e){ otaInstallBtn.disabled=false; otaCheckBtn.disabled=false; otaMsg.textContent=e.message; }
}
function watchOtaRestart(){
 if(otaWatchTimer)clearInterval(otaWatchTimer); let misses=0;
 otaWatchTimer=setInterval(async()=>{
  try{
   let st=await api('/api/state'); let current=String(st.version||'').replace(/^v/,''); misses=0;
   if(otaExpectedVersion && current===otaExpectedVersion){ clearInterval(otaWatchTimer); otaWatchTimer=null; otaProgress.style.width='100%'; otaMsg.textContent=lang==='de'?'Update erfolgreich installiert.':'Update installed successfully.'; otaCurrent.textContent='v'+current; otaInstallBtn.style.display='none'; otaCheckBtn.disabled=false; return; }
   try{ let os=await api('/api/ota/status'); if(os.state==='failed'){ clearInterval(otaWatchTimer); otaWatchTimer=null; otaProgress.style.width='0'; otaMsg.textContent=os.message||'OTA failed'; otaInstallBtn.disabled=false; otaCheckBtn.disabled=false; } else if(os.state==='rebooting'){ otaProgress.style.width='85%'; otaMsg.textContent=lang==='de'?'Firmware installiert – Neustart…':'Firmware installed – rebooting…'; } else if(os.running) otaProgress.style.width='55%'; }catch(_){}
  }catch(_){ misses++; if(misses>2){ otaProgress.style.width='75%'; otaMsg.textContent=lang==='de'?'ESP startet neu…':'ESP is rebooting…'; } }
 },2000);
}
async function manualUpload(){
 try{
  const f=fwFile.files[0];
  if(!f)throw Error(lang==='de'?'Bitte firmware.bin auswählen.':'Please select firmware.bin.');
  if(!f.name.toLowerCase().endsWith('.bin'))throw Error(lang==='de'?'Bitte eine .bin-Datei auswählen.':'Please select a .bin file.');

  otaCheckBtn.disabled=true;
  otaInstallBtn.disabled=true;
  otaProgress.style.width='0';
  otaMsg.textContent=lang==='de'?'Firmware wird über WLAN hochgeladen…':'Uploading firmware over Wi-Fi…';

  const form=new FormData();
  form.append('firmware',f,f.name);

  await new Promise((resolve,reject)=>{
   const xhr=new XMLHttpRequest();
   xhr.open('POST','/api/ota/upload',true);
   xhr.timeout=120000;

   xhr.upload.onprogress=(ev)=>{
    if(ev.lengthComputable){
     const pct=Math.max(1,Math.min(99,Math.round((ev.loaded/ev.total)*100)));
     otaProgress.style.width=pct+'%';
     otaMsg.textContent=(lang==='de'?'Firmware-Upload: ':'Firmware upload: ')+pct+'%';
    }
   };

   xhr.onload=()=>{
    let j={};
    try{j=JSON.parse(xhr.responseText||'{}')}catch(_){}
    if(xhr.status>=200&&xhr.status<300&&j.ok)resolve(j);
    else reject(Error(j.error||xhr.responseText||('HTTP '+xhr.status)));
   };
   xhr.onerror=()=>reject(Error(lang==='de'?'WLAN-Verbindung beim Firmware-Upload unterbrochen.':'Wi-Fi connection interrupted during firmware upload.'));
   xhr.ontimeout=()=>reject(Error(lang==='de'?'Firmware-Upload Zeitüberschreitung.':'Firmware upload timed out.'));
   xhr.send(form);
  });

  otaProgress.style.width='100%';
  otaMsg.textContent=lang==='de'?'Firmware übertragen. ESP startet neu…':'Firmware transferred. ESP is rebooting…';
  watchManualOtaRestart();
 }catch(e){
  otaProgress.style.width='0';
  otaMsg.textContent=e.message;
  otaCheckBtn.disabled=false;
  otaInstallBtn.disabled=false;
 }
}

function watchManualOtaRestart(){
 let sawOffline=false,tries=0;
 const timer=setInterval(async()=>{
  tries++;
  try{
   const st=await api('/api/state');
   if(sawOffline||tries>=4){
    clearInterval(timer);
    otaCurrent.textContent='v'+String(st.version||'').replace(/^v/,'');
    otaProgress.style.width='100%';
    otaMsg.textContent=lang==='de'?'Manuelles Firmware-Update erfolgreich.':'Manual firmware update successful.';
    otaCheckBtn.disabled=false;
    otaInstallBtn.disabled=false;
    fwFile.value='';
   }
  }catch(_){
   sawOffline=true;
   otaMsg.textContent=lang==='de'?'ESP startet neu…':'ESP is rebooting…';
  }

  if(tries>45){
   clearInterval(timer);
   otaMsg.textContent=lang==='de'?'ESP nach Update noch nicht erreichbar.':'ESP is not reachable after update yet.';
   otaCheckBtn.disabled=false;
   otaInstallBtn.disabled=false;
  }
 },2000);
}
document.querySelectorAll('#sensor input,#sensor select,#system input,#system select').forEach(el=>{
 el.addEventListener('input',()=>{formDirty=true});
 el.addEventListener('change',()=>{formDirty=true});
});
loadState(true);
setInterval(()=>{if(activePage==='status'&&!saveInProgress)loadState(false);if(activePage==='log')loadLog();},3000);
</script>
</body>
</html>
)HTML";
