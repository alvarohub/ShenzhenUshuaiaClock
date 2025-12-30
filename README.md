# Shenzhen-Ushuaia Clock / A climate-responsive hourglass embedded in stone.

Alvaro Cassinelli & Tobias Klein

A clock whose tick rate is controlled by the temperature difference between two cities; an interactive art installation that works like an hour glass that slows down or accelerate depending on the local climate, compared to the temperature of a distant glacier. The device condenses water from air and create droplets that "erode" a natural rock, controlled by real-time weather data from Ushuaia. The entire system is embedded within Tobias Klein's intricate rock sculptures - technology hidden within nature, making climate data physically present through the ancient elements of stone and water.

## Concept

The system uses a peltier to cool a piece of metal at the temperature of a glacier melting fast somewhere in the world. When reached, it stops and the temperature rises until it reaches the local temperature (of a busy, warm city). This melts the ice formed by condensation, triggering a water drop that falls through an optical sensor, accompanied by sound and LED effects and the process repeat by activating the peltier.

## Hardware

- **MCU**: M5Stack AtomS3 (ESP32-S3)
- **Cooling**: Peltier thermoelectric cooler with MOSFET control
- **Temperature Sensing**: DS18B20 waterproof temperature sensor (OneWire)
- **Drop Detection**: Optical sensor (IR break-beam)
- **LED Feedback**: WS2812B NeoPixel strip (8 LEDs)
- **Audio**: M5Stack Audio Player Unit
- **Connectivity**: WiFi (dual mode: Station + Access Point)

## Features

### Temperature Control

- Automatic temperature regulation using weather data from Open-Meteo API
- Configurable setpoint (manual or linked to weather stations)
- Hysteresis-based control to prevent rapid cycling
- Reactivation timer for periodic ice reformation

### Visual Feedback

- **Ambient Cube Light**:
  - Blue pulsating glow when actively cooling
  - Red steady glow when idle/warming
  - Configurable brightness (0-255)
- **Drop Event**: White flash that fades over configurable duration
- **Startup**: Green blink sequence (4 times)
- **Error**: Red on/off blink (1 second cycle)

### Web Interface

- Real-time system monitoring (temperature, Peltier state, drop count)
- Weather station data display (Shenzhen & Ushuaia)
- Live parameter adjustment with EEPROM persistence
- Manual testing controls (drop trigger, LED, audio, Peltier)
- System pause/resume functionality
- Reset to defaults from config.h
- Dual WiFi mode: Station (home network) + Access Point (direct connection)
- Auto-refresh with intelligent pause during user input

### Settings Persistence

- All parameters saved to EEPROM (ESP32 NVS)
- Settings survive power cycles and reboots
- Web interface for runtime configuration
- Development mode: erase flash to reload config.h defaults
- Production mode: preserve user settings across updates

## Configuration

Key parameters in `src/config.h`:

```cpp
// WiFi
#define WIFI_SSID       "YourNetwork"
#define WIFI_PASSWORD   "YourPassword"
#define WIFI_ENABLED    true

// Thermostat
#define MANUAL_SETPOINT -1.0         // °C - glacier temperature
#define REACTIVATE_TEMP 20.0          // °C - melting trigger temperature
#define DURATION_GLACIER_FREEZING 10000  // ms - hold time at setpoint
#define REACTIVATE_TIMER 1800000      // ms - auto-restart interval

// LEDs
#define NEOPIXEL_BRIGHTNESS 255       // 0-255 - drop flash brightness
#define LED_FADE_TOTAL_TIME 1500      // ms - fade duration
#define CUBE_LIGHT true               // Enable ambient lighting
#define CUBE_LIGHT_BRIGHTNESS 128     // 0-255 - ambient glow brightness
```

## Web Interface

Access the web interface at:

- Station mode: `http://<device-ip>/` (check serial monitor for IP)
- AP mode: `http://192.168.4.1/` (SSID: "ShenzhenUshuaia-XXXXXX", password: "meteorite123")

Features:

- Real-time hardware status indicators
- Live weather data from Shenzhen and Ushuaia
- Adjustable parameters with instant preview
- Manual test controls for all subsystems
- Settings automatically saved to EEPROM

## Development

### Prerequisites

- PlatformIO IDE (VS Code extension)
- ESP32 board support

### Building and Uploading

**Development Mode** (reload config.h on each flash):

```bash
# Erase EEPROM first
~/.platformio/penv/bin/platformio run --target erase

# Then upload
platformio run --target upload
```

**Production Mode** (preserve EEPROM settings):

```bash
platformio run --target upload
```

### Serial Monitor

```bash
platformio device monitor --baud 115200
```

## API Endpoints

### Status

- `GET /api/status` - JSON with all system state

### Control

- `POST /api/update` - Update parameters (form data)
- `POST /api/drop` - Trigger drop event
- `POST /api/peltier/toggle` - Force Peltier on/off
- `POST /api/system/toggle` - Pause/resume system
- `POST /api/test/led` - Test LEDs (5 seconds)
- `POST /api/test/audio` - Test audio playback
- `POST /api/reset-defaults` - Reset to config.h and restart

## Project Structure

```
ShenzhenUshuaiaClock/
├── src/
│   ├── main.cpp                 # Main program loop
│   ├── config.h                 # Configuration parameters
│   ├── Thermostat.h/cpp         # Temperature control logic
│   ├── TemperatureSensor.h/cpp  # DS18B20 interface
│   ├── DropDetector.h/cpp       # Optical sensor handling
│   ├── NeoPixelController.h/cpp # LED animations
│   ├── AudioPlayer.h/cpp        # M5 Audio Unit interface
│   ├── WiFiManager.h/cpp        # WiFi + weather API
│   ├── WebInterface.h/cpp       # HTTP server + web UI
│   ├── SettingsManager.h/cpp    # EEPROM persistence
│   └── WeatherStation.h         # Weather data structures
├── platformio.ini               # PlatformIO configuration
└── README.md                    # This file
```

## Weather Data

The system uses the [Open-Meteo API](https://open-meteo.com/) to fetch real-time weather data:

- **Shenzhen** (22.54°N, 114.06°E) - Glacier temperature source
- **Ushuaia** (-54.81°S, -68.30°W) - Local monitoring

Updates every 5 minutes (configurable).

## Troubleshooting

### WiFi Connection Failed

- Check credentials in `config.h`
- Device creates AP mode automatically: "ShenzhenUshuaia-XXXXXX"
- Connect to AP and configure via web interface

### Temperature Sensor Not Detected

- Check OneWire connection (pin G6)
- System operates with graceful degradation
- Web interface shows hardware status

### EEPROM Not Persisting

- Ensure `INITIALIZED_KEY` flag is set (automatic after first save)
- Use "Reset to Defaults" if needed
- Development mode erases EEPROM on each upload

### LEDs Not Working

- Check NeoPixel connection (pin G8)
- Verify power supply (WS2812B requires 5V)
- Adjust brightness if needed (NEOPIXEL_BRIGHTNESS)

## License

This project is part of the AWSOME_ROCKS art installation collaboration between Alvaro Cassinelli and Tobias Klein.

## Acknowledgments

- Weather data: [Open-Meteo API](https://open-meteo.com/)
- Hardware: M5Stack ecosystem
- Development: Collaborative human-AI pair programming with GitHub Copilot

---

**Live Installation**: Shenzhen ↔ Ushuaia  
**Project**: Shenzhen-Ushuaia Clock
**Year**: 2025
