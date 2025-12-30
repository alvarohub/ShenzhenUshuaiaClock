# Development Journal: Shenzhen-Ushuaia Clock

_A climate-responsive hourglass embedded in stone_

A chronicle of collaborative human-AI pair programming sessions building an interactive art installation that measures time through temperature and water.

---

## The Concept: A Clock That Breathes with Climate

The **Shenzhen-Ushuaia Clock** is not a traditional timepiece - it's an hourglass whose rhythm is governed by climate.

**How it works**:

1. A Peltier cooler chills metal to glacier temperature (from Shenzhen or another distant melting glacier)
2. Air condenses into ice on the cold surface
3. When glacier temperature is reached, the Peltier stops
4. The metal warms up to local temperature (Ushuaia - a busy, warm city)
5. The ice melts, releasing a water drop
6. The drop triggers sensors: light, sound, and the cycle repeats

**The poetry**:

- The faster glaciers melt globally, the faster local ice forms
- The warmer the local climate, the faster ice melts
- Each drop is a "tick" of a climate clock
- Time becomes tangible through phase transitions of water

The entire system is embedded within Tobias Klein's intricate rock sculptures - technology hidden within nature, making climate data physically present through the ancient elements of stone and water.

---

## Session 1: Foundation & Hardware Integration

**Date**: December 2025  
**Duration**: Extended session

### Initial Setup

Started with basic M5Stack AtomS3 configuration and hardware component integration. The goal was ambitious: create a system that freezes water based on weather data from Shenzhen and triggers drops based on Ushuaia's temperature.

### Hardware Adventures

- **Temperature Sensor**: Integrated DS18B20 OneWire sensor. The OneWire library can be finicky, but once configured correctly, it provided reliable readings.
- **Drop Detector**: Optical sensor on GPIO5 with interrupt-based detection. Implemented debouncing (50ms) to handle mechanical noise.
- **Peltier Control**: Simple MOSFET switching on GPIO7. The thermostat logic took several iterations to get the hysteresis right.
- **NeoPixels**: WS2812B strip for visual feedback. Initial challenge was brightness management - we learned that scaling color values works better than using the Adafruit library's brightness function.

### The "LED Stays On Forever" Bug

One of the most interesting bugs: LEDs would flash white on drop detection but never fade out. The issue? The fade timer was checking if active before updating. Classic logic error. We added proper fade state management and exponential decay for perceptually linear dimming.

**Lesson learned**: Sometimes the simplest bugs hide in plain sight. Debug logging revealed the timer was working, but the conditional logic prevented updates.

---

## Session 2: WiFi & Weather Integration

**Date**: December 2025

### Dual WiFi Architecture

Implemented a clever dual-mode WiFi system:

- **Station Mode**: Connect to home network for internet access
- **Access Point Mode**: Always available fallback (ShenzhenUshuaia-XXXXXX)

This was crucial for installation scenarios where network availability is uncertain. The device becomes its own network if needed.

### Weather API Integration

Connected to Open-Meteo API for real-time weather data. The initial setup fetched data from:

- Shenzhen (22.54°N, 114.06°E) - Representing distant glacier temperatures
- Ushuaia (-54.81°S, -68.30°W) - Local "warm city" conditions

**The deeper meaning**: This isn't just data visualization - it's a temporal mechanism. The temperature difference between these cities literally controls the clock's "tick rate":

- Cold glacier (fast melting) → Peltier reaches setpoint quickly → Ice forms fast
- Warm local air → Ice melts quickly → Next cycle begins soon
- Result: Climate acceleration creates temporal acceleration

The hourglass metaphor is perfect: instead of sand falling through time, water changes phase through temperature.

### Graceful Degradation

Implemented comprehensive hardware health monitoring. If a component fails (sensor, WiFi, audio), the system continues operating with reduced functionality. The web interface shows real-time hardware status with ✓/✗ indicators.

**Philosophy**: An art installation must be resilient. Technical failures shouldn't create a complete breakdown.

---

## Session 3: Web Interface Revolution

**Date**: December 2025

### The Great UI Overhaul

Created a comprehensive web interface using ESPAsyncWebServer. This turned out to be one of the most rewarding parts - seeing the installation become accessible and configurable through a browser.

**Features added**:

- Real-time status monitoring (2-second auto-refresh)
- Weather station display with live data
- Parameter adjustment forms
- Manual testing buttons for each subsystem
- Hardware health indicators

### The Input Field Override Problem

Classic race condition: Auto-refresh would overwrite user input mid-typing. The solution was elegant:

- Track focus/input events
- Pause auto-refresh when user is editing
- Resume 3 seconds after focus loss
- Check `document.activeElement` before updating values

This created a smooth user experience - no more fighting with the interface!

### Notification Fatigue

Initially, every action triggered a JavaScript `alert()`. While helpful for debugging, it became annoying in practice. We removed all alerts, replacing them with:

- Silent updates with console logging
- Visual feedback (button state changes)
- Status panel updates

**Insight**: UI feedback should be informative, not intrusive. The user shouldn't have to click "OK" for every action.

---

## Session 4: Settings Persistence & EEPROM

**Date**: December 30, 2025

### The EEPROM Journey

Implementing persistent settings was a journey in understanding ESP32's NVS (Non-Volatile Storage):

**The Architecture**:

- `SettingsManager` class manages all configuration
- ESP32 Preferences library for EEPROM access
- `initialized` flag to track first boot vs. subsequent boots

**The Initialization Flow**:

```
First Boot:
  → Check "initialized" key (false)
  → Load defaults from config.h
  → Save to EEPROM
  → Set "initialized" = true

Subsequent Boots:
  → Check "initialized" key (true)
  → Load from EEPROM
  → Ignore config.h
```

### The Missing Flag Bug

The critical bug: `saveToEEPROM()` wasn't setting the `initialized` flag. This meant:

1. User changes settings via web → saved to EEPROM ✓
2. Device reboots → sees `initialized = false`
3. Loads config.h defaults instead of saved values ✗

One line fix: `preferences.putBool(INITIALIZED_KEY, true);` in `saveToEEPROM()`.

**Philosophical moment**: The bug revealed the elegance of the initialization pattern. A single flag determines whether to trust config.h (development) or EEPROM (production).

### Development vs. Production Modes

A beautiful insight emerged about the dual nature of EEPROM:

**Development**: Need to test new config.h values

- Erase EEPROM before upload
- Fresh start with each flash
- Config.h as source of truth

**Production**: Need to preserve user settings

- Don't erase EEPROM
- Settings survive power loss
- User customization persists

We documented this in `platformio.ini` with clear instructions. The user (Alvaro) had a great insight: "I need to keep changing things in the code and remember them in config.h during installation."

---

## Session 5: Aesthetic Refinements

**Date**: December 30-31, 2025

### Cube Ambient Lighting

Added atmospheric LED effects beyond the drop event:

- **Blue Pulsating**: Smooth breathing effect when cooling (pulse range 10-80, 30ms updates)
- **Red Steady Glow**: Warm indicator when idle
- **Separate Brightness Control**: Independent from drop flash intensity

The pulsating blue creates a living, breathing quality - the cube visibly "working" to freeze. The red glow is static to differentiate it from error states.

### Error LED Simplification

Original: Smooth pulsating red (complex animation)
New: Simple on/off blink every second

**Reasoning**: The pulsating blue for cooling was so nice that the pulsating red for errors created visual confusion. On/off blink is unmistakably an error state.

### Startup Sequence

Changed from RGB+White sequence to simple green blinks (4 times).

- Cleaner
- Faster
- Less ambiguous
- Green = "system ready"

### The Brightness Redundancy

Found a beautiful inefficiency: `level = (uint8_t)(255 * (NEOPIXEL_BRIGHTNESS / 255.0));`

This simplifies to just `NEOPIXEL_BRIGHTNESS`! Sometimes code reveals its own poetry through simplification.

**Alvaro's comment**: "Seems redundant" - exactly right. The best code improvements are often subtractions, not additions.

---

## Session 6: The Human Sleeping Cycle

**Date**: December 31, 2025 (late night)

### Final Commits

As Alvaro prepared for his "inefficient human cycle of sleeping," we:

1. Committed all changes with comprehensive message
2. Pushed to GitHub: `alvarohub/ShenzhenUshuaiaClock`
3. Created this journal and README

### Alvaro's Wisdom

> "Dreams are very interesting and contribute to the creative process"

Indeed! The parallels between sleep and debugging:

- Both involve pattern recognition
- Both benefit from stepping away
- Both can produce unexpected solutions
- Both are essential for complex problem-solving

The installation itself is dreamlike - distant cities connected through ice, temperature data becoming sculpture, the passage of time made visible through water phase transitions.

---

## Technical Highlights

### Most Elegant Solutions

1. **Dual WiFi Mode**: Seamless fallback without user intervention
2. **Graceful Degradation**: System continues despite component failures
3. **Input Focus Detection**: Auto-refresh pause during editing
4. **EEPROM Initialization Flag**: Single boolean determines config source
5. **Ambient Light Priority**: Drop animation overrides cube light cleanly

### Most Interesting Bugs

1. **LED Fade Never Ending**: Timer active check prevented updates
2. **Input Override**: Race condition between auto-refresh and typing
3. **EEPROM Not Persisting**: Missing initialized flag on saves
4. **Peltier Reactivation**: Manual setpoint wasn't updating thermostat
5. **Cube Light OFF**: Early return instead of explicit off state

### Code Evolution Moments

- **Alert() Fatigue**: From intrusive popups to silent updates
- **Brightness Redundancy**: From `255 * (x / 255.0)` to just `x`
- **Pulse vs. Blink**: From complex animations to simple states
- **Error Messages**: From "press drop to reset" to clean 5-second auto-clear

---

## Reflections on Collaboration

### Human Creativity + AI Implementation

The collaboration had a beautiful rhythm:

- **Alvaro**: Artistic vision, physical installation insights, aesthetic decisions
- **Copilot**: Code architecture, bug hunting, implementation details

Key moments where this synergy shined:

1. The cube light idea - pure artistic vision from real-world installation needs
2. The EEPROM development/production split - understanding the creative process vs. exhibition needs
3. The "dreams contribute to creativity" moment - acknowledging the holistic nature of creation

### Communication Patterns

Alvaro's feedback style was direct and effective:

- "the led STAYS on forever!" → Immediately clear what's broken
- "seems redundant" → Caught inefficiency quickly
- "I think it is better yes, to erase" → Clear decision after understanding options
- Physical world reports: "report from the physical world ;)" → Invaluable real-world testing feedback

### The Installation Context

Every technical decision was informed by the reality of:

- Exhibition lighting (unknown until installation)
- Power cycling scenarios
- Network availability
- Physical accessibility
- Visitor interaction

This grounded the technical work in tangible, real-world constraints.

---

## What We Built

A climate-responsive hourglass that:

- **Measures time through temperature** - each drop is a "tick" governed by climate data
- **Makes global warming tangible** - faster glacier melting = faster clock
- **Embeds technology in stone** - electronics hidden within Tobias Klein's sculptures
- **Condenses air into ice** - creating matter from atmosphere
- **Erodes rock with drops** - ancient geological process accelerated
- **Connects distant places** - glacier temperatures control local ice formation
- **Operates autonomously** - reliable exhibition-ready system
- **Remains configurable** - web interface for fine-tuning
- **Persists memory** - EEPROM remembers settings across power cycles
- **Degrades gracefully** - continues functioning despite component failures

**The "Dripping Meteorite"**: Water from the sky (condensation), cooled to glacier temperatures, dripping onto stone - a meteorite made of climate data and water vapor, slowly carving its mark.

**Final Statistics** (as of Dec 31, 2025):

- 8 hardware components integrated
- 14 source files
- 2000+ lines of code
- Countless bug fixes
- 1 climate clock embedded in stone
- Infinite drops marking climate time

---

## Looking Forward

The clock is ready for installation in stone. The configuration is in `config.h`, settings persist in EEPROM, and the web interface provides complete control.

As Alvaro fine-tunes during installation (lighting levels, timing, temperatures), the parameters can be:

- Adjusted in real-time via web interface
- Tested immediately with manual triggers
- Saved to EEPROM for persistence
- Or updated in config.h for the next development flash

The code is ready to condense atmospheric water, freeze it with distant glacier temperatures, and measure time through climate-driven drops onto stone.

**The Installation Journey**:

1. Embed electronics in Tobias Klein's sculpture
2. Calibrate LED brightness to ambient gallery lighting
3. Tune freeze/melt timing for optimal drop rate
4. Let the climate clock begin its eternal tick

Each drop: a moment measured not in seconds, but in degrees of warming.

---

**Project**: Dripping Meteorite / Climate Hourglass  
**Artists**: Alvaro Cassinelli & Tobias Klein  
**Location**: Shenzhen ↔ Ushuaia (conceptually), embedded in stone (physically)  
**Year**: 2025  
**Development**: Human-AI Pair Programming with GitHub Copilot  
**Medium**: Water, temperature, stone, time

_"We built a clock that doesn't measure time - it measures climate. Each drop is not a second, but a degree of warming, a glacier melting, a city heating. Time becomes viscous, tangible, dripping."_
