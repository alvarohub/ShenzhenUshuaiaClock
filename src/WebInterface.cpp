#include "WebInterface.h"
#include "WeatherStation.h"
#include "WiFiManager.h"
#include "Thermostat.h"
#include "TemperatureSensor.h"
#include "NeoPixelController.h"
#include "AudioPlayer.h"
#include "SettingsManager.h"
#include <ArduinoJson.h>

// Global instance
WebInterface webInterface;

// External references to system components
extern Thermostat thermostat;
extern TemperatureSensor tempSensor;
extern float cachedPeltierTemperature;
extern int dropCount;
extern WeatherStation stations[];
extern SettingsManager settingsManager;
extern int NUM_STATIONS;  // Calculated from stations array size
extern int setpointMode;
extern float manualSetpoint;
extern WiFiManager wifiManager;
extern NeoPixelController neoPixels;
extern AudioPlayer audioPlayer;

// Hardware status
extern bool hwStatusTempSensor;
extern bool hwStatusDropDetector;
extern bool hwStatusNeoPixel;
extern bool hwStatusAudioPlayer;
extern bool hwStatusWiFi;
extern bool hwStatusWebServer;

// System control
extern bool systemRunning;

// Generate main HTML page
String WebInterface::getHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Dripping Meteorite</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      max-width: 800px;
      margin: 20px auto;
      padding: 20px;
      background: #1a1a1a;
      color: #e0e0e0;
    }
    h1 {
      color: #4a9eff;
      text-align: center;
      border-bottom: 2px solid #4a9eff;
      padding-bottom: 10px;
    }
    .card {
      background: #2a2a2a;
      border-radius: 8px;
      padding: 20px;
      margin: 20px 0;
      box-shadow: 0 4px 6px rgba(0,0,0,0.3);
    }
    .status-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
      gap: 15px;
      margin: 20px 0;
    }
    .status-item {
      background: #333;
      padding: 15px;
      border-radius: 6px;
      border-left: 4px solid #4a9eff;
    }
    .status-label {
      color: #999;
      font-size: 0.9em;
      margin-bottom: 5px;
    }
    .status-value {
      font-size: 1.5em;
      font-weight: bold;
      color: #4a9eff;
    }
    .control-group {
      margin: 15px 0;
    }
    label {
      display: block;
      margin-bottom: 5px;
      color: #bbb;
    }
    input[type="number"], input[type="text"] {
      width: 100%;
      padding: 10px;
      background: #333;
      border: 1px solid #555;
      border-radius: 4px;
      color: #e0e0e0;
      font-size: 1em;
    }
    select {
      width: 100%;
      padding: 10px;
      background: #333;
      border: 1px solid #555;
      border-radius: 4px;
      color: #e0e0e0;
      font-size: 1em;
    }
    button {
      background: #4a9eff;
      color: white;
      border: none;
      padding: 12px 24px;
      border-radius: 4px;
      cursor: pointer;
      font-size: 1em;
      margin: 5px;
    }
    button:hover {
      background: #3a8edf;
    }
    button:active {
      background: #2a7ecf;
    }
    .weather-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
      gap: 10px;
    }
    .weather-item {
      background: #333;
      padding: 10px;
      border-radius: 4px;
      text-align: center;
    }
    .weather-name {
      font-weight: bold;
      color: #4a9eff;
      margin-bottom: 5px;
    }
    .on { color: #4eff4a; }
    .off { color: #ff4a4a; }
    .hw-status {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
      gap: 10px;
    }
    .hw-item {
      background: #333;
      padding: 10px;
      border-radius: 4px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .hw-ok { color: #4eff4a; font-weight: bold; }
    .hw-fail { color: #ff4a4a; font-weight: bold; }
    .hw-warn { color: #ffaa4a; font-weight: bold; }
  </style>
</head>
<body>
  <h1>❄️ Melting Glaciers Clock ❄️</h1>
  
  <div class="card" style="background: #2c3e50;">
    <h2 style="margin-bottom: 10px;">System Control</h2>
    <button id="system-toggle-btn" onclick="toggleSystem()" style="font-size: 1.2em; padding: 15px 30px;">SYSTEM: RUNNING</button>
    <p style="font-size: 0.9em; color: #bbb; margin-top: 10px;">Pause/resume thermostat and sensor updates</p>
  </div>
  
  <div class="card">
    <h2>Hardware Status</h2>
    <div class="hw-status">
      <div class="hw-item">
        <span>Temperature Sensor</span>
        <span id="hw-temp" class="hw-ok">OK</span>
      </div>
      <div class="hw-item">
        <span>Drop Detector</span>
        <span id="hw-drop" class="hw-ok">OK</span>
      </div>
      <div class="hw-item">
        <span>NeoPixels</span>
        <span id="hw-neo" class="hw-ok">OK</span>
      </div>
      <div class="hw-item">
        <span>Audio Player</span>
        <span id="hw-audio" class="hw-ok">OK</span>
      </div>
      <div class="hw-item">
        <span>WiFi Station</span>
        <span id="hw-wifi" class="hw-warn">N/A</span>
      </div>
      <div class="hw-item">
        <span>Web Server</span>
        <span id="hw-web" class="hw-ok">OK</span>
      </div>
    </div>
  </div>
  
  <div class="card">
    <h2>System Status</h2>
    <div class="status-grid">
      <div class="status-item">
        <div class="status-label">Thermostat</div>
        <div class="status-value" id="thermostat-state">--</div>
      </div>
      <div class="status-item">
        <div class="status-label">Peltier Temp</div>
        <div class="status-value" id="peltier-temp">--°C</div>
      </div>
      <div class="status-item">
        <div class="status-label">Setpoint</div>
        <div class="status-value" id="setpoint">--°C</div>
      </div>
      <div class="status-item">
        <div class="status-label">Drop Count</div>
        <div class="status-value" id="drop-count">--</div>
      </div>
    </div>
  </div>
  
  <div class="card">
    <h2>Weather Stations</h2>
    <div class="weather-grid" id="weather-grid">
      <!-- Populated by JavaScript -->
    </div>
  </div>
  
  <div class="card">
    <h2>Control Parameters</h2>
    <div class="control-group">
      <label>Setpoint Mode</label>
      <select id="setpoint-mode" onchange="toggleSetpointMode()">
        <option value="-1">Manual</option>
        <option value="0">Link to Ilulissat</option>
        <option value="1">Link to El Calafate</option>
        <option value="2">Link to Hong Kong</option>
        <option value="3">Link to Shenzhen</option>
      </select>
    </div>
    <div class="control-group" id="manual-setpoint-group">
      <label>Manual Setpoint (°C)</label>
      <input type="number" step="0.5" id="manual-setpoint" value="-3.0">
    </div>
    <div class="control-group">
      <label>Reactivate Temperature (°C)</label>
      <input type="number" step="0.5" id="reactivate-temp" value="20.0">
    </div>
    <div class="control-group">
      <label>Freezing Duration (seconds)</label>
      <input type="number" step="1" id="freeze-duration" value="10">
    </div>
    <div class="control-group">
      <label>Reactivate Timer (minutes)</label>
      <input type="number" step="1" id="reactivate-timer" value="30">
    </div>
    <div class="control-group">
      <label>LED Fade Time (seconds)</label>
      <input type="number" step="0.1" id="led-fade-time" value="4">
    </div>
    <div class="control-group">
      <label>LED Brightness (0-255)</label>
      <input type="number" min="0" max="255" id="led-brightness" value="255">
    </div>
    <div class="control-group">
      <label>Cube Light (Ambient Glow)</label>
      <select id="cube-light">
        <option value="1">ON</option>
        <option value="0">OFF</option>
      </select>
    </div>
    <div>
      <label>Cube Light Brightness (0-255)</label>
      <input type="number" id="cube-brightness" min="0" max="255" step="1">
    </div>
    <button onclick="updateParameters()">Update Parameters</button>
    <button onclick="triggerDrop()">Test Drop</button>
  </div>
  
  <div class="card">
    <h2>Manual Tests</h2>
    <button id="peltier-toggle-btn" onclick="togglePeltier()">Peltier: OFF</button>
    <button onclick="testLED()">Test LED (5sec)</button>
    <button onclick="testAudio()">Test Audio</button>
  </div>
  
  <div class="card">
    <h2>Settings</h2>
    <button onclick="resetToDefaults()" style="background-color: #d9534f;">Reset to Defaults</button>
    <p style="font-size: 0.9em; color: #666;">Restores all settings from config.h</p>
  </div>
  
  <div class="card">
    <h2>Network</h2>
    <div id="network-info">Loading...</div>
  </div>
  
  <script>
    // Track if user is editing any input field
    let userIsEditing = false;
    let editTimeout = null;
    
    // Pause auto-refresh when user focuses on input fields
    document.addEventListener('focusin', (e) => {
      if (e.target.tagName === 'INPUT' || e.target.tagName === 'SELECT') {
        userIsEditing = true;
        clearTimeout(editTimeout);
      }
    });
    
    // Resume auto-refresh 3 seconds after user stops editing
    document.addEventListener('focusout', (e) => {
      if (e.target.tagName === 'INPUT' || e.target.tagName === 'SELECT') {
        clearTimeout(editTimeout);
        editTimeout = setTimeout(() => {
          userIsEditing = false;
        }, 3000);
      }
    });
    
    // Also pause when user types
    document.addEventListener('input', (e) => {
      if (e.target.tagName === 'INPUT') {
        userIsEditing = true;
        clearTimeout(editTimeout);
        editTimeout = setTimeout(() => {
          userIsEditing = false;
        }, 3000);
      }
    });
    
    // Fetch and update status every 2 seconds
    function updateStatus() {
      // Skip update if user is editing
      if (userIsEditing) {
        return;
      }
      
      fetch('/api/status')
        .then(response => response.json())
        .then(data => {
          // Update hardware status
          updateHWStatus('hw-temp', data.hardware.tempSensor);
          updateHWStatus('hw-drop', data.hardware.dropDetector);
          updateHWStatus('hw-neo', data.hardware.neoPixel);
          updateHWStatus('hw-audio', data.hardware.audioPlayer);
          updateHWStatus('hw-wifi', data.hardware.wifi);
          updateHWStatus('hw-web', data.hardware.webServer);
          
          // Update system status
          document.getElementById('thermostat-state').innerHTML = 
            data.thermostat.cooling ? '<span class="on">COOLING</span>' : '<span class="off">OFF</span>';
          document.getElementById('peltier-temp').textContent = data.peltierTemp.toFixed(1) + '°C';
          document.getElementById('setpoint').textContent = data.thermostat.setpoint.toFixed(1) + '°C';
          document.getElementById('drop-count').textContent = data.dropCount;
          
          // Update setpoint mode controls (only if not currently being edited)
          const setpointModeEl = document.getElementById('setpoint-mode');
          const manualSetpointEl = document.getElementById('manual-setpoint');
          if (document.activeElement !== setpointModeEl) {
            setpointModeEl.value = data.setpointMode;
          }
          if (document.activeElement !== manualSetpointEl) {
            manualSetpointEl.value = data.manualSetpoint.toFixed(1);
          }
          toggleSetpointMode();  // Show/hide manual setpoint field
          
          // Update settings fields (only if not being edited)
          const freezeDurationEl = document.getElementById('freeze-duration');
          const reactivateTimerEl = document.getElementById('reactivate-timer');
          const ledFadeTimeEl = document.getElementById('led-fade-time');
          const ledBrightnessEl = document.getElementById('led-brightness');
          const reactivateTempEl = document.getElementById('reactivate-temp');
          const cubeLightEl = document.getElementById('cube-light');
          const cubeBrightnessEl = document.getElementById('cube-brightness');
          
          if (document.activeElement !== freezeDurationEl) {
            freezeDurationEl.value = data.settings.freezeDurationSec.toFixed(0);
          }
          if (document.activeElement !== reactivateTimerEl) {
            reactivateTimerEl.value = data.settings.reactivateTimerMin.toFixed(0);
          }
          if (document.activeElement !== ledFadeTimeEl) {
            ledFadeTimeEl.value = data.settings.ledFadeTimeSec.toFixed(1);
          }
          if (document.activeElement !== ledBrightnessEl) {
            ledBrightnessEl.value = data.settings.ledBrightness;
          }
          if (document.activeElement !== reactivateTempEl) {
            reactivateTempEl.value = data.thermostat.reactivateTemp.toFixed(1);
          }
          if (document.activeElement !== cubeLightEl) {
            cubeLightEl.value = data.settings.cubeLight ? '1' : '0';
          }
          if (document.activeElement !== cubeBrightnessEl) {
            cubeBrightnessEl.value = data.settings.cubeLightBrightness;
          }
          
          // Update Peltier button state
          const peltierBtn = document.getElementById('peltier-toggle-btn');
          peltierBtn.textContent = data.thermostat.cooling ? 'Peltier: ON (Force OFF)' : 'Peltier: OFF (Force ON)';
          peltierBtn.style.backgroundColor = data.thermostat.cooling ? '#5cb85c' : '#d9534f';
          
          // Update weather stations
          let weatherHTML = '';
          data.weather.forEach(station => {
            weatherHTML += `
              <div class="weather-item">
                <div class="weather-name">${station.name}</div>
                <div>${station.temp.toFixed(1)}°C</div>
                <div>${station.humidity.toFixed(0)}%</div>
              </div>
            `;
          });
          document.getElementById('weather-grid').innerHTML = weatherHTML;
          
          // Update network info
          document.getElementById('network-info').innerHTML = 
            `<strong>AP:</strong> ${data.network.apSSID} @ ${data.network.apIP}<br>` +
            (data.network.stationConnected ? 
              `<strong>WiFi:</strong> ${data.network.stationIP}` : 
              `<strong>WiFi:</strong> Not connected`);
        })
        .catch(err => console.error('Status update failed:', err));
    }
    
    function updateHWStatus(elementId, status) {
      const elem = document.getElementById(elementId);
      elem.className = status ? 'hw-ok' : 'hw-fail';
      elem.textContent = status ? 'OK' : 'FAIL';
    }
    
    function toggleSetpointMode() {
      const mode = parseInt(document.getElementById('setpoint-mode').value);
      const manualGroup = document.getElementById('manual-setpoint-group');
      // Show manual setpoint input only when in manual mode
      manualGroup.style.display = (mode === -1) ? 'block' : 'none';
    }
    
    function updateParameters() {
      const params = new URLSearchParams();
      params.append('setpointMode', document.getElementById('setpoint-mode').value);
      params.append('manualSetpoint', document.getElementById('manual-setpoint').value);
      params.append('reactivateTemp', document.getElementById('reactivate-temp').value);
      // Convert to milliseconds
      params.append('freezeDuration', document.getElementById('freeze-duration').value * 1000);  // seconds to ms
      params.append('reactivateTimer', document.getElementById('reactivate-timer').value * 60000);  // minutes to ms
      // Convert seconds to milliseconds
      params.append('ledFadeTime', document.getElementById('led-fade-time').value * 1000);
      params.append('ledBrightness', document.getElementById('led-brightness').value);
      params.append('cubeLight', document.getElementById('cube-light').value);
      params.append('cubeLightBrightness', document.getElementById('cube-brightness').value);
      
      fetch('/api/update', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: params
      })
      .then(response => response.json())
      .then(data => {
        alert(data.message || 'Parameters updated!');
        updateStatus();  // Refresh status immediately
      })
      .catch(err => alert('Update failed: ' + err));
    }
    
    function triggerDrop() {
      fetch('/api/drop', {method: 'POST'})
        .then(response => response.json())
        .then(data => updateStatus())
        .catch(err => console.error('Failed to trigger drop:', err));
    }
    
    function togglePeltier() {
      fetch('/api/peltier/toggle', {method: 'POST'})
        .then(response => response.json())
        .then(data => {
          console.log('Peltier toggled:', data.message);
        })
        .catch(err => console.error('Peltier toggle failed:', err));
    }
    
    function toggleSystem() {
      fetch('/api/system/toggle', {method: 'POST'})
        .then(response => response.json())
        .then(data => {
          const btn = document.getElementById('system-toggle-btn');
          btn.textContent = data.running ? 'SYSTEM: RUNNING' : 'SYSTEM: PAUSED';
          btn.style.backgroundColor = data.running ? '#5cb85c' : '#d9534f';
        })
        .catch(err => console.error('System toggle failed:', err));
    }
    
    function testLED() {
      fetch('/api/test/led', {method: 'POST'})
        .then(response => response.json())
        .then(data => updateStatus())
        .catch(err => console.error('LED test failed:', err));
    }
    
    function testAudio() {
      fetch('/api/test/audio', {method: 'POST'})
        .then(response => response.json())
        .then(data => updateStatus())
        .catch(err => console.error('Audio test failed:', err));
    }
    
    function resetToDefaults() {
      if (confirm('Reset all settings to defaults from config.h? This will restart the device.')) {
        fetch('/api/reset', { method: 'POST' })
          .then(response => response.json())
          .then(data => {
            alert(data.message);
            setTimeout(() => { location.reload(); }, 3000);
          })
          .catch(error => console.error('Error:', error));
      }
    }
    
    // Start auto-refresh
    updateStatus();
    setInterval(updateStatus, 2000);
  </script>
</body>
</html>
)rawliteral";
}

// Handle status API endpoint - return JSON with current system state
void WebInterface::handleStatus(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  // Thermostat status
  doc["thermostat"]["cooling"] = thermostat.isCooling();
  doc["thermostat"]["setpoint"] = thermostat.getSetPoint();
  doc["thermostat"]["reactivateTemp"] = thermostat.getReactivateTemp();
  
  // Temperature and drops
  doc["peltierTemp"] = cachedPeltierTemperature;
  doc["dropCount"] = dropCount;
  
  // Setpoint mode
  doc["setpointMode"] = setpointMode;
  doc["manualSetpoint"] = manualSetpoint;
  
  // Hardware status
  doc["hardware"]["tempSensor"] = hwStatusTempSensor;
  doc["hardware"]["dropDetector"] = hwStatusDropDetector;
  doc["hardware"]["neoPixel"] = hwStatusNeoPixel;
  doc["hardware"]["audioPlayer"] = hwStatusAudioPlayer;
  doc["hardware"]["wifi"] = hwStatusWiFi;
  doc["hardware"]["webServer"] = hwStatusWebServer;
  
  // Weather stations
  JsonArray weatherArray = doc["weather"].to<JsonArray>();
  for (int i = 0; i < NUM_STATIONS; i++) {
    JsonObject station = weatherArray.add<JsonObject>();
    station["name"] = stations[i].name;
    station["temp"] = stations[i].temperature;
    station["humidity"] = stations[i].humidity;
  }
  
  // Network info
  doc["network"]["apSSID"] = apSSID;
  doc["network"]["apIP"] = apIP.toString();
  doc["network"]["stationConnected"] = (WiFi.status() == WL_CONNECTED);
  doc["network"]["stationIP"] = WiFi.localIP().toString();
  
  // Settings (with unit conversions for display)
  doc["settings"]["freezeDurationSec"] = settingsManager.currentSettings.durationGlacierFreezing / 1000.0;  // ms to seconds
  doc["settings"]["reactivateTimerMin"] = settingsManager.currentSettings.reactivateTimer / 60000.0;  // ms to minutes
  doc["settings"]["ledFadeTimeSec"] = settingsManager.currentSettings.ledFadeTotalTime / 1000.0;  // ms to seconds
  doc["settings"]["ledBrightness"] = settingsManager.currentSettings.neopixelBrightness;
  doc["settings"]["cubeLight"] = settingsManager.currentSettings.cubeLight;
  doc["settings"]["cubeLightBrightness"] = settingsManager.currentSettings.cubeLightBrightness;
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

// Handle parameter update API endpoint
void WebInterface::handleUpdate(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  // Parse request body (we'll get data from POST parameters for simplicity)
  if (request->hasParam("setpointMode", true)) {
    int newMode = request->getParam("setpointMode", true)->value().toInt();
    
    // Handle setpoint mode change
    if (newMode == -1) {
      // Switching to manual mode
      if (request->hasParam("manualSetpoint", true)) {
        manualSetpoint = request->getParam("manualSetpoint", true)->value().toFloat();
        thermostat.setSetPoint(manualSetpoint);
      }
      setpointMode = -1;
      doc["status"] = "ok";
      doc["message"] = "Switched to manual mode";
      
    } else if (newMode >= 0 && newMode < NUM_STATIONS) {
      // Switching to station-linked mode
      setpointMode = newMode;
      
      // Enable WiFi and try to connect
      if (!wifiManager.isConnected()) {
        wifiManager.begin();  // Enable WiFi
        if (wifiManager.connect()) {
          // Successfully connected - fetch weather data
          wifiManager.fetchWeather(stations, NUM_STATIONS);
          manualSetpoint = stations[setpointMode].temperature;
          thermostat.setSetPoint(manualSetpoint);
          doc["status"] = "ok";
          doc["message"] = "Connected to WiFi and linked to station";
        } else {
          // Connection failed - revert to manual
          setpointMode = -1;
          doc["status"] = "error";
          doc["message"] = "WiFi connection failed, staying in manual mode";
        }
      } else {
        // Already connected - just update setpoint
        wifiManager.fetchWeather(stations, NUM_STATIONS);
        manualSetpoint = stations[setpointMode].temperature;
        thermostat.setSetPoint(manualSetpoint);
        doc["status"] = "ok";
        doc["message"] = "Linked to station";
      }
    }
  }
  
  // Update other parameters if provided
  bool settingsChanged = false;
  
  if (request->hasParam("reactivateTemp", true)) {
    float temp = request->getParam("reactivateTemp", true)->value().toFloat();
    thermostat.setReactivateTemp(temp);
    settingsManager.currentSettings.reactivateTemp = temp;
    settingsChanged = true;
  }
  
  if (request->hasParam("manualSetpoint", true)) {
    manualSetpoint = request->getParam("manualSetpoint", true)->value().toFloat();
    settingsManager.currentSettings.manualSetpoint = manualSetpoint;
    // Update thermostat if in manual mode
    if (setpointMode == -1) {
      thermostat.setSetPoint(manualSetpoint);
    }
    settingsChanged = true;
  }
  
  if (request->hasParam("freezeDuration", true)) {
    unsigned long duration = request->getParam("freezeDuration", true)->value().toInt();
    settingsManager.currentSettings.durationGlacierFreezing = duration;
    settingsChanged = true;
  }
  
  if (request->hasParam("reactivateTimer", true)) {
    unsigned long timer = request->getParam("reactivateTimer", true)->value().toInt();
    settingsManager.currentSettings.reactivateTimer = timer;
    settingsChanged = true;
  }
  
  if (request->hasParam("ledFadeTime", true)) {
    uint16_t fadeTime = request->getParam("ledFadeTime", true)->value().toInt();
    settingsManager.currentSettings.ledFadeTotalTime = fadeTime;
    // TODO: Update NeoPixelController fade time if needed
    settingsChanged = true;
  }
  
  if (request->hasParam("ledBrightness", true)) {
    uint8_t brightness = request->getParam("ledBrightness", true)->value().toInt();
    settingsManager.currentSettings.neopixelBrightness = brightness;
    // TODO: Update NeoPixelController brightness if needed
    settingsChanged = true;
  }
  
  if (request->hasParam("cubeLight", true)) {
    bool cubeLight = request->getParam("cubeLight", true)->value().toInt();
    settingsManager.currentSettings.cubeLight = cubeLight;
    settingsChanged = true;
  }
  
  if (request->hasParam("cubeLightBrightness", true)) {
    uint8_t cubeBrightness = request->getParam("cubeLightBrightness", true)->value().toInt();
    settingsManager.currentSettings.cubeLightBrightness = cubeBrightness;
    settingsChanged = true;
  }
  
  // Save to EEPROM if any setting changed
  if (settingsChanged) {
    settingsManager.saveToEEPROM();
    Serial.println("Settings updated and saved to EEPROM");
  }
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

// Handle drop trigger API endpoint
void WebInterface::handleDrop(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  // Simulate drop detection (same as physical button)
  dropCount++; // Increment drop counter
  
  // 1. Trigger LED fade cycle
  neoPixels.onDropDetected(cachedPeltierTemperature, thermostat.getSetPoint());
  
  // 2. Play audio sample
  audioPlayer.playDropSound();
  
  // 3. Force peltier to reactivate immediately
  thermostat.forceActivate();
  
  doc["status"] = "ok";
  doc["message"] = "Drop triggered!";
  doc["dropCount"] = dropCount;
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

// Handle Peltier toggle - force ON or OFF (without breaking thermostat logic)
void WebInterface::handleTogglePeltier(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  if (thermostat.isCooling()) {
    // Currently ON - turn it OFF
    thermostat.turnOff();
    doc["message"] = "Peltier turned OFF (will restart based on thermostat logic)";
  } else {
    // Currently OFF - force it ON for 5 seconds
    thermostat.forceActivate();
    doc["message"] = "Peltier forced ON for 5 seconds";
  }
  
  doc["status"] = "ok";
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

// Handle Peltier test - force ON for 5 seconds
void WebInterface::handleTestPeltier(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  // Force peltier ON for 5 seconds (thermostat will override after that)
  thermostat.forceActivate();
  
  doc["status"] = "ok";
  doc["message"] = "Peltier forced ON for 5 seconds";
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

// Handle LED test - full white for 5 seconds
void WebInterface::handleTestLED(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  // Set LEDs to full white
  neoPixels.fill(255, 255, 255);
  neoPixels.show();
  
  // Note: LEDs will stay white until next update or fade
  // User can trigger another action to change them
  
  doc["status"] = "ok";
  doc["message"] = "LED test running (5 seconds)";
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

// Handle Audio test - play drop sound
void WebInterface::handleTestAudio(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  // Play audio sample
  audioPlayer.playDropSound();
  
  doc["status"] = "ok";
  doc["message"] = "Audio playing";
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

// Handle Reset to Defaults - restore all settings from config.h
void WebInterface::handleReset(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  Serial.println("Resetting to defaults...");
  settingsManager.resetToDefaults();
  
  doc["status"] = "ok";
  doc["message"] = "Settings reset to defaults. Device will restart in 3 seconds...";
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
  
  // Restart device after a short delay
  delay(1000);
  ESP.restart();
}

// Handle System Toggle - pause/resume thermostat and sensor updates
void WebInterface::handleToggleSystem(AsyncWebServerRequest *request) {
  JsonDocument doc;
  
  systemRunning = !systemRunning;  // Toggle the flag
  
  doc["status"] = "ok";
  doc["running"] = systemRunning;
  doc["message"] = systemRunning ? "System RESUMED" : "System PAUSED";
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}
