#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "WeatherStation.h"

class WiFiManager {
private:
  bool connected;
  bool attempted;
  bool enabled;
  unsigned long lastAttemptTime;
  unsigned long lastUpdateTime;
  
public:
  WiFiManager() 
    : connected(false),
      attempted(false),
      enabled(WIFI_ENABLED),
      lastAttemptTime(0),
      lastUpdateTime(0) {
  }
  
  // Initialize WiFi (does not connect yet)
  void begin() {
    WiFi.mode(WIFI_STA);
  }
  
  // Attempt to connect to WiFi
  bool connect() {
    if (connected) {
      return true; // Already connected
    }
    
    lastAttemptTime = millis();
    
    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Connecting...");
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 2) {
      delay(500);
      M5.Display.print(".");
      attempts++;
    }
    
    attempted = true;
    M5.Display.fillScreen(BLACK);
    
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      M5.Display.setCursor(10, 10);
      M5.Display.println("WiFi Connected!");
      M5.Display.setCursor(10, 30);
      M5.Display.print("IP: ");
      M5.Display.println(WiFi.localIP());
      delay(2000);
      return true;
    } else {
      connected = false;
      M5.Display.setCursor(10, 10);
      M5.Display.println("WiFi Failed!");
      M5.Display.setCursor(10, 30);
      M5.Display.println("Using presets");
      M5.Display.setCursor(10, 50);
      M5.Display.println("Press screen");
      M5.Display.setCursor(10, 65);
      M5.Display.println("to retry");
      delay(2000);
      return false;
    }
  }
  
  // Fetch weather data for all stations
  void fetchWeather(WeatherStation* stations, int numStations) {
    if (!connected || WiFi.status() != WL_CONNECTED) {
      return;  // Silently skip if no WiFi
    }
    
    HTTPClient http;
    
    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Fetching data...");
    
    // Fetch data for all weather stations
    for (int i = 0; i < numStations; i++) {
      String url = "https://api.open-meteo.com/v1/forecast?latitude=";
      url += String(stations[i].lat, 4);
      url += "&longitude=";
      url += String(stations[i].lon, 4);
      url += "&current=temperature_2m,relative_humidity_2m,dew_point_2m&timezone=";
      url += stations[i].timezone;
      
      http.begin(url);
      int httpCode = http.GET();
      
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
          stations[i].temperature = doc["current"]["temperature_2m"];
          stations[i].humidity = doc["current"]["relative_humidity_2m"];
          stations[i].dewPoint = doc["current"]["dew_point_2m"];
        }
      }
      http.end();
    }
    
    lastUpdateTime = millis();
  }
  
  // Check if periodic update is needed
  bool shouldUpdate() {
    return connected && (millis() - lastUpdateTime > WEATHER_UPDATE_INTERVAL);
  }
  
  // Check if periodic retry is needed
  bool shouldRetry() {
    return enabled && !connected && (millis() - lastAttemptTime > WIFI_RETRY_INTERVAL);
  }
  
  // Getters
  bool isConnected() const { return connected; }
  bool isEnabled() const { return enabled; }
  bool wasAttempted() const { return attempted; }
  
  // Enable/disable WiFi
  void enable() { enabled = true; }
  void disable() { enabled = false; }
};

// Global instance
extern WiFiManager wifiManager;

#endif // WIFI_MANAGER_H
