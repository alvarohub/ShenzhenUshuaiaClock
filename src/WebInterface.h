#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "config.h"

class WebInterface {
private:
  AsyncWebServer server;
  bool apMode;
  String apSSID;
  IPAddress apIP;
  IPAddress stationIP;
  
  // HTML page content
  String getHTML();
  
  // API endpoints
  void handleRoot(AsyncWebServerRequest *request);
  void handleStatus(AsyncWebServerRequest *request);
  void handleUpdate(AsyncWebServerRequest *request);
  void handleDrop(AsyncWebServerRequest *request);
  
public:
  WebInterface() : server(80), apMode(false), apSSID("DrippingMeteorite") {
    apIP = IPAddress(192, 168, 4, 1);
  }
  
  // Initialize web server with dual mode (AP + Station)
  bool begin(bool enableAP = true) {
    bool success = false;
    
    // Always start AP mode for direct access
    if (enableAP) {
      WiFi.mode(WIFI_AP_STA);  // Both AP and Station
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      apMode = WiFi.softAP(apSSID.c_str(), WEBSERVER_AP_PASSWORD);
      
      if (apMode) {
        Serial.println("Access Point started:");
        Serial.print("  SSID: ");
        Serial.println(apSSID);
        Serial.print("  IP: ");
        Serial.println(WiFi.softAPIP());
        success = true;
      }
    } else {
      WiFi.mode(WIFI_STA);  // Station only
    }
    
    // Setup web server routes
    setupRoutes();
    
    // Start web server
    server.begin();
    Serial.println("Web server started on port 80");
    
    return success;
  }
  
  // Setup HTTP routes
  void setupRoutes() {
    // Serve main HTML page
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
      request->send(200, "text/html", getHTML());
    });
    
    // API endpoint for system status (JSON)
    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
      handleStatus(request);
    });
    
    // API endpoint to update parameters
    server.on("/api/update", HTTP_POST, [this](AsyncWebServerRequest *request) {
      handleUpdate(request);
    });
    
    // API endpoint to trigger drop test
    server.on("/api/drop", HTTP_POST, [this](AsyncWebServerRequest *request) {
      handleDrop(request);
    });
    
    // Handle not found
    server.onNotFound([](AsyncWebServerRequest *request) {
      request->send(404, "text/plain", "Not found");
    });
  }
  
  // Get current AP IP
  IPAddress getAPIP() {
    return apIP;
  }
  
  // Get current station IP (if connected to WiFi)
  IPAddress getStationIP() {
    if (WiFi.status() == WL_CONNECTED) {
      stationIP = WiFi.localIP();
    }
    return stationIP;
  }
  
  // Check if AP mode is active
  bool isAPActive() {
    return apMode;
  }
  
  // Get AP SSID
  String getAPSSID() {
    return apSSID;
  }
};

// Global instance
extern WebInterface webInterface;

#endif // WEB_INTERFACE_H
