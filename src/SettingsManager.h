#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <Preferences.h>
#include "config.h"

class SettingsManager {
private:
  Preferences preferences;
  const char* NAMESPACE = "settings";
  const char* INITIALIZED_KEY = "initialized";
  
public:
  // Settings structure matching config.h parameters
  struct Settings {
    float manualSetpoint;
    float reactivateTemp;
    unsigned long durationGlacierFreezing;
    unsigned long reactivateTimer;
    uint8_t neopixelBrightness;
    uint16_t ledFadeTotalTime;
    unsigned long tempReadInterval;
    uint8_t audioVolume;
    uint8_t dropSoundTrack;
    unsigned long weatherUpdateInterval;
    bool cubeLight;  // Ambient cube lighting
    uint8_t cubeLightBrightness;  // 0-255
  };
  
  Settings currentSettings;
  
  SettingsManager() {
    // Initialize with default values from config.h
    loadDefaults();
  }
  
  // Load default values from config.h
  void loadDefaults() {
    currentSettings.manualSetpoint = MANUAL_SETPOINT;
    currentSettings.reactivateTemp = REACTIVATE_TEMP;
    currentSettings.durationGlacierFreezing = DURATION_GLACIER_FREEZING;
    currentSettings.reactivateTimer = REACTIVATE_TIMER;
    currentSettings.neopixelBrightness = NEOPIXEL_BRIGHTNESS;
    currentSettings.ledFadeTotalTime = LED_FADE_TOTAL_TIME;
    currentSettings.tempReadInterval = TEMP_READ_INTERVAL;
    currentSettings.audioVolume = AUDIO_PLAYER_VOLUME;
    currentSettings.dropSoundTrack = DROP_SOUND_TRACK;
    currentSettings.weatherUpdateInterval = WEATHER_UPDATE_INTERVAL;
    currentSettings.cubeLight = CUBE_LIGHT;
    currentSettings.cubeLightBrightness = CUBE_LIGHT_BRIGHTNESS;
  }
  
  // Initialize preferences - load from EEPROM or use defaults on first boot
  bool begin() {
    if (!preferences.begin(NAMESPACE, false)) {
      Serial.println("Failed to initialize Preferences");
      return false;
    }
    
    // Check if this is first boot
    bool isInitialized = preferences.getBool(INITIALIZED_KEY, false);
    
    if (!isInitialized) {
      Serial.println("First boot - initializing EEPROM with defaults from config.h");
      loadDefaults();
      saveToEEPROM();
      preferences.putBool(INITIALIZED_KEY, true);
    } else {
      Serial.println("Loading settings from EEPROM");
      loadFromEEPROM();
    }
    
    preferences.end();
    return true;
  }
  
  // Save current settings to EEPROM
  void saveToEEPROM() {
    preferences.begin(NAMESPACE, false);
    
    preferences.putFloat("setpoint", currentSettings.manualSetpoint);
    preferences.putFloat("reactivateT", currentSettings.reactivateTemp);
    preferences.putULong("freezeDur", currentSettings.durationGlacierFreezing);
    preferences.putULong("reactTimer", currentSettings.reactivateTimer);
    preferences.putUChar("neoBright", currentSettings.neopixelBrightness);
    preferences.putUShort("fadeDur", currentSettings.ledFadeTotalTime);
    preferences.putULong("tempInt", currentSettings.tempReadInterval);
    preferences.putUChar("audioVol", currentSettings.audioVolume);
    preferences.putUChar("dropTrack", currentSettings.dropSoundTrack);
    preferences.putULong("weatherInt", currentSettings.weatherUpdateInterval);
    preferences.putBool("cubeLight", currentSettings.cubeLight);
    preferences.putUChar("cubeBright", currentSettings.cubeLightBrightness);
    preferences.putBool(INITIALIZED_KEY, true);  // Mark as initialized
    
    preferences.end();
    Serial.println("Settings saved to EEPROM");
  }
  
  // Load settings from EEPROM
  void loadFromEEPROM() {
    preferences.begin(NAMESPACE, true); // Read-only mode
    
    currentSettings.manualSetpoint = preferences.getFloat("setpoint", MANUAL_SETPOINT);
    currentSettings.reactivateTemp = preferences.getFloat("reactivateT", REACTIVATE_TEMP);
    currentSettings.durationGlacierFreezing = preferences.getULong("freezeDur", DURATION_GLACIER_FREEZING);
    currentSettings.reactivateTimer = preferences.getULong("reactTimer", REACTIVATE_TIMER);
    currentSettings.neopixelBrightness = preferences.getUChar("neoBright", NEOPIXEL_BRIGHTNESS);
    currentSettings.ledFadeTotalTime = preferences.getUShort("fadeDur", LED_FADE_TOTAL_TIME);
    currentSettings.tempReadInterval = preferences.getULong("tempInt", TEMP_READ_INTERVAL);
    currentSettings.audioVolume = preferences.getUChar("audioVol", AUDIO_PLAYER_VOLUME);
    currentSettings.dropSoundTrack = preferences.getUChar("dropTrack", DROP_SOUND_TRACK);
    currentSettings.weatherUpdateInterval = preferences.getULong("weatherInt", WEATHER_UPDATE_INTERVAL);
    currentSettings.cubeLight = preferences.getBool("cubeLight", CUBE_LIGHT);
    currentSettings.cubeLightBrightness = preferences.getUChar("cubeBright", CUBE_LIGHT_BRIGHTNESS);
    
    preferences.end();
    
    Serial.println("Settings loaded from EEPROM:");
    printSettings();
  }
  
  // Reset to defaults and save
  void resetToDefaults() {
    Serial.println("Resetting to default values from config.h");
    loadDefaults();
    saveToEEPROM();
  }
  
  // Print current settings
  void printSettings() {
    Serial.printf("  Manual Setpoint: %.1f °C\n", currentSettings.manualSetpoint);
    Serial.printf("  Reactivate Temp: %.1f °C\n", currentSettings.reactivateTemp);
    Serial.printf("  Freeze Duration: %lu ms\n", currentSettings.durationGlacierFreezing);
    Serial.printf("  Reactivate Timer: %lu ms\n", currentSettings.reactivateTimer);
    Serial.printf("  LED Brightness: %d\n", currentSettings.neopixelBrightness);
    Serial.printf("  LED Fade Time: %d ms\n", currentSettings.ledFadeTotalTime);
    Serial.printf("  Temp Read Interval: %lu ms\n", currentSettings.tempReadInterval);
    Serial.printf("  Audio Volume: %d\n", currentSettings.audioVolume);
    Serial.printf("  Drop Sound Track: %d\n", currentSettings.dropSoundTrack);
    Serial.printf("  Weather Update Interval: %lu ms\n", currentSettings.weatherUpdateInterval);
    Serial.printf("  Cube Light: %s\n", currentSettings.cubeLight ? "ON" : "OFF");
    Serial.printf("  Cube Light Brightness: %d\n", currentSettings.cubeLightBrightness);
  }
};

#endif // SETTINGS_MANAGER_H
