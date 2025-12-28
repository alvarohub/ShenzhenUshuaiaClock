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
    strip.fill(strip.Color(r, g, b));
  }
  
  // Fill entire strip with one color (32-bit color)
  void fill(uint32_t color) {
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
    uint8_t maxWhite = (uint8_t)(255 * (NEOPIXEL_BRIGHTNESS / 255.0));
    fill(maxWhite, maxWhite, maxWhite);
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
  
  // Startup system check - R/G/B/White sequence, 1 second each
  void startSystem() {
    uint8_t level = (uint8_t)(255 * (NEOPIXEL_BRIGHTNESS / 255.0));
    
    // Red
    fill(level, 0, 0);
    show();
    delay(1000);
    
    // Green
    fill(0, level, 0);
    show();
    delay(1000);
    
    // Blue
    fill(0, 0, level);
    show();
    delay(1000);
    
    // White
    fill(level, level, level);
    show();
    delay(1000);
    
    // Clear
    fill(0, 0, 0);
    show();
  }
  
  // Error indication - pulse red LEDs forever (hardware problem)
  // Does NOT clear the screen so initialization status remains visible
  void pulseRedError() {
    uint8_t maxRed = (uint8_t)(255 * (NEOPIXEL_BRIGHTNESS / 255.0));
    while (true) {
      M5.update();
      
      // Pulse red by varying the color value, not the brightness setting
      for (int level = 10; level <= 255; level += 5) {
        uint8_t red = (uint8_t)((level / 255.0) * maxRed);
        fill(red, 0, 0);
        show();
        delay(10);
      }
      for (int level = 255; level >= 10; level -= 5) {
        uint8_t red = (uint8_t)((level / 255.0) * maxRed);
        fill(red, 0, 0);
        show();
        delay(10);
      }
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
