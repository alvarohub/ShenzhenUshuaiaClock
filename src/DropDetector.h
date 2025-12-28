#ifndef DROP_DETECTOR_H
#define DROP_DETECTOR_H

#include <M5Unified.h>
#include "config.h"

class DropDetector {
private:
  int sensorPin;
  int interruptMode;        // RISING or FALLING
  unsigned long debounceMs;
  
  volatile bool interruptTriggered;
  unsigned long lastDetectionTime;
  bool interruptEnabled;
  
  // Static ISR handler - needs access to instance
  static DropDetector* instance;
  static void IRAM_ATTR handleInterrupt();
  
public:
  // Constructor
  DropDetector(int pin = PIN_DROP_DETECTOR, int mode = DROP_TRIGGER_MODE, unsigned long debounce = DROP_DEBOUNCE_MS) 
    : sensorPin(pin),
      interruptMode(mode),
      debounceMs(debounce),
      interruptTriggered(false),
      lastDetectionTime(0),
      interruptEnabled(false) {
    instance = this;
  }
  
  // Initialize the drop detector
  void begin() {
    pinMode(sensorPin, INPUT_PULLUP);
    enableInterrupt();
  }
  
  // Enable interrupt detection
  void enableInterrupt() {
    if (!interruptEnabled) {
      attachInterrupt(digitalPinToInterrupt(sensorPin), handleInterrupt, interruptMode);
      interruptEnabled = true;
    }
  }
  
  // Disable interrupt detection
  void disableInterrupt() {
    if (interruptEnabled) {
      detachInterrupt(digitalPinToInterrupt(sensorPin));
      interruptEnabled = false;
    }
  }
  
  // Update method - call regularly in loop
  // Returns true if a drop was detected (after debounce)
  bool update() {
    if (interruptTriggered) {
      unsigned long currentTime = millis();
      
      // Check if enough time has passed since last detection (debounce)
      if (currentTime - lastDetectionTime >= debounceMs) {
        // Valid detection!
        lastDetectionTime = currentTime;
        interruptTriggered = false;  // Clear the flag
        return true;
      } else {
        // Too soon - ignore this trigger (bounce)
        interruptTriggered = false;
      }
    }
    return false;
  }
  
  // Check if a drop is currently detected (without debouncing)
  bool isTriggered() const {
    return interruptTriggered;
  }
  
  // Get current sensor state
  bool getSensorState() const {
    return digitalRead(sensorPin);
  }
  
  // Set debounce time in milliseconds
  void setDebounceTime(unsigned long ms) {
    debounceMs = ms;
  }
  
  // Get time since last detection
  unsigned long timeSinceLastDetection() const {
    return millis() - lastDetectionTime;
  }
  
  // Reset the detector state
  void reset() {
    interruptTriggered = false;
    lastDetectionTime = 0;
  }
  
  // Change trigger mode (RISING, FALLING, or CHANGE)
  void setTriggerMode(int mode) {
    disableInterrupt();
    interruptMode = mode;
    enableInterrupt();
  }
  
  // Test mode - displays drop detection visually on screen
  void testMode() {
    M5.Display.setTextSize(1.6);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Drop Sensor Test");
    M5.Display.setTextSize(1.4);
    
    int dropCount = 0;
    bool isGreen = false;
    
    while (true) {
      M5.update();
      
      if (update()) {
        // Drop detected!
        dropCount++;
        isGreen = true;
        M5.Display.fillScreen(GREEN);
        M5.Display.setTextColor(BLACK, GREEN);
        M5.Display.setTextSize(1.6);
        M5.Display.setCursor(10, 10);
        M5.Display.println("Drop Sensor Test");
        M5.Display.setTextSize(1.4);
        M5.Display.setCursor(10, 50);
        M5.Display.printf("Drops: %d", dropCount);
        delay(200); // Keep green briefly
      } else if (isGreen) {
        // Return to black
        isGreen = false;
        M5.Display.fillScreen(BLACK);
        M5.Display.setTextColor(WHITE, BLACK);
        M5.Display.setTextSize(1.6);
        M5.Display.setCursor(10, 10);
        M5.Display.println("Drop Sensor Test");
        M5.Display.setTextSize(1.4);
        M5.Display.setCursor(10, 50);
        M5.Display.printf("Drops: %d", dropCount);
      }
      
      delay(10);
    }
  }
};

// Global instance (like Serial, Wire, etc.)
extern DropDetector dropDetector;

#endif // DROP_DETECTOR_H
