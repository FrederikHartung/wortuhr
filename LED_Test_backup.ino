#include <Adafruit_NeoPixel.h>
#include <RTClib.h>

#define NUM_LEDS 114 //Count of LEDs
#define DATA_PIN 23 //ID of the Pin for data transfer from ESP32 to NeoPixlel LED strip
//#define DEBUG

Adafruit_NeoPixel pixels(NUM_LEDS, DATA_PIN, NEO_RGB + NEO_KHZ800);
RTC_DS3231 rtc;
int brightnessRed = 0;
int brightnessGreen = 20;
int brightnessBlue = 10;

#include <time.h>
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
#include <WiFi.h>
#include <WebServer.h>
WebServer server;
#include <AutoConnect.h>
AutoConnect portal(server);
AutoConnectConfig config;

// Web server timing
unsigned long webServerStartTime = 0;
const unsigned long WEB_SERVER_TIMEOUT = 30 * 60 * 1000; // 30 minutes in milliseconds
bool webServerActive = false;
bool UpdateChecked = false;

void setLed(int number, int value_r, int value_g, int value_b){
  pixels.setPixelColor(number, pixels.Color(value_g, value_r, value_b, 0));
}

void startWebServer() {
  Serial.println("Starting WiFi and Web Server...");
  
  // Configure AutoConnect
  config.ota = AC_OTA_BUILTIN;
  config.autoReconnect = true;
  config.title = "Wortuhr";
  config.apid = "Wortuhr";
  portal.config(config);
  
  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/settime", HTTP_POST, handleSetTime);
  server.on("/status", handleStatus);
  
  // Start the portal
  if (portal.begin()) {
    Serial.println("Web server started successfully");
    Serial.print("Access via: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to start web server");
  }
}

void handleRoot() {
  DateTime now = getCurrentTime();
  
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wortuhr - Time Setting</title>
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 400px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        h1 { color: #333; text-align: center; }
        .time-display { background: #e8f4f8; padding: 15px; border-radius: 5px; text-align: center; margin: 20px 0; }
        .current-time { font-size: 24px; color: #2c5282; font-weight: bold; }
        input, button { width: 100%; padding: 10px; margin: 5px 0; border-radius: 5px; border: 1px solid #ccc; }
        button { background: #4a90e2; color: white; border: none; cursor: pointer; }
        button:hover { background: #357abd; }
        .status { margin-top: 20px; padding: 10px; background: #f8f8f8; border-radius: 5px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🕐 Wortuhr Configuration</h1>
        
        <div class="time-display">
            <div>Current Time:</div>
            <div class="current-time">)" + String(now.day()) + "." + String(now.month()) + "." + String(now.year()) + R"( )" + 
            String(now.hour()) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute()) + ":" + (now.second() < 10 ? "0" : "") + String(now.second()) + R"(</div>
        </div>
        
        <form action="/settime" method="POST">
            <h3>Set New Time:</h3>
            <input type="date" name="date" value=")" + String(now.year()) + "-" + (now.month() < 10 ? "0" : "") + String(now.month()) + "-" + (now.day() < 10 ? "0" : "") + String(now.day()) + R"(" required>
            <input type="time" name="time" value=")" + (now.hour() < 10 ? "0" : "") + String(now.hour()) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute()) + R"(" required>
            <button type="submit">Set Time</button>
        </form>
        
        <div class="status">
            <strong>Status:</strong><br>
            Web server will shut down automatically in )" + String((WEB_SERVER_TIMEOUT - (millis() - webServerStartTime)) / 60000) + R"( minutes<br>
            LED Count: )" + String(NUM_LEDS) + R"(<br>
            RTC Status: Active
        </div>
        
        <button onclick="window.location.href='/status'">Refresh Status</button>
    </div>
</body>
</html>
  )";
  
  server.send(200, "text/html", html);
}

void handleSetTime() {
  if (server.hasArg("date") && server.hasArg("time")) {
    String dateStr = server.arg("date");
    String timeStr = server.arg("time");
    
    // Parse date (YYYY-MM-DD)
    int year = dateStr.substring(0, 4).toInt();
    int month = dateStr.substring(5, 7).toInt();
    int day = dateStr.substring(8, 10).toInt();
    
    // Parse time (HH:MM)
    int hour = timeStr.substring(0, 2).toInt();
    int minute = timeStr.substring(3, 5).toInt();
    
    // Set the RTC
    DateTime newTime(year, month, day, hour, minute, 0);
    rtc.adjust(newTime);
    
    Serial.println("Time updated via web interface:");
    Serial.println("Date: " + dateStr + " Time: " + timeStr);
    
    // Clear LEDs and redirect
    pixels.clear();
    pixels.show();
    
    String response = R"(
<!DOCTYPE html>
<html>
<head>
    <meta http-equiv="refresh" content="2; url=/">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Time Set Successfully</title>
    <style>
        body { font-family: Arial; text-align: center; margin: 50px; background: #f0f0f0; }
        .success { background: #d4edda; color: #155724; padding: 20px; border-radius: 10px; display: inline-block; }
    </style>
</head>
<body>
    <div class="success">
        <h2>✅ Time Set Successfully!</h2>
        <p>New time: )" + dateStr + " " + timeStr + R"(</p>
        <p>Redirecting back...</p>
    </div>
</body>
</html>
    )";
    
    server.send(200, "text/html", response);
  } else {
    server.send(400, "text/plain", "Missing date or time parameter");
  }
}

void handleStatus() {
  DateTime now = getCurrentTime();
  unsigned long remainingTime = (WEB_SERVER_TIMEOUT - (millis() - webServerStartTime)) / 1000;
  
  String json = "{";
  json += "\"current_time\": \"" + String(now.day()) + "." + String(now.month()) + "." + String(now.year()) + " " + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()) + "\",";
  json += "\"remaining_seconds\": " + String(remainingTime) + ",";
  json += "\"led_count\": " + String(NUM_LEDS) + ",";
  json += "\"rtc_status\": \"active\",";
  json += "\"wifi_ip\": \"" + WiFi.localIP().toString() + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

void checkWebServerTimeout() {
  if (webServerActive && (millis() - webServerStartTime) > WEB_SERVER_TIMEOUT) {
    Serial.println("Web server timeout reached - shutting down to save power");
    server.stop();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    webServerActive = false;
    Serial.println("WiFi and web server stopped");
  }
}

void setup() {
  Serial.begin(115200);

  //Pixel
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();
  pixels.show();

  //RTC
  bool rtcInitSuccessful = rtc.begin();
  if(rtcInitSuccessful){
    Serial.println("rtc init successful");
  }
  else{
    Serial.println("rtc init not successful");
  }

  if (rtc.lostPower())
  {
    Serial.println("RTC lost power. Battery was removed or is empty.");
  }

  //DateTime time = DateTime(2022, 12, 22, 19, 11, 0);
  //rtc.adjust(time);
  
  // Start WiFi and Web Server
  startWebServer();
  webServerStartTime = millis();
  webServerActive = true;
}

/*
void getTime()
{
  Serial.println("config time...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("config time done");

  delay(1000);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("could not get time try 1");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(4000);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(3000);
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("could not get time try 2");
    }
    else{
      Serial.println("get time try 2 successfull");
    }
  }
  else{
      Serial.println("get time try 1 successfull");
  }
}
*/


void showEverySecondLed(){
  for (int i = 0; i < NUM_LEDS; i++) {

    if(i % 2 == 0){
      setLed(i, 100, 0, 0);
    }
    else{
      setLed(i, 0, 0, 0);
    }
  }

  pixels.show();
  delay(500);

  for (int i = 0; i < NUM_LEDS; i++) {
   if(i % 2 == 0){
      setLed(i, 0, 0, 0);
    }
   else{
      setLed(i, 100, 0, 0);
    }
  }

  pixels.show();
  delay(500);
}

void showSpecificLed(){
  setLed(0, 100, 0, 0);
  setLed(1, 0, 100, 0);
  setLed(2, 0, 0, 100);
  setLed(3, 100, 100, 100);
  pixels.show();
  delay(2000);
  pixels.clear();

  setLed(0, 0, 0, 0);
  setLed(2, 0, 0, 0);
  setLed(2, 0, 0, 0);
  setLed(3, 0, 0, 0);
  pixels.show();
  delay(500);
}



int posi1 = 0;
int posi2 = 1;
int posi3 = 2;

void showRunningLights(){
  setLed(posi1, 50, 0, 0);
  setLed(posi2, 0, 50, 0);
  setLed(posi3, 0, 0, 50);
  if(posi1 == 0){
   setLed(NUM_LEDS - 1, 0, 0, 0);  
  }
  else{
    setLed(posi1 - 1, 0, 0, 0); 
  }

  pixels.show();

  posi1++;
  if(posi1 == NUM_LEDS){
    posi1 = 0;
  }

  posi2++;
  if(posi2 == NUM_LEDS){
    posi2 = 0;
  }

  posi3++;
  if(posi3 == NUM_LEDS){
    posi3 = 0;
  }
}

void letAllShine(){
  for(int i = 0; i < NUM_LEDS; i++){
      setLed(i, 10, 10, 10);
  }

    pixels.show();
}

void checkForTimeUpdate(){
  Serial.println("waiting for time input...");
  if (Serial.available()) { // if there is data comming
    String input = Serial.readStringUntil('\n'); // read string until newline character
    //Dummy utc in epoch seconds time: 1672670580

    if(input.equals("clear")){
        pixels.clear();
    }
    else if(input.length() == 10){
      Serial.println("trying to adjust the rtc time to new value");
      rtc.adjust(DateTime(input.toInt()));
      pixels.clear();
    }
    else{ //set imput as epoch seconds string
    Serial.println("unknown what to do with input: " + input);
    }
  }
}

int wintertimeAdjustSeconds = 1 * 60 * 60; //UTC +1 hour
int summerTimeAdjustSeconds = 2 * 60 * 60; //UTC +2 hour

DateTime getCurrentTime(){
  DateTime now = rtc.now();
  int currentMonth = now.month();

  if(currentMonth > 4 && currentMonth < 10){ //summertime in germany from april to november
    return DateTime(now.unixtime() + summerTimeAdjustSeconds);
  }
  else{
    return DateTime(now.unixtime() + wintertimeAdjustSeconds);
  }
}

void setLedOff(int position){
  setLed(position, 0, 0, 0); 
}

void setLedOn(int position){
  setLed(position, brightnessRed, brightnessGreen, brightnessBlue); 
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

/*
    for (int i = 88; i < 95; i++) //done
    { //Zwanzig
      setLedOn(i);
    }

    for (int i = 66; i < 70; i++) //done
    { //Nach
      setLedOn(i);
    }
*/
    break;

  case 5:
    #if defined(DEBUG)
      Serial.println("tm_min / 5 = 5");
    #endif

/*
    for (int i = 66; i < 70; i++) //done
    { //Nach
      setLedOff(i);
    }

    for (int i = 88; i < 95; i++) //done
    { //Zwanzig
      setLedOff(i);
    }
*/

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

/*
    for (int i = 88; i < 95; i++)
    { //Zwanzig
      setLedOn(i);
    }
      for (int i = 74; i < 77; i++) //done
    { // Vor
      setLedOn(i);
    }
    */
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

/*
    for (int i = 88; i < 95; i++) //done
    { //Zwanzig
      setLedOff(i);
    }
    for (int i = 81; i < 88; i++) //done
    { //Viertel
      setLedOn(i);
    }
    for (int i = 74; i < 77; i++) //done
    { // Vor
      setLedOn(i);
    }
    */

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

/*
    for (int i = 81; i < 88; i++) //done
    { //Viertel
      setLedOff(i);
    }
*/

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

void testCornerLeds(){
setLed(113, 10, 0, 0); 
  setLed(112, 10, 0, 0); 
  setLed(111, 10, 0, 0); 
  setLed(110, 10, 0, 0); 
  pixels.show();
  delay(5000); //sleep 10 sec
  setLed(113, 0, 10, 0); 
  setLed(112, 0, 10, 0); 
  setLed(111, 0, 10, 0); 
  setLed(110, 0, 10, 0); 
  pixels.show();
  delay(5000); //sleep 10 sec
  setLed(113, 0, 0, 10); 
  setLed(112, 0, 0, 10); 
  setLed(111, 0, 0, 10); 
  setLed(110, 0, 0, 10); 
  pixels.show();
  delay(5000); //sleep 10 sec
}


void loop() {
  // Handle web server if active
  if (webServerActive) {
    portal.handleRequest();
    checkWebServerTimeout();
  }
  
  checkForTimeUpdate();
  printDateTime(getCurrentTime());
  //letAllShine();
  displayCurrentTime(); 
  delay(5000);
  Serial.println("-----------------");
}

