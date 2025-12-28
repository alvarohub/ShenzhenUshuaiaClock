#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <M5Unified.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

class TemperatureSensor {
private:
  OneWire oneWire;
  DallasTemperature dallas;
  int dataPin;
  float currentTemp;
  bool sensorFound;
  
public:
  // Constructor
  TemperatureSensor(int pin = PIN_TEMPERATURE) 
    : oneWire(pin), 
      dallas(&oneWire),
      dataPin(pin),
      currentTemp(25.0),
      sensorFound(false) {
  }
  
  // Initialize the sensor
  bool begin() {
    dallas.begin();
    int deviceCount = dallas.getDeviceCount();
    sensorFound = (deviceCount > 0);
    
    if (sensorFound) {
      // Set resolution (9-12 bits, higher = more accurate but slower)
      dallas.setResolution(TEMP_SENSOR_RESOLUTION);
    }
    
    return sensorFound;
  }
  
  // Read temperature (blocking call)
  float readTemperature() {
    if (!sensorFound) {
      return currentTemp; // Return last known value if sensor not found
    }
    
    dallas.requestTemperatures();
    currentTemp = dallas.getTempCByIndex(0);
    
    // Check for sensor error (-127.0 is error value)
    if (currentTemp == -127.0 || currentTemp == 85.0) {
      // 85.0 is power-on reset value, might indicate read error
      return currentTemp; // Keep last valid reading
    }
    
    return currentTemp;
  }
  
  // Non-blocking temperature read (call update() first, then getTemperature())
  void requestUpdate() {
    if (sensorFound) {
      dallas.requestTemperatures();
    }
  }
  
  float getTemperature() {
    if (sensorFound) {
      float temp = dallas.getTempCByIndex(0);
      if (temp != -127.0 && temp != 85.0) {
        currentTemp = temp;
      }
    }
    return currentTemp;
  }
  
  // Get last read temperature without new reading
  float getLastTemperature() {
    return currentTemp;
  }
  
  // Check if sensor is connected
  bool isConnected() {
    return sensorFound;
  }
  
  // Get number of devices on bus
  int getDeviceCount() {
    return dallas.getDeviceCount();
  }
  
  // Rescan for devices
  void rescan() {
    dallas.begin();
    sensorFound = (dallas.getDeviceCount() > 0);
  }
  
  // Test mode - displays temperature readings on screen
  void testMode() {
    M5.Display.setTextSize(1.6);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE, BLACK);
    
    while (true) {
      M5.update();
      
      float temp = readTemperature();
      
      M5.Display.fillScreen(BLACK);
      M5.Display.setTextSize(1.6);
      M5.Display.setCursor(10, 10);
      M5.Display.println("Temp Sensor Test");
      M5.Display.setTextSize(1.4);
      M5.Display.setCursor(10, 50);
      M5.Display.printf("%.2f C", temp);
      
      if (isConnected()) {
        M5.Display.setCursor(10, 80);
        M5.Display.setTextColor(GREEN, BLACK);
        M5.Display.println("Sensor OK");
      } else {
        M5.Display.setCursor(10, 80);
        M5.Display.setTextColor(RED, BLACK);
        M5.Display.println("No Sensor!");
      }
      
      delay(500);
    }
  }
};

// Global instance (like Serial, Wire, etc.)
extern TemperatureSensor tempSensor;

#endif // TEMPERATURE_SENSOR_H
