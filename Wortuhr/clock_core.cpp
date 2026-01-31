#include "clock_core.h"

// Hardware objects
Adafruit_NeoPixel pixels(NUM_LEDS, DATA_PIN, NEO_RGB + NEO_KHZ800);
RTC_DS3231 rtc;

// Configuration variables
int brightnessRed = 0;
int brightnessGreen = 20;
int brightnessBlue = 10;

// Timezone configuration
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
const char *ntpServer = "pool.ntp.org";
const unsigned long WEB_SERVER_TIMEOUT = 30 * 60 * 1000; // 30 minutes in milliseconds

// Timezone adjustment
int wintertimeAdjustSeconds = 1 * 60 * 60; //UTC +1 hour
int summerTimeAdjustSeconds = 2 * 60 * 60; //UTC +2 hour

// Calculate if current date/time is in summer time (DST) for Germany
// Returns true if summer time is active, false otherwise
bool isSummerTime(const DateTime& dt) {
  // Calculate last Sunday in March (DST starts at 2:00 AM)
  int startDay = (31 - (5 * dt.year() / 4 + 4) % 7);

  // Calculate last Sunday in October (DST ends at 3:00 AM)
  int endDay = (31 - (5 * dt.year() / 4 + 1) % 7);

  DateTime startSummerTime(dt.year(), 3, startDay, 2, 0, 0);  // Last Sunday in March, 2:00 AM
  DateTime endSummerTime(dt.year(), 10, endDay, 3, 0, 0);     // Last Sunday in October, 3:00 AM

  return (dt >= startSummerTime && dt < endSummerTime);
}

void initializeClock() {
  Serial.begin(115200);

  // Initialize NeoPixel strip
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Initialize RTC
  bool rtcInitSuccessful = rtc.begin();
  if(rtcInitSuccessful){
    Serial.println("rtc init successful");
  }
  else{
    Serial.println("rtc init not successful");
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power. Battery was removed or is empty.");
  }
}

void setLed(int number, int value_r, int value_g, int value_b){
  pixels.setPixelColor(number, pixels.Color(value_g, value_r, value_b, 0));
}

void setLedOn(int position){
  setLed(position, brightnessRed, brightnessGreen, brightnessBlue); 
}

void setLedOff(int position){
  setLed(position, 0, 0, 0); 
}

void clearAllLEDs() {
  pixels.clear();
  pixels.show();
}

void setRTCTime(int year, int month, int day, int hour, int minute, int second) {
  DateTime newTime(year, month, day, hour, minute, second);
  rtc.adjust(newTime);
}

DateTime getCurrentTime(){
  DateTime now = rtc.now();

  if(isSummerTime(now)){
    return DateTime(now.unixtime() + summerTimeAdjustSeconds);
  }
  else{
    return DateTime(now.unixtime() + wintertimeAdjustSeconds);
  }
}

void printDateTime(DateTime time){
  Serial.println("time:" + String(time.day()) + "-" + String(time.month()) + "-" + String(time.year()) + " " + String(time.hour()) + "h:" + String(time.minute()) + "m:" + String(time.second()) + "s");
}

void displayCurrentTime()
{
  DateTime now = getCurrentTime();

  int tm_min = now.minute();
  int tm_hour = now.hour();
  
  switch (tm_min % 5)
  { // Punkte min

  case 0:
    #if defined(DEBUG)
      Serial.println("0 minutes");
    #endif

    setLedOff(110);
    setLedOff(111);
    setLedOff(112);
    setLedOff(113);

    break;

  case 1:
    #if defined(DEBUG)
      Serial.println("1 minutes");
    #endif

    setLedOn(110);
    break;

  case 2:
    #if defined(DEBUG)
      Serial.println("2 minutes");
    #endif

    setLedOn(110);
    setLedOn(111);
    break;

  case 3:
    #if defined(DEBUG)
      Serial.println("3 minutes");
    #endif

    setLedOn(110);
    setLedOn(111);
    setLedOn(112);
    break;

  case 4:
    #if defined(DEBUG)
      Serial.println("4 minutes");
    #endif

    setLedOn(110);
    setLedOn(111);
    setLedOn(112);
    setLedOn(113);
    break;

  default:
    setLedOn(2);
    //get time
    break;
  }

  switch (tm_min / 5)
  {

  case 0:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 0");
    #endif

    for (int i = 106; i < 110; i++) //done
    { //Fünf
      setLedOff(i);
    }

    for (int i = 74; i < 77; i++) //done
    { //Vor
      setLedOff(i);
    }

    for (int i = 0; i < 3; i++) //done
    { //Uhr
      setLedOn(i);
    }

    break;

  case 1:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 1");
    #endif

    for (int i = 0; i < 3; i++) //done
    { //Uhr
      setLedOff(i);
    }

    for (int i = 106; i < 110; i++) //done
    { //Fünf
      setLedOn(i);
    }

    for (int i = 66; i < 70; i++) //done 
    { //Nach
      setLedOn(i);
    }
    break;
  case 2:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 2");
    #endif

    for (int i = 106; i < 110; i++)//done
    { //Fünf
      setLedOff(i);
    }
    for (int i = 66; i < 70; i++) //done 
    { //Nach
      setLedOff(i);
    }

    for (int i = 95; i < 99; i++) //done
    { // Zehn
      setLedOn(i);
    }
    for (int i = 66; i < 70; i++) //done
    { //Nach
      setLedOn(i);
    }
    break;

  case 3:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 3");
    #endif

    for (int i = 95; i < 99; i++) //done
    { // Zehn
      setLedOff(i);
    }
        
    for (int i = 66; i < 70; i++) //done
    { //Nach
      setLedOff(i);
    }

    for (int i = 81; i < 88; i++) //done
    { //Viertel
      setLedOn(i);
    }

    break;

  case 4:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 4");
    #endif

    for (int i = 81; i < 88; i++) //done
    { //Viertel
      setLedOff(i);
    }

    for (int i = 95; i < 99; i++) //done
    { // Zehn
      setLedOn(i);
    }

        for (int i = 74; i < 77; i++) //done
    { // Vor
      setLedOn(i);
    }

    for (int i = 55; i < 59; i++) //done
    { //Halb
      setLedOn(i);
    }

    break;

  case 5:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 5");
    #endif

    for (int i = 95; i < 99; i++) //done
    { // Zehn
      setLedOff(i);
    }

        for (int i = 74; i < 77; i++) //done
    { // Vor
      setLedOff(i);
    }

    for (int i = 55; i < 59; i++) //done
    { //Halb
      setLedOff(i);
    }

    for (int i = 106; i < 110; i++) //done
    { //Fünf
      setLedOn(i);
    }
    for (int i = 74; i < 77; i++) //done
    { // Vor
      setLedOn(i);
    }
    for (int i = 55; i < 59; i++) //done
    { //Halb
      setLedOn(i);
    }
    break;

  case 6:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 6");
    #endif

    for (int i = 106; i < 110; i++) //done
    { //Fünf
      setLedOff(i);
    }
    for (int i = 74; i < 77; i++) //done
    { // Vor
      setLedOff(i);
    }

    for (int i = 55; i < 59; i++) //done
    { //Halb
      setLedOn(i);
    }
    break;
  case 7:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 7");
    #endif

    for (int i = 106; i < 110; i++) //done
    { //Fünf
      setLedOn(i);
    }
    for (int i = 66; i < 70; i++) //done
    { //Nach
      setLedOn(i);
    }

    for (int i = 55; i < 59; i++) //done
    { //Halb
      setLedOn(i);
    }

    break;

  case 8:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 8");
    #endif

    for (int i = 106; i < 110; i++) //done
    { //Fünf
      setLedOff(i);
    }
    for (int i = 66; i < 70; i++) //done
    { //Nach
      setLedOff(i);
    }

    for (int i = 95; i < 99; i++) //done
    { // Zehn
      setLedOn(i);
    }
      for (int i = 66; i < 70; i++) //done 
    { //Nach
      setLedOn(i);
    }
    for (int i = 55; i < 59; i++) //done
    { //Halb
      setLedOn(i);
    }

    break;

  case 9:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 9");
    #endif

    for (int i = 95; i < 99; i++) //done
    { // Zehn
      setLedOff(i);
    }
      for (int i = 66; i < 70; i++) //done 
    { //Nach
      setLedOff(i);
    }
    for (int i = 55; i < 59; i++) //done
    { //Halb
      setLedOff(i);
    }

    for (int i = 77; i < 88; i++) //done
    { //Dreiviertel
      setLedOn(i);
    }
    break;

  case 10:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 10");
    #endif

    for (int i = 77; i < 88; i++) //done
    { //Dreiviertel
      setLedOff(i);
    }

    for (int i = 95; i < 99; i++) //done
    { //Zehn
      setLedOn(i);
    }
    for (int i = 74; i < 77; i++) //done
    { // Vor
      setLedOn(i);
    }
    break;

  case 11:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 11");
    #endif

    for (int i = 106; i < 110; i++) //done
    { //Fünf
      setLedOn(i);
    }

    for (int i = 95; i < 99; i++) //done
    { //Zehn
      setLedOff(i);
    }
    for (int i = 74; i < 77; i++) //done
    { // Vor
      setLedOn(i);
    }

    break;

  default:
      setLedOn(2);
    break;
  }

  if (tm_hour > 12)
  {
    tm_hour = tm_hour - 12;
  }
  
  if (tm_min > 14) //??
  {
    #if defined(DEBUG)
      Serial.println("tm_min > 14 = true");
    #endif

    if (tm_hour == 12)
    {
    #if defined(DEBUG)
      Serial.println("tm_hour == 12");
    #endif

      tm_hour = 1;
    }
    else
    {
    #if defined(DEBUG)
      Serial.println("tm_hour != 12");
    #endif
      tm_hour = tm_hour + 1;
    }
  }

  switch (tm_hour)
  {
  case 1:
    #if defined(DEBUG)
      Serial.println("tm_hour = 1");
    #endif

    for (int i = 17; i < 22; i++) //done
    { //Zwölf
      setLedOff(i);
    }
    for (int i = 52; i < 55; i++) //done
    { //Ein
      setLedOn(i);
    }
    if (tm_min > 4)
    { //s for Eins
      setLedOn(51);
    }

    break;
  case 2:
    #if defined(DEBUG)
      Serial.println("tm_hour = 2");
    #endif

    for (int i = 51; i < 55; i++) //done
    { //Eins
      setLedOff(i);
    }
    for (int i = 44; i < 48; i++)
    { //Zwei
      setLedOn(i);
    }
    break;
  case 3:
    #if defined(DEBUG)
      Serial.println("tm_hour = 3");
    #endif

    for (int i = 44; i < 48; i++)
    { //Zwei
      setLedOff(i);
    }
    for (int i = 33; i < 37; i++)
    { //Drei
      setLedOn(i);
    }
    break;
  case 4:
    for (int i = 33; i < 37; i++)
    { //Drei
      setLedOff(i);
    }
    for (int i = 40; i < 44; i++)
    { //Vier
      setLedOn(i);
    }
    break;
  case 5:
    for (int i = 40; i < 44; i++)
    { //Vier
      setLedOff(i);
    }
    for (int i = 62; i < 66; i++)
    { //Fünf
      setLedOn(i);
    }
    break;
  case 6:
    for (int i = 62; i < 66; i++)
    { //Fünf
      setLedOff(i);
    }
    for (int i = 28; i < 33; i++)
    { //Sechs
      setLedOn(i);
    }
    break;
  case 7:
    for (int i = 28; i < 33; i++)
    { //Sechs
      setLedOff(i);
    }
    for (int i = 11; i < 17; i++)
    { //Sieben
      setLedOn(i);
    }
    break;
  case 8:

    for (int i = 11; i < 17; i++)
    { //Sieben
      setLedOff(i);
    }
    for (int i = 22; i < 26; i++)
    { //Acht
      setLedOn(i);
    }
    break;
  case 9:
    for (int i = 22; i < 26; i++)
    { //Acht
      setLedOff(i);
    }
    for (int i = 4; i < 8; i++)
    { //Neun
      setLedOn(i);
    }
    break;
  case 10:
    for (int i = 4; i < 8; i++)
    { //Neun
      setLedOff(i);
    }
    for (int i = 7; i < 11; i++)
    { //Zehn
      setLedOn(i);
    }
    break;
  case 11:
    for (int i = 7; i < 11; i++)
    { //Zehn
      setLedOff(i);
    }
    for (int i = 60; i < 63; i++)
    { //Elf
      setLedOn(i);
    }
    break;
  case 12:
    #if defined(DEBUG)
      Serial.println("tm_hour = 12");
    #endif

    for (int i = 60; i < 63; i++)
    { //Elf
      setLedOff(i);
    }
    for (int i = 17; i < 22; i++) //done
    { //Zwölf
      setLedOn(i);
    }
    break;
  }

  setLedOn(99); //E
  setLedOn(100); //S
  setLedOn(102); //I
  setLedOn(103); //S
  setLedOn(104); //T  
  
  pixels.show();   // Send the updated pixel colors to the hardware.
}