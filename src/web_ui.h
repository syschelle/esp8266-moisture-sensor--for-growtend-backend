#pragma once
#include <Arduino.h>

static const char WEB_UI[] PROGMEM = R"HTML(
<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP8266 Moisture Sensor</title>
<style>
:root{--bg:#eef2ff;--card:#fff;--fg:#172033;--muted:#65718a;--line:#dbe2f0;--accent:#2563eb;--good:#15803d;--warn:#b45309;--bad:#b91c1c;--shadow:0 8px 24px rgba(15,23,42,.08)}
:root[data-theme="dark"]{--bg:#0f172a;--card:#162033;--fg:#e5edf8;--muted:#9aabc2;--line:#2a3850;--accent:#60a5fa;--good:#4ade80;--warn:#fbbf24;--bad:#f87171;--shadow:none}
*{box-sizing:border-box} body{margin:0;background:var(--bg);color:var(--fg);font:14px/1.45 system-ui,-apple-system,Segoe UI,Roboto,sans-serif}
header{position:sticky;top:0;z-index:10;background:var(--card);border-bottom:1px solid var(--line);padding:12px 18px;display:flex;align-items:center;justify-content:space-between;gap:12px}
.brand{font-weight:800}.sub{color:var(--muted);font-size:12px}.wrap{max-width:1050px;margin:0 auto;padding:20px}
nav{display:flex;flex-wrap:wrap;gap:8px;margin-bottom:18px}.tab{border:1px solid var(--line);background:var(--card);color:var(--fg);padding:9px 13px;border-radius:10px;cursor:pointer}
.tab.active{background:var(--accent);border-color:var(--accent);color:#fff}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:14px}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:16px;box-shadow:var(--shadow)}.title{font-size:12px;color:var(--muted);text-transform:uppercase;letter-spacing:.04em}
.value{font-size:30px;font-weight:800;margin-top:5px}.row{display:flex;justify-content:space-between;gap:12px;padding:8px 0;border-bottom:1px solid var(--line)}.row:last-child{border-bottom:0}
label{display:block;margin:12px 0 5px;font-weight:650}input,select{width:100%;padding:10px 11px;border:1px solid var(--line);border-radius:9px;background:var(--card);color:var(--fg)}
button{border:0;background:var(--accent);color:#fff;padding:10px 14px;border-radius:9px;cursor:pointer;font-weight:700}button.secondary{background:transparent;color:var(--fg);border:1px solid var(--line)}button.danger{background:var(--bad)}
.actions{display:flex;gap:9px;flex-wrap:wrap;margin-top:15px}.hint{color:var(--muted);font-size:12px;margin-top:7px}.ok{color:var(--good)}.warn{color:var(--warn)}.bad{color:var(--bad)}
section{display:none}section.active{display:block}pre{background:#08111f;color:#d6e4f0;padding:14px;border-radius:10px;overflow:auto;max-height:420px;white-space:pre-wrap}
.progress{height:10px;background:var(--line);border-radius:999px;overflow:hidden;margin-top:9px}.progress>span{display:block;height:100%;background:var(--accent);width:0}
.bigmoist{font-size:56px;font-weight:900;line-height:1}.footer{color:var(--muted);text-align:center;padding:20px}
@media(max-width:650px){.wrap{padding:12px}.value{font-size:25px}.bigmoist{font-size:46px}}
</style>
</head>
<body>
<header>
  <div><div class="brand" id="headerName">ESP8266 Moisture Sensor</div><div class="sub" id="headerMeta">...</div></div>
  <button class="secondary" onclick="toggleTheme()" id="themeBtn">◐</button>
</header>
<div class="wrap">
<nav>
 <button class="tab active" data-tab="status">Status</button>
 <button class="tab" data-tab="sensor">Sensor</button>
 <button class="tab" data-tab="system">System</button>
 <button class="tab" data-tab="log">Systemprotokoll</button>
 <button class="tab" data-tab="ota">OTA</button>
 <button class="tab" data-tab="reset">Werkseinstellungen</button>
</nav>

<section id="status" class="active">
 <div class="grid">
  <div class="card"><div class="title" data-i18n="moisture">Bodenfeuchte</div><div class="bigmoist" id="moisture">-- %</div><div class="hint" id="moistState">...</div></div>
  <div class="card"><div class="title">ADC</div><div class="value" id="raw">--</div><div class="hint" id="pin">A0</div></div>
  <div class="card"><div class="title">WLAN</div><div class="value" id="rssi">-- dBm</div><div class="hint" id="ip">--</div></div>
  <div class="card"><div class="title" data-i18n="lastMeasurement">Letzte Messung</div><div class="value" style="font-size:18px" id="last">--</div><div class="hint" id="uptime">--</div></div>
 </div>
 <div class="card" style="margin-top:14px">
  <div class="row"><span data-i18n="device">Gerät</span><b id="devName">--</b></div>
  <div class="row"><span data-i18n="sensorName">Sensorname</span><b id="statusSensorName">--</b></div>
  <div class="row"><span data-i18n="firmware">Firmware</span><b id="fw">--</b></div>
  <div class="row"><span>NTP</span><b id="ntp">--</b></div>
  <div class="row"><span>API</span><b>/api/current-values</b></div>
 </div>
</section>

<section id="sensor">
 <div class="card">
  <h2 data-i18n="sensorSettings">Sensoreinstellungen</h2>
  <label data-i18n="sensorName">Sensorname</label><input id="sensorName" maxlength="32">
  <label data-i18n="signalPin">Signalpin</label>
  <select id="signalPin"><option value="A0">A0 (ADC)</option></select>
  <div class="hint" data-i18n="pinHint">Der ESP8266 besitzt nur einen analogen Eingang. Die Einstellung ist bereits so angelegt, dass spätere Plattformen mehrere ADC-Pins anbieten können.</div>
  <label data-i18n="measureInterval">Messintervall (Sekunden)</label><input id="interval" type="number" min="1" max="300">
  <label data-i18n="sampleCount">Messungen pro Mittelwert</label><input id="samples" type="number" min="1" max="50">
  <div class="actions"><button onclick="saveSensor()" data-i18n="save">Speichern</button></div>
 </div>
 <div class="card" style="margin-top:14px">
  <h2 data-i18n="calibration">Kalibrierung</h2>
  <div class="row"><span data-i18n="dryValue">Trockenwert</span><b id="dryVal">--</b></div>
  <div class="row"><span data-i18n="wetValue">Nasswert</span><b id="wetVal">--</b></div>
  <div class="row"><span>ADC live</span><b id="liveRaw">--</b></div>
  <div class="actions">
    <button onclick="calibrate('dry')" data-i18n="takeDry">Aktuellen Wert als TROCKEN speichern</button>
    <button onclick="calibrate('wet')" data-i18n="takeWet">Aktuellen Wert als NASS speichern</button>
  </div>
  <div class="hint" data-i18n="calHint">Für die Trocken-Kalibrierung den Sensor trocken messen. Für Nass den Sensor in gut befeuchteter Erde messen. Die Werte müssen ausreichend auseinander liegen.</div>
 </div>
</section>

<section id="system">
 <div class="card">
  <h2 data-i18n="systemSettings">Systemeinstellungen</h2>
  <label data-i18n="deviceName">Gerätename</label><input id="deviceName" maxlength="32">
  <label>Wi-Fi SSID</label><input id="ssid" maxlength="32">
  <label data-i18n="wifiPass">Wi-Fi Passwort</label><input id="wifiPass" type="password" maxlength="64" placeholder="********">
  <div class="hint" data-i18n="passHint">Leer lassen, um das gespeicherte Passwort beizubehalten.</div>
  <label>NTP Server</label><input id="ntpServer" maxlength="63">
  <label data-i18n="timezone">Zeitzone (POSIX TZ)</label><input id="timezone" maxlength="63">
  <label data-i18n="language">Sprache</label><select id="language"><option value="de">Deutsch</option><option value="en">English</option></select>
  <label>Theme</label><select id="theme"><option value="light">Light</option><option value="dark">Dark</option></select>
  <div class="actions"><button onclick="saveSystem()" data-i18n="save">Speichern</button><button class="secondary" onclick="reboot()" data-i18n="reboot">Neustart</button></div>
 </div>
</section>

<section id="log">
 <div class="card"><div class="actions" style="margin-top:0"><button class="secondary" onclick="loadLog()" data-i18n="refresh">Aktualisieren</button></div><pre id="logText">...</pre></div>
</section>

<section id="ota">
 <div class="card">
  <h2>OTA Update</h2>
  <div class="row"><span data-i18n="installed">Installiert</span><b id="otaCurrent">--</b></div>
  <div class="row"><span data-i18n="available">Verfügbar</span><b id="otaAvailable">--</b></div>
  <div class="actions"><button onclick="checkOta()" data-i18n="checkUpdate">Update prüfen</button></div>
  <div class="progress"><span id="otaProgress"></span></div>
  <div class="hint" id="otaMsg"></div>
 </div>
 <div class="card" style="margin-top:14px">
  <h3 data-i18n="manualUpdate">Manuelles Firmware-Update</h3>
  <input id="fwFile" type="file" accept=".bin,application/octet-stream">
  <div class="actions"><button onclick="manualUpload()" data-i18n="uploadFirmware">Firmware hochladen</button></div>
 </div>
</section>

<section id="reset">
 <div class="card">
  <h2 data-i18n="factoryReset">Werkseinstellungen</h2>
  <p data-i18n="resetText">Alle gespeicherten Einstellungen einschließlich WLAN und Kalibrierung werden gelöscht.</p>
  <button class="danger" onclick="factoryReset()" data-i18n="resetButton">Werkseinstellungen laden</button>
 </div>
</section>
<div class="footer" id="footer">ESP8266 Moisture Sensor</div>
</div>
<script>
let S={}, lang='de';
const T={
de:{moisture:'Bodenfeuchte',lastMeasurement:'Letzte Messung',device:'Gerät',sensorName:'Sensorname',firmware:'Firmware',sensorSettings:'Sensoreinstellungen',signalPin:'Signalpin',pinHint:'Der ESP8266 besitzt nur einen analogen Eingang. Die Einstellung ist bereits so angelegt, dass spätere Plattformen mehrere ADC-Pins anbieten können.',measureInterval:'Messintervall (Sekunden)',sampleCount:'Messungen pro Mittelwert',save:'Speichern',calibration:'Kalibrierung',dryValue:'Trockenwert',wetValue:'Nasswert',takeDry:'Aktuellen Wert als TROCKEN speichern',takeWet:'Aktuellen Wert als NASS speichern',calHint:'Für die Trocken-Kalibrierung den Sensor trocken messen. Für Nass den Sensor in gut befeuchteter Erde messen. Die Werte müssen ausreichend auseinander liegen.',systemSettings:'Systemeinstellungen',deviceName:'Gerätename',wifiPass:'Wi-Fi Passwort',passHint:'Leer lassen, um das gespeicherte Passwort beizubehalten.',timezone:'Zeitzone (POSIX TZ)',language:'Sprache',reboot:'Neustart',refresh:'Aktualisieren',installed:'Installiert',available:'Verfügbar',checkUpdate:'Update prüfen',manualUpdate:'Manuelles Firmware-Update',uploadFirmware:'Firmware hochladen',factoryReset:'Werkseinstellungen',resetText:'Alle gespeicherten Einstellungen einschließlich WLAN und Kalibrierung werden gelöscht.',resetButton:'Werkseinstellungen laden'},
en:{moisture:'Soil moisture',lastMeasurement:'Last measurement',device:'Device',sensorName:'Sensor name',firmware:'Firmware',sensorSettings:'Sensor settings',signalPin:'Signal pin',pinHint:'The ESP8266 has only one analog input. The setting is already structured so later platforms can expose multiple ADC pins.',measureInterval:'Measurement interval (seconds)',sampleCount:'Samples per average',save:'Save',calibration:'Calibration',dryValue:'Dry value',wetValue:'Wet value',takeDry:'Store current value as DRY',takeWet:'Store current value as WET',calHint:'For dry calibration measure the sensor dry. For wet calibration measure it in well-watered soil. The two values must be sufficiently different.',systemSettings:'System settings',deviceName:'Device name',wifiPass:'Wi-Fi password',passHint:'Leave empty to keep the stored password.',timezone:'Timezone (POSIX TZ)',language:'Language',reboot:'Reboot',refresh:'Refresh',installed:'Installed',available:'Available',checkUpdate:'Check for update',manualUpdate:'Manual firmware update',uploadFirmware:'Upload firmware',factoryReset:'Factory reset',resetText:'All stored settings including Wi-Fi and calibration will be erased.',resetButton:'Restore factory settings'}
};
function tr(){document.querySelectorAll('[data-i18n]').forEach(e=>{let k=e.dataset.i18n;if(T[lang]&&T[lang][k])e.textContent=T[lang][k]})}
function setTheme(t){document.documentElement.dataset.theme=t||'light'}
function toggleTheme(){let t=document.documentElement.dataset.theme==='dark'?'light':'dark';setTheme(t);document.getElementById('theme').value=t}
document.querySelectorAll('.tab').forEach(b=>b.onclick=()=>{document.querySelectorAll('.tab').forEach(x=>x.classList.remove('active'));document.querySelectorAll('section').forEach(x=>x.classList.remove('active'));b.classList.add('active');document.getElementById(b.dataset.tab).classList.add('active');if(b.dataset.tab==='log')loadLog()});
async function api(url,opt){let r=await fetch(url,opt);let txt=await r.text();let j={};try{j=JSON.parse(txt)}catch(e){} if(!r.ok)throw Error(j.error||txt||('HTTP '+r.status));return j}
function fmtUptime(s){s=Number(s||0);let d=Math.floor(s/86400),h=Math.floor((s%86400)/3600),m=Math.floor((s%3600)/60);return (d?d+'d ':'')+h+'h '+m+'m'}
async function loadState(){
 try{
  S=await api('/api/state');
  lang=S.settings.language||'de'; tr(); setTheme(S.settings.theme||'light');
  headerName.textContent=S.settings.device_name; headerMeta.textContent=(S.wifi.connected?S.wifi.ip:'AP '+S.wifi.ap_ip)+' · '+S.version;
  moisture.textContent=S.sensor.valid?S.sensor.moisture_percent.toFixed(1)+' %':'-- %';
  moistState.textContent=S.sensor.calibrated?(S.sensor.valid?'OK':'Warte auf Messung'):'Kalibrierung erforderlich';
  moistState.className='hint '+(S.sensor.calibrated?'ok':'warn');
  raw.textContent=S.sensor.valid?S.sensor.raw_adc:'--'; liveRaw.textContent=raw.textContent; pin.textContent=S.settings.signal_pin;
  rssi.textContent=S.wifi.connected?S.wifi.rssi+' dBm':'-- dBm'; ip.textContent=S.wifi.connected?S.wifi.ip:'AP: '+S.wifi.ap_ip;
  last.textContent=S.sensor.last_measurement_local||'--'; uptime.textContent='Uptime: '+fmtUptime(S.uptime_seconds);
  devName.textContent=S.settings.device_name; statusSensorName.textContent=S.settings.sensor_name; fw.textContent=S.version; ntp.textContent=S.time.valid?'OK':'wartet';
  sensorName.value=S.settings.sensor_name; signalPin.value=S.settings.signal_pin; interval.value=S.settings.measure_interval_seconds; samples.value=S.settings.sample_count;
  dryVal.textContent=S.settings.dry_adc; wetVal.textContent=S.settings.wet_adc;
  deviceName.value=S.settings.device_name; ssid.value=S.settings.ssid; ntpServer.value=S.settings.ntp_server; timezone.value=S.settings.timezone; language.value=S.settings.language; theme.value=S.settings.theme;
  otaCurrent.textContent=S.version; footer.textContent='ESP8266 Moisture Sensor '+S.version;
 }catch(e){headerMeta.textContent=e.message}
}
async function postForm(url,data){return api(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(data)})}
async function saveSensor(){try{await postForm('/api/settings/sensor',{sensor_name:sensorName.value,signal_pin:signalPin.value,interval:interval.value,samples:samples.value});await loadState();alert('OK')}catch(e){alert(e.message)}}
async function saveSystem(){try{await postForm('/api/settings/system',{device_name:deviceName.value,ssid:ssid.value,wifi_password:wifiPass.value,ntp_server:ntpServer.value,timezone:timezone.value,language:language.value,theme:theme.value});alert(lang==='de'?'Gespeichert. Neustart bei geänderten WLAN-Daten empfohlen.':'Saved. Reboot recommended after Wi-Fi changes.');await loadState()}catch(e){alert(e.message)}}
async function calibrate(which){try{await postForm('/api/calibration/'+which,{});await loadState()}catch(e){alert(e.message)}}
async function loadLog(){try{let r=await fetch('/api/log');logText.textContent=await r.text()}catch(e){logText.textContent=e.message}}
async function reboot(){if(confirm(lang==='de'?'ESP neu starten?':'Reboot ESP?')){await postForm('/api/reboot',{});alert('Reboot...')}}
async function factoryReset(){if(confirm(lang==='de'?'Wirklich alle Einstellungen löschen?':'Really erase all settings?')){await postForm('/api/factory-reset',{});alert('Reset...')}}
function semver(v){return String(v||'0').replace(/^v/,'').split('.').map(x=>parseInt(x,10)||0)}
function newer(a,b){let A=semver(a),B=semver(b);for(let i=0;i<3;i++){if((A[i]||0)>(B[i]||0))return true;if((A[i]||0)<(B[i]||0))return false}return false}
async function sha256Hex(buf){let h=await crypto.subtle.digest('SHA-256',buf);return Array.from(new Uint8Array(h)).map(x=>x.toString(16).padStart(2,'0')).join('')}
async function uploadBuf(buf){
 otaProgress.style.width='5%';
 let blob=new Blob([buf],{type:'application/octet-stream'});let fd=new FormData();fd.append('firmware',blob,'firmware.bin');
 let r=await fetch('/api/ota/upload',{method:'POST',body:fd});
 let t=await r.text();let j={};try{j=JSON.parse(t)}catch(e){}if(!r.ok)throw Error(j.error||t||'OTA failed');
 otaProgress.style.width='100%';otaMsg.textContent='Update OK – rebooting...'
}
async function checkOta(){
 try{
  otaMsg.textContent='Manifest...';let c=await api('/api/ota/config');let m=await (await fetch(c.manifest_url,{cache:'no-store'})).json();
  otaAvailable.textContent=m.version||'--';
  if(!newer(m.version,S.version)){otaMsg.textContent=lang==='de'?'Keine neuere Version verfügbar.':'No newer version available.';return}
  if(!confirm((lang==='de'?'Version ':'Version ')+m.version+(lang==='de'?' installieren?':' install?')))return;
  otaMsg.textContent='Firmware download...';let rr=await fetch(m.url,{cache:'no-store'});if(!rr.ok)throw Error('Firmware HTTP '+rr.status);let buf=await rr.arrayBuffer();
  if(m.size&&Number(m.size)!==buf.byteLength)throw Error('Firmware size mismatch');
  if(m.sha256){otaMsg.textContent='SHA-256...';let h=await sha256Hex(buf);if(h.toLowerCase()!==String(m.sha256).toLowerCase())throw Error('SHA-256 mismatch')}
  await uploadBuf(buf);
 }catch(e){otaMsg.textContent=e.message;otaMsg.className='hint bad'}
}
async function manualUpload(){try{let f=fwFile.files[0];if(!f)throw Error('Select firmware.bin');await uploadBuf(await f.arrayBuffer())}catch(e){otaMsg.textContent=e.message;otaMsg.className='hint bad'}}
loadState();setInterval(loadState,5000);
</script>
</body></html>
)HTML";
