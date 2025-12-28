#include "WebInterface.h"
#include "WeatherStation.h"
#include "WiFiManager.h"
#include "Thermostat.h"
#include "TemperatureSensor.h"
#include <ArduinoJson.h>

// Global instance
WebInterface webInterface;

// External references to system components
extern Thermostat thermostat;
extern TemperatureSensor tempSensor;
extern float cachedPeltierTemperature;
extern int dropCount;
extern WeatherStation stations[];
extern int NUM_STATIONS;  // Calculated from stations array size
extern int setpointMode;
extern float manualSetpoint;
extern WiFiManager wifiManager;

// Hardware status
extern bool hwStatusTempSensor;
extern bool hwStatusDropDetector;
extern bool hwStatusNeoPixel;
extern bool hwStatusAudioPlayer;
extern bool hwStatusWiFi;
extern bool hwStatusWebServer;

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
  <h1>❄️ Dripping Meteorite ❄️</h1>
  
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
      <input type="number" id="freeze-duration" value="900">
    </div>
    <div class="control-group">
      <label>Reactivate Timer (seconds)</label>
      <input type="number" id="reactivate-timer" value="1800">
    </div>
    <div class="control-group">
      <label>LED Fade Time (milliseconds)</label>
      <input type="number" id="led-fade-time" value="4000">
    </div>
    <div class="control-group">
      <label>LED Brightness (0-255)</label>
      <input type="number" min="0" max="255" id="led-brightness" value="255">
    </div>
    <button onclick="updateParameters()">Update Parameters</button>
    <button onclick="triggerDrop()">Test Drop</button>
  </div>
  
  <div class="card">
    <h2>Network</h2>
    <div id="network-info">Loading...</div>
  </div>
  
  <script>
    // Fetch and update status every 2 seconds
    function updateStatus() {
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
          
          // Update setpoint mode controls
          document.getElementById('setpoint-mode').value = data.setpointMode;
          document.getElementById('manual-setpoint').value = data.manualSetpoint.toFixed(1);
          toggleSetpointMode();  // Show/hide manual setpoint field
          
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
      params.append('freezeDuration', document.getElementById('freeze-duration').value);
      params.append('reactivateTimer', document.getElementById('reactivate-timer').value);
      params.append('ledFadeTime', document.getElementById('led-fade-time').value);
      params.append('ledBrightness', document.getElementById('led-brightness').value);
      
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
        .then(data => alert('Drop triggered!'))
        .catch(err => alert('Failed to trigger drop'));
    }
    
    // Update status every 2 seconds
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
  if (request->hasParam("reactivateTemp", true)) {
    float temp = request->getParam("reactivateTemp", true)->value().toFloat();
    thermostat.setReactivateTemp(temp);
  }
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}
