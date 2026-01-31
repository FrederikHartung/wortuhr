#ifndef CONFIG_H
#define CONFIG_H

// Hardware Configuration
#define NUM_LEDS 114 //Count of LEDs
#define DATA_PIN 23 //ID of the Pin for data transfer from ESP32 to NeoPixel LED strip

// Debug Configuration
//#define DEBUG

// LED Brightness Configuration
extern int brightnessRed;
extern int brightnessGreen;
extern int brightnessBlue;

// Timezone Configuration
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;
extern const char *ntpServer;

// Web Server Configuration
extern const unsigned long WEB_SERVER_TIMEOUT; // 30 minutes in milliseconds

#endif