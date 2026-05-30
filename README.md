# German Word Clock (Wortuhr)

A German word clock for an ESP32, a DS3231 real-time clock module, and a 114 LED NeoPixel strip. Instead of showing numbers, it illuminates words to spell out the current time, for example "ES IST ZEHN VOR HALB DREI".

## Features

- Displays the time in German words using 114 NeoPixel LEDs
- Uses a DS3231 RTC module for timekeeping
- Handles German summer/winter time automatically
- Keeps the RTC internally in UTC and converts to German local time for display
- Shows extra minutes with four corner LEDs
- Starts a setup WiFi access point for 30 minutes after boot
- Hosts a small web interface at `http://wortuhr.local` or `http://192.168.4.1`
- Allows changing hour, minute, and second with plus/minus controls
- Supports serial time setting as an alternative
- Keeps all Arduino code in one `.ino` file for Arduino IDE compatibility

## Hardware Requirements

- ESP32 microcontroller, currently used with a Freenove ESP32 Wrover/Wroover style dev module
- 114x WS2812B / NeoPixel compatible LEDs
- DS3231 RTC module with backup battery
- 5V power supply sized for the LED strip
- Wiring/prototyping material

### Wiring

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| NeoPixel Data | GPIO 23 | LED strip data input |
| DS3231 SDA | GPIO 21 | I2C data line |
| DS3231 SCL | GPIO 22 | I2C clock line |
| DS3231 VCC | 3.3V | RTC power |
| NeoPixel VCC | 5V | External power recommended |

## Software Setup

### Prerequisites

- Arduino IDE
- ESP32 board package for Arduino IDE
- Required libraries:
  - `Adafruit_NeoPixel`
  - `RTClib`
- ESP32 built-in libraries:
  - `WiFi`
  - `WebServer`
  - `ESPmDNS`

### Installation

1. Clone this repository:

   ```bash
   git clone https://github.com/FrederikHartung/wortuhr.git
   cd wortuhr
   ```

2. Open `LED_Test/LED_Test.ino` in Arduino IDE.

3. Install the required libraries via Arduino Library Manager:
   - Search and install `Adafruit NeoPixel`
   - Search and install `RTClib`

4. Select your ESP32 board in Arduino IDE.

5. Upload the sketch to the ESP32.

## Usage

### Web Interface

After every ESP32 restart, the setup WiFi starts automatically for 30 minutes.

Connect to:

- Network name: `Saskia`
- Password: `wortuhr2026`

Then open:

- `http://wortuhr.local`
- or `http://192.168.4.1`

The page shows the current internal clock time and provides plus/minus controls for:

- Hour
- Minute
- Second

Each click immediately writes the adjusted time to the RTC. The web server, mDNS, and WiFi access point shut down automatically after 30 minutes.

### Serial Interface

Open Serial Monitor with `115200` baud.

Supported commands:

- Send a 10-digit UTC epoch timestamp to update the RTC directly.
- Send `clear` to clear the LEDs.

Important: the RTC is treated as UTC internally. The web interface converts German local time back to UTC before storing it.

## Time Handling

The sketch reads the DS3231 RTC as UTC. `getCurrentTime()` converts that UTC time into German local time:

- Winter time: UTC+1
- Summer time: UTC+2

Summer time is detected using the EU/Germany transition rules:

- Starts on the last Sunday in March at 01:00 UTC
- Ends on the last Sunday in October at 01:00 UTC

## Time Display Examples

- 14:00 -> "ES IST ZWEI UHR"
- 14:05 -> "ES IST FUENF NACH ZWEI"
- 14:15 -> "ES IST VIERTEL NACH ZWEI"
- 14:30 -> "ES IST HALB DREI"
- 14:45 -> "ES IST DREIVIERTEL DREI"

## Configuration

The project is currently intentionally kept as one Arduino sketch:

- `LED_Test/LED_Test.ino`

Relevant settings are near the top of the file:

```cpp
#define NUM_LEDS 114
#define DATA_PIN 23

const char *setupWifiSsid = "Saskia";
const char *setupWifiPassword = "wortuhr2026";
const char *mdnsHostname = "wortuhr";
const unsigned long setupWifiTimeoutMs = 30UL * 60UL * 1000UL;

int brightnessRed = 0;
int brightnessGreen = 20;
int brightnessBlue = 10;
```

Uncomment `#define DEBUG` to enable additional serial output.

## LED Layout

The LEDs are arranged to form German time words. The layout includes:

- Main time words such as `EINS`, `ZWEI`, `DREI`
- Time indicators such as `UHR`, `VOR`, `NACH`, `HALB`, `VIERTEL`
- Corner LEDs `110-113` for minutes within each 5-minute interval

## Test Functions

The sketch still contains helper functions for hardware checks:

- `showEverySecondLed()`
- `showSpecificLed()`
- `showRunningLights()`
- `letAllShine()`
- `testCornerLeds()`

They are not called by default in `loop()`.

## Troubleshooting

### WiFi/Web Interface

- If `wortuhr.local` does not open, try `http://192.168.4.1`.
- If no setup WiFi appears, restart the ESP32 and check within the first 30 minutes.
- If the page opens but time changes look wrong, check that the RTC was not manually set to local time via serial.

### RTC

- If Serial Monitor prints `RTC lost power`, check or replace the RTC backup battery.
- If the displayed time is wrong, use the web interface or send a UTC epoch timestamp via serial.

### LEDs

- If no LEDs light up, check power, ground, and GPIO 23 data wiring.
- If colors are wrong, verify the LED strip type and color order.
- If only part of the strip works, check solder joints and power capacity.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
