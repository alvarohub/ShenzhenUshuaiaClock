#ifndef NEOPIXEL_CONTROLLER_H
#define NEOPIXEL_CONTROLLER_H

#include <M5Unified.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

class NeoPixelController {
private:
  Adafruit_NeoPixel strip;
  int pin;
  int numLeds;
  bool initialized;
  
  // Drop-activated fade state
  float dropTemperature;     // Temperature when drop was detected
  float glacierTemperature;  // Target temperature (setpoint)
  bool fadeActive;           // Whether fade animation is active
  
  // Timer-based fade state
  unsigned long fadeStartTime;  // When the fade started
  bool timerFadeActive;         // Whether timer-based fade is active
  
public:
  // Constructor
  NeoPixelController(int ledPin = PIN_NEOPIXEL, int ledCount = NEOPIXEL_COUNT) 
    : strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800),
      pin(ledPin),
      numLeds(ledCount),
      initialized(false),
      dropTemperature(0.0),
      glacierTemperature(0.0),
      fadeActive(false),
      fadeStartTime(0),
      timerFadeActive(false) {
  }
  
  // Initialize the NeoPixel strip
  bool begin() {
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    strip.setBrightness(255); // Always use max brightness, we'll scale colors instead
    initialized = true;
    return true;  // Adafruit_NeoPixel::begin() doesn't fail, always returns successfully
  }
  
  // Check if hardware is working
  bool isInitialized() const {
    return initialized;
  }
  
  // Get number of LEDs
  int getNumLeds() {
    return numLeds;
  }
  
  // Set individual pixel color (RGB)
  void setPixelColor(int pixel, uint8_t r, uint8_t g, uint8_t b) {
    if (pixel >= 0 && pixel < numLeds) {
      strip.setPixelColor(pixel, strip.Color(r, g, b));
    }
  }
  
  // Set individual pixel color (32-bit color)
  void setPixelColor(int pixel, uint32_t color) {
    if (pixel >= 0 && pixel < numLeds) {
      strip.setPixelColor(pixel, color);
    }
  }
  
  // Update the strip (must call this to show changes)
  void show() {
    if (!initialized) return;
    strip.show();
  }
  
  // Clear all pixels
  void clear() {
    if (!initialized) return;
    strip.clear();
  }
  
  // Fill entire strip with one color
  void fill(uint8_t r, uint8_t g, uint8_t b) {
    if (!initialized) return;
    strip.fill(strip.Color(r, g, b));
  }
  
  // Fill entire strip with one color (32-bit color)
  void fill(uint32_t color) {
    if (!initialized) return;
    strip.fill(color);
  }
  
  // Fill range of pixels
  void fillRange(int start, int count, uint8_t r, uint8_t g, uint8_t b) {
    strip.fill(strip.Color(r, g, b), start, count);
  }
  
  // Helper to create color value
  uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
    return strip.Color(r, g, b);
  }
  
  // Get direct access to strip for advanced operations
  Adafruit_NeoPixel& getStrip() {
    return strip;
  }
  
  // Trigger drop event - LEDs go full white and start fade cycle
  void onDropDetected(float currentTemp, float targetTemp) {
    dropTemperature = currentTemp;
    glacierTemperature = targetTemp;
    fadeActive = true;
    
    // Start timer-based fade
    timerFadeActive = true;
    fadeStartTime = millis();
    
    // Set full white immediately (scaled by global brightness)
    fill(NEOPIXEL_BRIGHTNESS, NEOPIXEL_BRIGHTNESS, NEOPIXEL_BRIGHTNESS);
    show();
  }
  
  // Update fade based on current temperature
  // Maps temperature from dropTemperature (full white) to glacierTemperature (black)
  void updateTemperatureFade(float currentTemp) {
    if (!fadeActive) {
      return;  // No fade active, LEDs stay as they are
    }
    
    // Map temperature to brightness (0.0 = black, 1.0 = full white)
    float ratio = (currentTemp - glacierTemperature) / (dropTemperature - glacierTemperature);
    
    // Constrain to valid range
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;
    
    // Calculate white level (same for R, G, B)
    uint8_t whiteLevel = (uint8_t)(ratio * 255);
    
    // Set all LEDs to white with calculated brightness
    fill(whiteLevel, whiteLevel, whiteLevel);
    show();
  }
  
  // Update fade based on timer with exponential decay (perceptually linear)
  // Formula: brightness = 255 * exp(-t * ln(255) / LED_FADE_TOTAL_TIME)
  // This ensures fade from 255 at t=0 to 1 at t=LED_FADE_TOTAL_TIME
  void updateTimerFade() {
    if (!timerFadeActive) {
      return;  // No timer fade active
    }
    
    unsigned long elapsed = millis() - fadeStartTime;
    
    // Check if fade is complete
    if (elapsed >= LED_FADE_TOTAL_TIME) {
      timerFadeActive = false;
      fill(0, 0, 0);  // Full black
      show();
      return;
    }
    
    // Linear fade from 255 to 0 over LED_FADE_TOTAL_TIME
    float ratio = 1.0 - ((float)elapsed / (float)LED_FADE_TOTAL_TIME);
    
    // Scale by global brightness setting (0-255) for full resolution
    float scaledBrightness = ratio * 255.0 * (NEOPIXEL_BRIGHTNESS / 255.0);
    uint8_t whiteLevel = (uint8_t)scaledBrightness;
    
    // Set all LEDs to white with calculated brightness
    fill(whiteLevel, whiteLevel, whiteLevel);
    show();
  }
  
  // Check if fade is currently active
  bool isFading() const {
    return timerFadeActive;
  }
  
  // Ambient cube lighting - blue pulse when cooling, red glow when off
  void updateAmbientLight(bool isCooling, bool cubeLightEnabled, uint8_t brightness) {
    if (timerFadeActive) {
      return;  // Drop animation active - skip ambient lighting
    }
    
    if (!cubeLightEnabled) {
      // Turn off ambient lighting
      fill(0, 0, 0);
      show();
      return;
    }
    
    static unsigned long lastPulseUpdate = 0;
    static int pulseDirection = 1;  // 1 = getting brighter, -1 = getting dimmer
    static uint8_t pulseLevel = 30;  // Current pulse brightness (10-80)
    
    // Update pulse every 30ms for smooth animation
    if (millis() - lastPulseUpdate < 30) {
      return;
    }
    lastPulseUpdate = millis();
    
    if (isCooling) {
      // Blue pulsating (smooth breathing effect)
      pulseLevel += pulseDirection * 2;
      if (pulseLevel >= 80) {
        pulseLevel = 80;
        pulseDirection = -1;
      } else if (pulseLevel <= 10) {
        pulseLevel = 10;
        pulseDirection = 1;
      }
      
      uint8_t blue = (uint8_t)((pulseLevel / 255.0) * brightness);
      fill(0, 0, blue);
      show();
    } else {
      // Red steady glow (no pulsing to avoid confusion with error)
      fill(brightness, 0, 0);
      show();
    }
  }
  
  // Startup system check - blink green 4 times
  void startSystem() {
    for (int i = 0; i < 4; i++) {
      // Green ON
      fill(0, NEOPIXEL_BRIGHTNESS, 0);
      show();
      delay(1000);
      
      // OFF
      fill(0, 0, 0);
      show();
      delay(1000);
    }
  }
  
  // Error indication - blink red LEDs on/off every second (hardware problem)
  // Does NOT clear the screen so initialization status remains visible
  void pulseRedError() {
    while (true) {
      M5.update();
      
      // Red ON for 1 second
      fill(NEOPIXEL_BRIGHTNESS, 0, 0);
      show();
      delay(1000);
      
      // OFF for 1 second
      fill(0, 0, 0);
      show();
      delay(1000);
    }
  }
  
  // Test mode - cycles through colors
  void testMode() {
    M5.Display.setTextSize(1.6);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setCursor(10, 10);
    M5.Display.println("NeoPixel Test");
    M5.Display.setTextSize(1.4);
    M5.Display.setCursor(10, 50);
    M5.Display.printf("%d LEDs", numLeds);
    
    while (true) {
      M5.update();
      
      // Red
      M5.Display.fillRect(10, 80, 100, 20, BLACK);
      M5.Display.setCursor(10, 80);
      M5.Display.setTextColor(RED, BLACK);
      M5.Display.println("RED");
      fill(255, 0, 0);
      show();
      delay(1000);
      
      // Green
      M5.Display.fillRect(10, 80, 100, 20, BLACK);
      M5.Display.setCursor(10, 80);
      M5.Display.setTextColor(GREEN, BLACK);
      M5.Display.println("GREEN");
      fill(0, 255, 0);
      show();
      delay(1000);
      
      // Blue
      M5.Display.fillRect(10, 80, 100, 20, BLACK);
      M5.Display.setCursor(10, 80);
      M5.Display.setTextColor(BLUE, BLACK);
      M5.Display.println("BLUE");
      fill(0, 0, 255);
      show();
      delay(1000);
      
      // White
      M5.Display.fillRect(10, 80, 100, 20, BLACK);
      M5.Display.setCursor(10, 80);
      M5.Display.setTextColor(WHITE, BLACK);
      M5.Display.println("WHITE");
      fill(255, 255, 255);
      show();
      delay(1000);
      
      // Off
      M5.Display.fillRect(10, 80, 100, 20, BLACK);
      M5.Display.setCursor(10, 80);
      M5.Display.println("OFF");
      clear();
      show();
      delay(500);
    }
  }
};

// Global instance (like Serial, Wire, etc.)
extern NeoPixelController neoPixels;

#endif // NEOPIXEL_CONTROLLER_H
