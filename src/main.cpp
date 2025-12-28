#include <M5Unified.h>
#include "config.h"
#include "WeatherStation.h"
#include "WiFiManager.h"
#include "TemperatureSensor.h"
#include "NeoPixelController.h"
#include "DropDetector.h"
#include "Thermostat.h"
#include "AudioPlayer.h"

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

// Weather stations - easily editable!
// Note: Ilulissat (Greenland) - Jakobshavn Glacier, one of fastest melting
//       El Calafate (Patagonia) - Near Upsala & Perito Moreno glaciers
//       Hong Kong - High humidity reference point
//       Shenzhen - Local reference station
WeatherStation stations[] = {
  {"Ilulissat", 69.2198, -51.0986, "America/Godthab", -2.0, 50.0, 0.0},                    // Greenland glacier
  {"El Calafate", -50.3375, -72.2647, "America/Argentina/Rio_Gallegos", -2.0, 50.0, 0.0},  // Patagonia glacier
  {"Hong Kong", 22.3193, 114.1694, "Asia/Hong_Kong", 26.0, 75.0, 0.0},                     // Humid reference
  {"Shenzhen", 22.5431, 114.0579, "Asia/Shanghai", 14.0, 75.0, 0.0}                        // Local station
}; 

const int NUM_STATIONS = sizeof(stations) / sizeof(stations[0]);
int glacierIndex = 0;      // Which glacier to display (0 or 1)
const int LOCAL_INDEX = 3; // Shenzhen is local reference

// Station references for clarity
WeatherStation& currentGlacierStation = stations[0];  // Can be changed via glacierIndex
WeatherStation& localStation = stations[LOCAL_INDEX];

// Drop counter
int dropCount = 0;

// Display update timing (non-blocking)
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 500; // Update display every 500ms

// Temperature sensor timing (non-blocking)
unsigned long lastTempRead = 0;
float cachedPeltierTemperature = 20.0; // Cached temperature value

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
    if (tempSensor.begin()) {
      M5.Display.println("OK");
    } else {
      M5.Display.setTextColor(RED);
      M5.Display.println("FAIL");
      M5.Display.setTextColor(GREEN);
      #if GLOBAL_DEBUG
        delay(2000);
        neoPixels.pulseRedError();  // Stop in debug mode
      #endif
      hardwareOK = false;
    }
  #endif
  
  // Drop Detector
  #if !GLOBAL_DEBUG || DEBUG_DROP_SENSOR
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("2.DropSens.");
    dropDetector.begin();
    M5.Display.println("OK");
  #endif
  
  // NeoPixels
  #if !GLOBAL_DEBUG || DEBUG_NEOPIXEL
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("3.NeoPix...");
    if (neoPixels.begin()) {
      neoPixels.clear();
      neoPixels.show();
      M5.Display.println("OK");
    } else {
      M5.Display.setTextColor(RED);
      M5.Display.println("FAIL");
      M5.Display.setTextColor(GREEN);
      #if GLOBAL_DEBUG
        delay(2000);
        while(true) { delay(1000); }  // Stop (can't pulse LEDs if they failed)
      #endif
      hardwareOK = false;
    }
  #endif
  
  // WiFi
  #if !GLOBAL_DEBUG
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("4.WiFi.....");
    wifiManager.begin();
    if (wifiManager.isEnabled()) {
      if (wifiManager.connect()) {
        M5.Display.println("OK");
        wifiManager.fetchWeather(stations, NUM_STATIONS);
      } else {
        M5.Display.setTextColor(YELLOW);
        M5.Display.println("FAIL");
        M5.Display.setTextColor(GREEN);
      }
    } else {
      M5.Display.setTextColor(YELLOW);
      M5.Display.println("DIS");
      M5.Display.setTextColor(GREEN);
    }
  #endif
  
  // Audio Player
  #if !GLOBAL_DEBUG || DEBUG_AUDIO_PLAYER
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("5.AudioPlayer.");
    if (audioPlayer.begin()) {
      M5.Display.println("OK");
    } else {
      M5.Display.setTextColor(RED);
      M5.Display.println("FAIL");
      M5.Display.setTextColor(GREEN);
      #if GLOBAL_DEBUG
        delay(2000);
        neoPixels.pulseRedError();  // Stop in debug mode
      #endif
      hardwareOK = false;
    }
  #endif
  
  // Thermostat
  #if !GLOBAL_DEBUG || DEBUG_THERMOSTAT
    M5.Display.setCursor(5, y); y += 12;
    M5.Display.print("6.Thermostat..");
    thermostat.begin();
    M5.Display.println("OK");
  #endif
  
  M5.Display.setCursor(5, y); y += 15;
  M5.Display.setTextSize(1.4);
  
  // Check if any component failed (only stop in normal mode)
  #if !GLOBAL_DEBUG
    if (!hardwareOK) {
      M5.Display.setTextColor(RED);
      M5.Display.println("HARDWARE ERROR!");
      delay(2000);
      neoPixels.pulseRedError();  // Never returns
    }
  #endif
  
  M5.Display.setTextColor(WHITE);
  M5.Display.println("ALL READY!");
  
  // LED startup sequence
  neoPixels.startSystem();
  
  // ==================================================
  // PROGRAM INITIALIZATION
  // ==================================================
  // 1. Set thermostat setpoint to glacier temperature
  thermostat.setSetPoint(currentGlacierStation.temperature);
  
  // 2. Set reactivate temperature from config
  thermostat.setReactivateTemp(REACTIVATE_TEMP);
  
  // 3. Start cooling immediately
  thermostat.turnOn();
  
  delay(2000);
  
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
  
  // Read peltier/ice temperature from Dallas sensor (only every TEMP_READ_INTERVAL)
  unsigned long currentMillis = millis();
  if (currentMillis - lastTempRead >= TEMP_READ_INTERVAL) {
    cachedPeltierTemperature = tempSensor.readTemperature();
    lastTempRead = currentMillis;
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
  
  // Update LED brightness based on timer (exponential fade)
  neoPixels.updateTimerFade();
  
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
    // Update thermostat setpoint with new glacier temperature
    thermostat.setSetPoint(currentGlacierStation.temperature);
  }
  
  // ==================================================
  // DISPLAY UPDATE
  // ==================================================
  
  // Update display only periodically (non-blocking)
  if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = currentMillis;
    displaySystemStatus(cachedPeltierTemperature);
  }
  
  // LCD button (BtnA - button under the display) to toggle WiFi or refresh
  if (M5.BtnA.wasPressed()) {
    if (wifiManager.isConnected()) {
      // If connected, refresh weather data
      wifiManager.fetchWeather(stations, NUM_STATIONS);
    } else if (wifiManager.isEnabled()) {
      // If WiFi enabled but not connected, retry connection
      wifiManager.connect();
    } else {
      // If WiFi disabled, enable it and connect
      wifiManager.enable();
      M5.Display.fillScreen(BLACK);
      M5.Display.setCursor(10, 10);
      M5.Display.println("WiFi Enabled");
      delay(1000);
      if (wifiManager.connect()) {
        wifiManager.fetchWeather(stations, NUM_STATIONS);
      }
    }
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
