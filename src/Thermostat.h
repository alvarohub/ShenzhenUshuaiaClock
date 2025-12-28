#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <M5Unified.h>
#include "config.h"

class Thermostat {
private:
  int controlPin;           // Pin controlling the MOSFET
  float setPoint;           // Target temperature (cooling stops here)
  float reactivateTemp;     // Temperature to restart cooling (asymmetric hysteresis)
  float currentTemp;        // Actual temperature from Dallas sensor
  bool coolingActive;       // Current state of the Peltier
  unsigned long setpointReachedTime;  // Time when setpoint was first reached
  bool inFreezingDuration;  // True when maintaining freeze after reaching setpoint
  unsigned long coolingStoppedTime;   // Time when cooling was turned off
  
public:
  // Constructor
  Thermostat(int pin = PIN_PELTIER) 
    : controlPin(pin), 
      setPoint(0.0),
      reactivateTemp(25.0),  // Will be set to appropriate value in setup
      currentTemp(25.0), 
      coolingActive(false),
      setpointReachedTime(0),
      inFreezingDuration(false),
      coolingStoppedTime(0) {
  }
  
  // Initialize the thermostat hardware
  void begin() {
    pinMode(controlPin, OUTPUT);
    digitalWrite(controlPin, LOW); // Start with Peltier OFF
  }
  
  // Set the target temperature (will be updated from Ilulissat data)
  void setSetPoint(float temp) {
    setPoint = temp;
  }
  
  // Get current set point
  float getSetPoint() {
    return setPoint;
  }
  
  // Update current temperature reading (from Dallas sensor)
  void setCurrentTemp(float temp) {
    currentTemp = temp;
  }
  
  // Get current temperature
  float getCurrentTemp() {
    return currentTemp;
  }
  
  // Check if cooling is active
  bool isCooling() {
    return coolingActive;
  }
  
  // Main control loop - call this regularly
  // Asymmetric hysteresis: Cool to setPoint, maintain for duration, then wait until reactivateTemp or timer
  void update() {
    if (coolingActive) {
      // Peltier is ON (cooling)
      if (currentTemp <= setPoint) {
        // Setpoint reached
        if (!inFreezingDuration) {
          // First time reaching setpoint - start freeze duration timer
          inFreezingDuration = true;
          setpointReachedTime = millis();
        } else {
          // Check if freeze duration has elapsed
          if (millis() - setpointReachedTime >= DURATION_GLACIER_FREEZING) {
            // Duration complete - turn off cooling
            coolingActive = false;
            inFreezingDuration = false;
            coolingStoppedTime = millis();  // Record when cooling stopped
            digitalWrite(controlPin, LOW);
          }
          // Otherwise keep cooling
        }
      } else {
        // Temperature rose above setpoint - reset duration tracking
        inFreezingDuration = false;
      }
    } else {
      // Peltier is OFF (ice melting)
      // Turn ON when:
      // 1. Temperature rises to reactivateTemp, OR
      // 2. Timer has elapsed since cooling stopped
      if (currentTemp >= reactivateTemp || 
          (millis() - coolingStoppedTime >= REACTIVATE_TIMER)) {
        coolingActive = true;
        inFreezingDuration = false;
        digitalWrite(controlPin, HIGH);
      }
    }
  }
  
  // Force cooling to start immediately (called when drop detected)
  void forceActivate() {
    if (!coolingActive) {
      coolingActive = true;
      inFreezingDuration = false;
      digitalWrite(controlPin, HIGH);
    }
  }
  
  // Manual control (for testing)
  void turnOn() {
    coolingActive = true;
    digitalWrite(controlPin, HIGH);
  }
  
  void turnOff() {
    coolingActive = false;
    digitalWrite(controlPin, LOW);
  }
   
  // Set/get reactivate temperature
  void setReactivateTemp(float temp) {
    reactivateTemp = temp;
  }
  
  float getReactivateTemp() {
    return reactivateTemp;
  }
  
  // Test mode - manual Peltier control with button
  void testMode() {
    M5.Display.setTextSize(1.6);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Peltier Test");
    M5.Display.setTextSize(1.4);
    M5.Display.setCursor(10, 50);
    M5.Display.println("Press button");
    M5.Display.setCursor(10, 70);
    M5.Display.println("to toggle");
    
    bool peltierOn = false;
    
    while (true) {
      M5.update();
      
      if (M5.BtnA.wasPressed()) {
        peltierOn = !peltierOn;
        digitalWrite(controlPin, peltierOn ? HIGH : LOW);
        
        // Update display
        if (peltierOn) {
          M5.Display.fillScreen(GREEN);
          M5.Display.setTextColor(BLACK, GREEN);
          M5.Display.setTextSize(1.6);
          M5.Display.setCursor(10, 10);
          M5.Display.println("Peltier Test");
          M5.Display.setTextSize(4);
          M5.Display.setCursor(30, 60);
          M5.Display.println("ON");
        } else {
          M5.Display.fillScreen(RED);
          M5.Display.setTextColor(WHITE, RED);
          M5.Display.setTextSize(1.6);
          M5.Display.setCursor(10, 10);
          M5.Display.println("Peltier Test");
          M5.Display.setTextSize(4);
          M5.Display.setCursor(30, 60);
          M5.Display.println("OFF");
        }
      }
      
      delay(10);
    }
  }
};

// Global instance (like Serial, Wire, etc.)
extern Thermostat thermostat;

#endif // THERMOSTAT_H
