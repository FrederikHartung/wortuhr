#ifndef CLOCK_CORE_H
#define CLOCK_CORE_H

#include <Adafruit_NeoPixel.h>
#include <RTClib.h>
#include "config.h"

// Hardware objects
extern Adafruit_NeoPixel pixels;
extern RTC_DS3231 rtc;

// Function declarations
void initializeClock();
void setLed(int number, int value_r, int value_g, int value_b);
void setLedOn(int position);
void setLedOff(int position);
void clearAllLEDs();
void setRTCTime(int year, int month, int day, int hour, int minute, int second);
DateTime getCurrentTime();
void displayCurrentTime();
void printDateTime(DateTime time);

#endif