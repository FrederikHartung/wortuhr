/*
 * German Word Clock (Wortuhr)
 * 
 * A beautiful word clock that displays time in German using NeoPixel LEDs.
 * Features web interface for time setting and automatic 30-minute shutdown.
 * 
 * Hardware:
 * - ESP32 microcontroller
 * - 114x WS2812B NeoPixel LED strip on pin 23
 * - DS3231 RTC module (I2C)
 * 
 * Web Interface:
 * - Starts automatically on power-up
 * - Access via http://wortuhr.local or displayed IP
 * - Shuts down after 30 minutes to save power
 */

#include "config.h"
#include "clock_core.h"
#include "web_interface.h"
#include "utilities.h"

// Uncomment to enable LED test functions
// #include "led_tests.h"

void setup() {
  // Initialize core clock functionality
  initializeClock();
  
  // Start web server for time configuration
  startWebServer();
  webServerStartTime = millis();
  webServerActive = true;
  
  Serial.println("Wortuhr initialized successfully!");
  Serial.println("Web interface available for 30 minutes");
}

void loop() {
  // Handle web server if active
  if (webServerActive) {
    portal.handleRequest();
    checkWebServerTimeout();
  }
  
  // Check for serial time updates (legacy support)
  checkForTimeUpdate();
  
  // Display current time and debug info
  printDateTime(getCurrentTime());
  // displayCurrentTime();  // Commented out for web server testing
  
  // Update every 5 seconds
  delay(5000);
  Serial.println("-----------------");
  
  // Uncomment these lines to run LED tests
  // showEverySecondLed();
  // showSpecificLed();
  // showRunningLights();
  // letAllShine();
  // testCornerLeds();
}