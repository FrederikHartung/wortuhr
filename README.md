# German Word Clock (Wortuhr)

A beautiful word clock that displays time in German using a NeoPixel LED strip. Instead of showing numbers, it illuminates words to spell out the current time, like "ES IST ZEHN VOR HALB DREI" (It is ten to half past two).

## Features

- Displays time in German words using 114 NeoPixel LEDs
- Real-time clock with DS3231 RTC module for accurate timekeeping
- **Accurate German DST (Daylight Saving Time) handling**:
  - Automatic calculation of last Sunday in March and October
  - Precise 2:00 AM / 3:00 AM transition times
  - UTC+1 for winter time, UTC+2 for summer time
- Minute precision indicators with corner LEDs
- **Web-based configuration interface** with mDNS support (http://wortuhr.local)
- WiFi Access Point mode for easy setup
- Serial interface for time setting (alternative)
- Customizable LED brightness and colors

## Hardware Requirements

- ESP32 microcontroller
- 114x WS2812B NeoPixel LED strip
- DS3231 RTC module with backup battery
- 5V power supply (capacity depends on LED strip length and brightness)
- Breadboard and jumper wires for prototyping

### Wiring

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| NeoPixel Data | GPIO 23 | LED strip data input |
| DS3231 SDA | GPIO 21 | I2C data line |
| DS3231 SCL | GPIO 22 | I2C clock line |
| DS3231 VCC | 3.3V | RTC power |
| NeoPixel VCC | 5V | External power recommended for full strip |

## Software Setup

### Prerequisites

- Arduino IDE 1.8.x or newer
- ESP32 board package for Arduino IDE
- Required libraries (install via Arduino Library Manager) with all needed Dependencies:
  - "Adafruit_NeoPixel"
  - "RTClib"
  - WiFi (included with ESP32 board package)
  - WebServer (included with ESP32 board package)
  - ESPmDNS (included with ESP32 board package)

### Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/FrederikHartung/wortuhr.git
   cd wortuhr
   ```

2. Open `Wortuhr.ino` in Arduino IDE

3. Install required libraries:
   - Go to Tools → Manage Libraries
   - Search and install "Adafruit NeoPixel"
   - Search and install "RTClib"

4. Select your ESP32 board:
   - Tools → Board → ESP32 Arduino → ESP32 Dev Module

5. Configure upload settings:
   - Tools → Upload Speed → 921600
   - Tools → CPU Frequency → 240MHz (WiFi/BT)
   - Tools → Flash Size → 4MB

6. Connect your ESP32 and upload the code

## Usage

### First Setup

#### Option 1: Web Interface (Recommended)

1. **Power on the device**: The WiFi access point starts automatically
2. **Connect to WiFi**:
   - Network name: "Wortuhr"
   - Password: "wortuhr123"
3. **Open web browser**: Navigate to http://wortuhr.local or http://192.168.4.1
4. **Set the time**: Use the date and time picker to set the current time
5. **Auto-shutdown**: The web server automatically shuts down after 30 minutes to save power

#### Option 2: Serial Interface

1. **Set the time**: Open Serial Monitor (115200 baud) and send a 10-digit epoch timestamp to set the RTC time. You can get the current epoch time from [epochconverter.com](https://epochconverter.com).

2. **Test the display**: The clock will automatically start displaying the current time in German words.

### Serial Commands

- Send a 10-digit epoch timestamp to update the RTC time
- Send `"clear"` to turn off all LEDs

### Time Display Examples

- 14:00 → "ES IST ZWEI UHR"
- 14:05 → "ES IST FÜNF NACH ZWEI" 
- 14:15 → "ES IST VIERTEL NACH ZWEI"
- 14:30 → "ES IST HALB DREI"
- 14:45 → "ES IST DREIVIERTEL DREI"

## Configuration

### LED Settings
```cpp
#define NUM_LEDS 114        // Total number of LEDs
#define DATA_PIN 23         // GPIO pin for LED data
int brightnessRed = 0;      // Red brightness (0-255)
int brightnessGreen = 20;   // Green brightness (0-255)  
int brightnessBlue = 10;    // Blue brightness (0-255)
```

### Debug Mode
Uncomment `#define DEBUG` to enable detailed serial output for troubleshooting.

## LED Layout

The LEDs are arranged to form German time words. The layout includes:

- Main time words (EINS, ZWEI, DREI, etc.)
- Time indicators (UHR, VOR, NACH, HALB, VIERTEL)
- Minute precision dots (corner LEDs 110-113)

## Troubleshooting

### RTC Issues
- **"RTC lost power"**: Replace the backup battery (CR2032)
- **Incorrect time**: Send a new epoch timestamp via serial

### LED Issues
- **No LEDs lighting**: Check power supply and data pin connection
- **Wrong colors**: Verify wiring and LED strip type (WS2812B)
- **Partial lighting**: Check power supply capacity

### General Issues
- **Upload fails**: Try different USB cable or lower upload speed
- **Serial not working**: Ensure correct baud rate (115200)

## Development

### Code Structure

The project is organized into multiple modules:

- **Wortuhr.ino** - Main program entry point
- **config.h** - Hardware and configuration constants
- **clock_core.h/cpp** - Core timekeeping and LED display logic
- **web_interface.h/cpp** - Web server for time configuration
- **led_tests.h/cpp** - LED testing functions (optional)
- **utilities.h/cpp** - Utility functions (serial interface, etc.)

### Adding Features

The modular code structure makes it easy to modify:

- **Change LED layout**: Modify the LED number ranges in `clock_core.cpp`
- **Add animations**: Create new functions and call them in `loop()`
- **Web interface customization**: Edit `web_interface.cpp` for UI changes
- **WiFi settings**: Update credentials in `web_interface.h` or `config.h`

### Testing Functions

Several test functions are available:
- `showEverySecondLed()`: Alternating LED test pattern
- `showSpecificLed()`: Individual LED color test
- `showRunningLights()`: Moving light animation
- `testCornerLeds()`: Corner LED functionality test

## License

This project is open source. See [LICENSE](LICENSE) file for details.

## Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Test your changes thoroughly
4. Submit a pull request with a clear description

## Support

For questions or issues:
- Open a GitHub issue
- Check the serial monitor for debug information
- Verify hardware connections