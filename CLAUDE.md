# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview
This is an Arduino project for a German word clock (Wortuhr) that displays time using a 114-LED NeoPixel strip. The clock shows time in German words like "ES IST FÜNF VOR HALB ZWEI" (It is five to half past one) and includes minute indicator dots. The code is now organized into multiple modules for better maintainability.

## Hardware Setup
- **114 NeoPixel LEDs** (WS2812B strip) connected to pin 23
- **DS3231 RTC module** for timekeeping with backup battery
- **ESP32 microcontroller** as the main processor
- LED arrangement forms German words for time display

## Code Structure

The project is organized into multiple modules:

### Core Files
- **Wortuhr.ino** - Main program entry point and loop
- **config.h** - Hardware and configuration constants
- **clock_core.h/cpp** - Core timekeeping and LED display logic
- **web_interface.h/cpp** - Web server for time configuration
- **led_tests.h/cpp** - LED testing functions (optional)
- **utilities.h/cpp** - Utility functions (serial interface, etc.)

### Key Functions
- `displayCurrentTime()` - Primary time display logic with German word mapping (clock_core.cpp)
- `setLed(number, r, g, b)` - Sets individual LED colors (note: parameter order is number, red, green, blue, but internally uses GRB format)
- `getCurrentTime()` - Gets RTC time with German timezone adjustment (UTC+1/+2)
- `isSummerTime(dt)` - Accurately calculates if a given DateTime is in DST (clock_core.cpp)
- `startWebServer()` - Initializes web interface with 30-minute timeout and mDNS support
- `checkForTimeUpdate()` - Serial interface for time setting via epoch seconds (utilities.cpp)

### LED Layout
LEDs are mapped to German time words:
- LEDs 0-2: "UHR" (o'clock)
- LEDs 4-7: "NEUN" (nine)
- LEDs 7-10: "ZEHN" (ten)
- LEDs 11-16: "SIEBEN" (seven)
- LEDs 17-21: "ZWÖLF" (twelve)
- LEDs 22-25: "ACHT" (eight)
- And so on for all German time words...

Corner LEDs 110-113 indicate minutes within each 5-minute interval.

## Development Commands
This is an Arduino project - use Arduino IDE or PlatformIO for compilation and upload:

### Arduino IDE
- Open `Wortuhr.ino` in Arduino IDE
- Select ESP32 board type
- Upload via USB connection

### PlatformIO (if used)
```bash
pio run          # Compile
pio run -t upload # Upload to device
```

## Configuration
- `NUM_LEDS`: Set to 114 for the current LED strip
- `DATA_PIN`: Pin 23 for data connection
- `DEBUG`: Uncomment to enable serial debugging output
- Default brightness: Red=0, Green=20, Blue=10

## Time Setting

### Web Interface (Recommended)
- Connect to WiFi access point "Wortuhr" (password: "wortuhr123")
- Access via http://wortuhr.local or http://192.168.4.1
- Web server automatically starts on power-up and shuts down after 30 minutes
- Features:
  - German language interface
  - Date and time picker for easy configuration
  - Auto-refreshing timeout countdown
  - mDNS support for friendly hostname
  - UTF-8 encoded for proper emoji display

### Serial Interface (Alternative)
Send epoch timestamp (10 digits) via Serial at 115200 baud to update RTC time.
Send "clear" via Serial to turn off all LEDs.

## Architecture Notes
- Time display uses switch statements for 5-minute intervals (tm_min/5) and individual minutes (tm_min%5)
- German timezone handling with automatic summer/winter time adjustment:
  - Accurate DST calculation using last Sunday of March (2:00 AM) and October (3:00 AM)
  - `isSummerTime()` function correctly determines DST status for any date/time
  - UTC+1 for winter time, UTC+2 for summer time
- Web interface automatically starts on power-up and shuts down after 30 minutes
- LED color order internally uses GRB but setLed function expects RGB parameters

## Development Guidelines

### Adding Features
- **Clock modifications**: Edit `clock_core.cpp` for time display logic
- **Web interface changes**: Modify `web_interface.cpp` for configuration pages
- **Hardware settings**: Update `config.h` for pin assignments and constants
- **Testing**: Uncomment LED test includes in main .ino file

### Code Organization
- Keep production code separate from test functions
- Use appropriate modules for different functionality
- Test functions are in `led_tests.cpp` and disabled by default
- All configuration values are centralized in `config.h`

### Backup
- Original unorganized code is saved as `LED_Test_backup.ino`

## Project Naming
- Main file: `Wortuhr.ino`
- Project directory should be named `Wortuhr` (Arduino requires folder name to match .ino filename)