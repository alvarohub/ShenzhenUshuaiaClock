#include <M5Unified.h>
#include "config.h"
#include "WeatherStation.h"
#include "WeatherStationData.h"
#include "WiFiManager.h"
#include "TemperatureSensor.h"
#include "NeoPixelController.h"
#include "DropDetector.h"
#include "Thermostat.h"
#include "AudioPlayer.h"
#include "WebInterface.h"
#include "SettingsManager.h"

// ============================================
// DEBUG FLAGS - Set to true to enable testing
// ============================================
#define GLOBAL_DEBUG false  // When TRUE: only init components with debug=true, stop on fail
                            // When FALSE: init all components, stop on fail

#define DEBUG_DROP_SENSOR false
#define DEBUG_TEMPERATURE false
#define DEBUG_NEOPIXEL false
#define DEBUG_AUDIO_PLAYER false
#define DEBUG_THERMOSTAT false

// Global variables
int glacierIndex = 0;      // Which glacier to display (0 or 1)

// Station references for clarity
WeatherStation& currentGlacierStation = stations[0];  // Can be changed via glacierIndex
WeatherStation& localStation = stations[LOCAL_SHENZHEN];

// Drop counter
int dropCount = 0;

// Setpoint mode: -1 = manual, 0-3 = linked to station index
int setpointMode = -1;  // Start in manual mode
float manualSetpoint = MANUAL_SETPOINT;  // Current setpoint value

// System control
bool systemRunning = true;  // Global flag to pause/resume system updates

// Hardware status tracking
bool hwStatusTempSensor = false;
bool hwStatusDropDetector = false;
bool hwStatusNeoPixel = false;
bool hwStatusAudioPlayer = false;
bool hwStatusWiFi = false;
bool hwStatusWebServer = false;

// Display update timing (non-blocking)
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 500; // Update display every 500ms

// Temperature sensor timing (non-blocking)
unsigned long lastTempRead = 0;
float cachedPeltierTemperature = 20.0; // Cached temperature value

// External references to system components
extern SettingsManager settingsManager;

// Forward declarations
void displayWeather(WeatherStation& station, int x, int y, int width, int height);
void displayWeatherStations(float peltierTemperature);
void displaySystemStatus(float peltierTemperature);

void setup() {

   delay(2000); // Wait for system to stabilize (power surges, etc.)

  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);
  
  M5.Display.setRotation(1);
  M5.Display.setTextSize(1.2);
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextColor(GREEN);
  
  int y = 5;
  M5.Display.setCursor(5, y); y += 15;
  M5.Display.println("Initializing...");
  
  // Initialize settings from EEPROM
  M5.Display.setCursor(5, y); y += 12;
  M5.Display.print("Settings...");
  if (settingsManager.begin()) {
    M5.Display.println("OK");
    // Load settings into global variables
    manualSetpoint = settingsManager.currentSettings.manualSetpoint;
  } else {
    M5.Display.setTextColor(RED);
    M5.Display.println("FAIL");
    M5.Display.setTextColor(GREEN);
  }
  
  // Track hardware initialization status
  bool hardwareOK = true;
  
 // INITIALIZATION SEQUENCE:
 // (Temperature sensor, drop detector, NeoPixels, WiFi, Audio Player)
  
  #if GLOBAL_DEBUG
    // DEBUG MODE: Only initialize components being tested
    M5.Display.setTextColor(YELLOW);
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.println("DEBUG MODE");
    M5.Display.setTextColor(GREEN);
  #endif
  
  // Temperature Sensor
  #if !GLOBAL_DEBUG || DEBUG_TEMPERATURE
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("1.TempSens.");
    hwStatusTempSensor = tempSensor.begin();
    if (hwStatusTempSensor) {
      M5.Display.println("OK");
    } else {
      M5.Display.setTextColor(RED);
      M5.Display.println("FAIL");
      M5.Display.setTextColor(GREEN);
      // Continue anyway - graceful degradation
      hardwareOK = false;
    }
  #else
    hwStatusTempSensor = true;  // Assume OK if not tested
  #endif
  
  // Drop Detector
  #if !GLOBAL_DEBUG || DEBUG_DROP_SENSOR
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("2.DropSens.");
    dropDetector.begin();
    hwStatusDropDetector = true;
    M5.Display.println("OK");
  #else
    hwStatusDropDetector = true;  // Assume OK if not tested
  #endif
  
  // NeoPixels
  #if !GLOBAL_DEBUG || DEBUG_NEOPIXEL
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("3.NeoPix...");
    hwStatusNeoPixel = neoPixels.begin();
    if (hwStatusNeoPixel) {
      neoPixels.clear();
      neoPixels.show();
      M5.Display.println("OK");
    } else {
      M5.Display.setTextColor(RED);
      M5.Display.println("FAIL");
      M5.Display.setTextColor(GREEN);
      // Continue anyway - graceful degradation
      hardwareOK = false;
    }
  #else
    hwStatusNeoPixel = true;  // Assume OK if not tested
  #endif
  
  // WiFi
  #if !GLOBAL_DEBUG
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("4.WiFi.....");
    wifiManager.begin();
    if (wifiManager.isEnabled()) {
      hwStatusWiFi = wifiManager.connect();
      if (hwStatusWiFi) {
        M5.Display.println("OK");
        wifiManager.fetchWeather(stations, NUM_STATIONS);
      } else {
        M5.Display.setTextColor(YELLOW);
        M5.Display.println("FAIL");
        M5.Display.setTextColor(GREEN);
      }
    } else {
      hwStatusWiFi = false;
      M5.Display.setTextColor(YELLOW);
      M5.Display.println("DIS");
      M5.Display.setTextColor(GREEN);
    }
  #else
    hwStatusWiFi = false;  // Not tested in debug mode
  #endif
  
  // Audio Player
  #if !GLOBAL_DEBUG || DEBUG_AUDIO_PLAYER
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("5.AudioPlayer.");
    hwStatusAudioPlayer = audioPlayer.begin();
    if (hwStatusAudioPlayer) {
      M5.Display.println("OK");
    } else {
      M5.Display.setTextColor(RED);
      M5.Display.println("FAIL");
      M5.Display.setTextColor(GREEN);
      // Continue anyway - graceful degradation
      hardwareOK = false;
    }
  #else
    hwStatusAudioPlayer = true;  // Assume OK if not tested
  #endif
  
  // Thermostat
  #if !GLOBAL_DEBUG || DEBUG_THERMOSTAT
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("6.Thermostat..");
    thermostat.begin();
    M5.Display.println("OK");
  #endif
  
  // Web Server (always start in AP mode)
  #if !GLOBAL_DEBUG
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("7.WebServer...");
    hwStatusWebServer = webInterface.begin(WEBSERVER_ENABLED);
    if (hwStatusWebServer) {
      M5.Display.println("OK");
    } else {
      M5.Display.setTextColor(YELLOW);
      M5.Display.println("FAIL");
      M5.Display.setTextColor(GREEN);
    }
  #else
    hwStatusWebServer = false;  // Not tested in debug mode
  #endif
  
  M5.Display.setCursor(5, y); y += 15;
  M5.Display.setTextSize(1.4);
  
  // Display hardware status summary
  if (!hardwareOK) {
    M5.Display.setTextColor(YELLOW);
    M5.Display.println("Some HW failed");
    M5.Display.setTextColor(WHITE);
    M5.Display.println("Running anyway!");
    delay(2000);
  }
  
  M5.Display.setTextColor(WHITE);
  M5.Display.println("ALL READY!");
  
  // LED startup sequence
  neoPixels.startSystem();
  
  // ==================================================
  // PROGRAM INITIALIZATION
  // ==================================================
  
  // Set thermostat setpoint to manual default (not linked to station yet)
  thermostat.setSetPoint(manualSetpoint);
  
  // Set reactivate temperature from config
  thermostat.setReactivateTemp(REACTIVATE_TEMP);
  
  // Start cooling immediately
  thermostat.turnOn();
  
  // Display web server access info
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextSize(1.2);
  M5.Display.setTextColor(CYAN);
  M5.Display.setCursor(5, 5);
  M5.Display.println("Web Interface:");
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(5, 25);
  M5.Display.print("AP: ");
  M5.Display.println(webInterface.getAPSSID());
  M5.Display.setCursor(5, 40);
  M5.Display.print("IP: ");
  M5.Display.println(webInterface.getAPIP().toString());
  M5.Display.setCursor(5, 55);
  M5.Display.setTextColor(YELLOW);
  M5.Display.println("Pass: " + String(WEBSERVER_AP_PASSWORD));
  
  delay(3000);
  
  // M5.Display.setTextSize(2);
  // M5.Display.fillScreen(BLACK);
  // M5.Display.setTextColor(WHITE);
  
  // // Display initial message
  // M5.Display.setCursor(10, 10);
  // M5.Display.println("AtomS3");
  // M5.Display.setCursor(10, 30);
  // M5.Display.println("Glacier");
  // M5.Display.setCursor(10, 40);
  // M5.Display.println("Monitor");
  
  // ============================================
  // DEBUG TEST MODES - Enter if flag is enabled
  // ============================================
  #if DEBUG_DROP_SENSOR
    dropDetector.testMode();  // Never returns
  #endif
  
  #if DEBUG_TEMPERATURE
    tempSensor.testMode();  // Never returns
  #endif
  
  #if DEBUG_NEOPIXEL
    neoPixels.testMode();  // Never returns
  #endif
  
  #if DEBUG_THERMOSTAT
    thermostat.begin();
    thermostat.testMode();  // Never returns
  #endif
  
  #if DEBUG_AUDIO_PLAYER
    audioPlayer.testMode();  // Never returns
  #endif
}

void loop() {
  M5.update();
  
  // ==================================================
  // MAIN PROGRAM FLOW
  // ==================================================
  
  // Only run system updates if not paused
  if (systemRunning) {
    // Read peltier/ice temperature from Dallas sensor (only every TEMP_READ_INTERVAL)
    if (millis() - lastTempRead >= TEMP_READ_INTERVAL) {
      cachedPeltierTemperature = tempSensor.readTemperature();
      lastTempRead = millis();
    }
    
    // Update thermostat with current temperature
    thermostat.setCurrentTemp(cachedPeltierTemperature);
    
    // Run thermostat control logic
    thermostat.update();
    
    // Check for drop detection (update returns true when drop detected with debouncing)
    if (dropDetector.update()) {
      // Drop detected! 
      dropCount++; // Increment drop counter
      
      // 1. Trigger LED fade cycle (full white, then fade to black as it cools)
      neoPixels.onDropDetected(cachedPeltierTemperature, thermostat.getSetPoint());
      
      // 2. Play audio sample
      audioPlayer.playDropSound();
      
      // 3. Force peltier to reactivate immediately
      thermostat.forceActivate();
    }
  }
  
  // Update LED brightness based on timer (always, even when paused)
  neoPixels.updateTimerFade();
  
  // Update ambient cube lighting (blue pulse when cooling, red glow when off)
  neoPixels.updateAmbientLight(thermostat.isCooling(), settingsManager.currentSettings.cubeLight, settingsManager.currentSettings.cubeLightBrightness);
  
  // ==================================================
  // PERIODIC WIFI UPDATES
  // ==================================================

  // Periodic WiFi reconnection attempt if enabled but not connected
  if (wifiManager.shouldRetry()) {
    wifiManager.connect();
  }
  
  // Update weather every 5 minutes (only if WiFi connected)
  if (wifiManager.shouldUpdate()) {
    wifiManager.fetchWeather(stations, NUM_STATIONS);
    // Update thermostat setpoint only if linked to a station
    if (setpointMode >= 0 && setpointMode < NUM_STATIONS) {
      manualSetpoint = stations[setpointMode].temperature;
      thermostat.setSetPoint(manualSetpoint);
    }
  }
  
  // ==================================================
  // DISPLAY UPDATE
  // ==================================================
  
  // Update display only periodically (non-blocking)
  // Skip display updates during LED fade for smooth animation
  if (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL && !neoPixels.isFading()) {
    lastDisplayUpdate = millis();
    displaySystemStatus(cachedPeltierTemperature);
  }
  
  // LCD button (BtnA - button under the display) simulates drop event
  if (M5.BtnA.wasPressed()) {
    // Simulate drop detection!
    dropCount++; // Increment drop counter
    
    // 1. Trigger LED fade cycle (full white, then fade to black as it cools)
    neoPixels.onDropDetected(cachedPeltierTemperature, thermostat.getSetPoint());
    
    // 2. Play audio sample
    audioPlayer.playDropSound();
    
    // 3. Force peltier to reactivate immediately
    thermostat.forceActivate();
  }
  
  // COMMENTED: WiFi disable functionality
  // The LCD button now only enables/connects WiFi or refreshes
  // To disable WiFi, set WIFI_ENABLED to false in config.h
  
  // COMMENTED: Original glacier cycling functionality
  // Uncomment to cycle through glacier locations and update reference
  // if (M5.BtnA.wasPressed()) {
  //   glacierIndex = (glacierIndex + 1) % (NUM_STATIONS - 1);
  //   currentGlacierStation = stations[glacierIndex];
  // }
  
  //delay(1000);
}

void displayWeather(WeatherStation& station, int x, int y, int width, int height) {
  // Draw rectangle border
  M5.Display.drawRect(x, y, width, height, WHITE);
  
  // Location name
  M5.Display.setTextSize(1);
  M5.Display.setCursor(x + 3, y + 3);
  M5.Display.setTextColor(WHITE);
  M5.Display.println(station.name);
  
  // Temperature
  M5.Display.setCursor(x + 3, y + 17);
  M5.Display.print("T:");
  M5.Display.setTextColor(CYAN);
  M5.Display.printf("%.1fC", station.temperature);
  
  // Humidity
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(x + 3, y + 32);
  M5.Display.print("H:");
  M5.Display.setTextColor(GREEN);
  M5.Display.printf("%.0f%%", station.humidity);
  
  // Dew Point
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(x + 3, y + 47);
  M5.Display.print("D:");
  M5.Display.setTextColor(ORANGE);
  M5.Display.printf("%.1fC", station.dewPoint);
}

void displayWeatherStations(float peltierTemperature) {
  // OLD DISPLAY: Weather stations with ice temperature
  // Not currently called - can be activated later
  
  // Clear screen and display both stations
  M5.Display.fillScreen(BLACK);
  
  // Left 3/4 - Top half: Glacier (current selection)
  displayWeather(currentGlacierStation, 0, 0, 96, 64);
  
  // Left 3/4 - Bottom half: Shenzhen
  displayWeather(localStation, 0, 64, 96, 64);
  
  // Right 1/4 - Peltier temperature display
  M5.Display.drawRect(96, 0, 32, 128, WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(98, 3);
  M5.Display.setTextColor(WHITE);
  M5.Display.println("Ice");
  M5.Display.setCursor(98, 20);
  M5.Display.setTextColor(YELLOW);
  M5.Display.printf("%.1f", peltierTemperature);
  M5.Display.setCursor(98, 35);
  M5.Display.setTextColor(WHITE);
  M5.Display.println("C");
}

void displaySystemStatus(float peltierTemperature) {
  // NEW DISPLAY: System status information
  
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextSize(1.2);
  
  int y = 5;
  
  // Thermostat state
  M5.Display.setCursor(5, y);
  M5.Display.setTextColor(WHITE);
  M5.Display.print("Thermostat: ");
  if (thermostat.isCooling()) {
    M5.Display.setTextColor(GREEN);
    M5.Display.println("ON");
  } else {
    M5.Display.setTextColor(RED);
    M5.Display.println("OFF");
  }
  y += 15;
  
  // Setpoint temperature
  M5.Display.setCursor(5, y);
  M5.Display.setTextColor(WHITE);
  M5.Display.print("Setpoint: ");
  M5.Display.setTextColor(CYAN);
  M5.Display.printf("%.1f C", thermostat.getSetPoint());
  y += 15;
  
  // Reactivate temperature
  M5.Display.setCursor(5, y);
  M5.Display.setTextColor(WHITE);
  M5.Display.print("Reactivate: ");
  M5.Display.setTextColor(ORANGE);
  M5.Display.printf("%.1f C", thermostat.getReactivateTemp());
  y += 20;
  
  // Peltier temperature
  M5.Display.setCursor(5, y);
  M5.Display.setTextColor(WHITE);
  M5.Display.print("Peltier: ");
  M5.Display.setTextColor(YELLOW);
  M5.Display.printf("%.1f C", peltierTemperature);
  y += 20;
  
  // Drop count
  M5.Display.setCursor(5, y);
  M5.Display.setTextColor(WHITE);
  M5.Display.print("Drops: ");
  M5.Display.setTextColor(MAGENTA);
  M5.Display.printf("%d", dropCount);
}
