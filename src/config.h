#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// PIN CONFIGURATION FOR ATOMS3
// ============================================
// Note: Grove Port A pins are obtained dynamically using M5.getPin()
// in the AudioPlayer class (port_a_pin1 and port_a_pin2).

// Drop detector (optical sensor)
#define PIN_DROP_DETECTOR G5

// Dallas temperature sensor (OneWire)
#define PIN_TEMPERATURE G6

// NeoPixel LED strip
#define PIN_NEOPIXEL G8

// Peltier cooler control (MOSFET)
#define PIN_PELTIER G7




// ============================================
// SYSTEM CONFIGURATION
// ============================================

// WiFi credentials:
//const char* ssid =;//YOUR_WIFI_SSID";
//const char* password = "aobi9897";//"YOUR_WIFI_PASSWORD";

#define WIFI_SSID       "AndroidAPda3a"
#define WIFI_PASSWORD   "aobi9897"
#define WIFI_ENABLED    false  // Set to false to run without WiFi (uses preset values)
#define WIFI_RETRY_INTERVAL 60000  // Retry WiFi connection every 60 seconds if failed

// Web Server settings
#define WEBSERVER_ENABLED true           // Enable web interface
#define WEBSERVER_AP_PASSWORD "meteorite123"  // Access Point password (min 8 chars)

// Thermostat settings
#define THERMOSTAT_HYSTERESIS 0.5  // °C (deprecated - using REACTIVATE_TEMP instead)
#define MANUAL_SETPOINT -1.0       // °C - Default glacier temperature when not linked to a station
#define REACTIVATE_TEMP 16.0       // °C - Temperature to restart cooling after ice melts (should be higher than the dew point at the local station). 
#define DURATION_GLACIER_FREEZING 10000 // 900000  // milliseconds - Keep cooling after reaching glacier temp. For instance, 5 minutes = 300000 ms. For more ice, we can keep it longer, for instance 15 minutes = 900000 ms.
#define REACTIVATE_TIMER 900000     // milliseconds - Auto-restart cooling after this time. Example: 30 minutes is 1800000 ms. 15 min is 900000 ms.

// Drop detector settings
#define DROP_DEBOUNCE_MS 50       // milliseconds
#define DROP_TRIGGER_MODE FALLING  // RISING, FALLING, or CHANGE

// NeoPixel settings
#define NEOPIXEL_COUNT 8//64           // Number of LEDs
#define NEOPIXEL_BRIGHTNESS 255     // 0-255
#define NEOPIXEL_TEST_MODE false   // Set to true to show constant green for testing
#define LED_FADE_TOTAL_TIME 1500  // milliseconds - Total fade duration (255 to 1)
#define CUBE_LIGHT true  // Ambient light: blue pulse when cooling, red glow when off
#define CUBE_LIGHT_BRIGHTNESS 100  // 0-255 - Brightness for ambient cube light (independent of drop flash)

// Temperature sensor settings
#define TEMP_SENSOR_RESOLUTION 12  // 9-12 bits
#define TEMP_READ_INTERVAL 5000     // milliseconds - How often to read temperature sensor

// Audio Player settings
#define AUDIO_PLAYER_BAUD_RATE 9600  // Serial baud rate for audio module
#define AUDIO_PLAYER_VOLUME 30       // Default volume (value range: 0-30)
#define DROP_SOUND_TRACK 1           // Track number for drop sound effect

// Weather update interval
#define WEATHER_UPDATE_INTERVAL 300000  // 5 minutes in milliseconds

#endif // CONFIG_H
