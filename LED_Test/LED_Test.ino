#include <Adafruit_NeoPixel.h>
#include <RTClib.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#define NUM_LEDS 114 // Count of LEDs
#define DATA_PIN 23  // ID of the Pin for data transfer from ESP32 to NeoPixlel LED strip
// #define DEBUG

const char *setupWifiSsid = "Saskia";
const char *setupWifiPassword = "wortuhr2026";
const char *mdnsHostname = "wortuhr";
const unsigned long setupWifiTimeoutMs = 30UL * 60UL * 1000UL;
const unsigned long clockUpdateIntervalMs = 5000UL;
const int wintertimeAdjustSeconds = 1 * 60 * 60; // UTC +1 hour
const int summerTimeAdjustSeconds = 2 * 60 * 60; // UTC +2 hour

Adafruit_NeoPixel pixels(NUM_LEDS, DATA_PIN, NEO_RGB + NEO_KHZ800);
RTC_DS3231 rtc;
WebServer server(80);
int brightnessRed = 0;
int brightnessGreen = 20;
int brightnessBlue = 10;
bool setupWifiActive = false;
unsigned long setupWifiStartedAt = 0;
unsigned long lastClockUpdateAt = 0;

DateTime getCurrentTime();
bool isGermanDST(const DateTime &utc);

void setLed(int number, int value_r, int value_g, int value_b)
{
  pixels.setPixelColor(number, pixels.Color(value_g, value_r, value_b, 0));
}

String twoDigits(int value)
{
  return value < 10 ? "0" + String(value) : String(value);
}

String formatDateTime(DateTime time)
{
  return twoDigits(time.day()) + "." + twoDigits(time.month()) + "." + String(time.year()) + " " +
         twoDigits(time.hour()) + ":" + twoDigits(time.minute()) + ":" + twoDigits(time.second());
}

DateTime convertGermanLocalTimeToUtc(const DateTime &local)
{
  DateTime summerCandidate(local.unixtime() - summerTimeAdjustSeconds);
  if (isGermanDST(summerCandidate))
  {
    return summerCandidate;
  }

  return DateTime(local.unixtime() - wintertimeAdjustSeconds);
}

void handleRoot()
{
  DateTime now = getCurrentTime();
  unsigned long elapsedMs = millis() - setupWifiStartedAt;
  unsigned long remainingSeconds = elapsedMs >= setupWifiTimeoutMs ? 0 : (setupWifiTimeoutMs - elapsedMs) / 1000UL;

  String html = R"(
<!doctype html>
<html lang="de">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Wortuhr</title>
  <style>
    body { margin: 0; font-family: Arial, sans-serif; background: #f5f5f5; color: #202124; }
    main { max-width: 420px; margin: 40px auto; padding: 24px; }
    h1 { margin: 0 0 24px; font-size: 28px; }
    .label { margin: 0 0 6px; color: #5f6368; font-size: 14px; }
    .time { margin: 0 0 20px; font-size: 30px; font-weight: 700; }
    .controls { display: grid; gap: 12px; margin: 24px 0; }
    .row { display: grid; grid-template-columns: 64px 1fr 64px; align-items: center; gap: 10px; }
    .value { min-height: 44px; display: grid; place-items: center; border: 1px solid #c9cdd2; border-radius: 6px; background: white; font-size: 22px; font-weight: 700; }
    form { margin: 0; }
    button { box-sizing: border-box; width: 100%; min-height: 44px; border: 0; border-radius: 6px; background: #1a73e8; color: white; font-size: 24px; font-weight: 700; }
    .hint { margin: 0; color: #5f6368; line-height: 1.4; }
  </style>
</head>
<body>
  <main>
    <h1>Wortuhr</h1>
    <p class="label">Aktuelle interne Uhrzeit</p>
    <p class="time">)" + formatDateTime(now) + R"(</p>
    <section class="controls">
      <p class="label">Uhrzeit anpassen</p>
      <div class="row">
        <form action="/adjusttime" method="post"><input type="hidden" name="field" value="hour"><input type="hidden" name="delta" value="-1"><button type="submit">-</button></form>
        <div class="value">Stunde )" + twoDigits(now.hour()) + R"(</div>
        <form action="/adjusttime" method="post"><input type="hidden" name="field" value="hour"><input type="hidden" name="delta" value="1"><button type="submit">+</button></form>
      </div>
      <div class="row">
        <form action="/adjusttime" method="post"><input type="hidden" name="field" value="minute"><input type="hidden" name="delta" value="-1"><button type="submit">-</button></form>
        <div class="value">Minute )" + twoDigits(now.minute()) + R"(</div>
        <form action="/adjusttime" method="post"><input type="hidden" name="field" value="minute"><input type="hidden" name="delta" value="1"><button type="submit">+</button></form>
      </div>
      <div class="row">
        <form action="/adjusttime" method="post"><input type="hidden" name="field" value="second"><input type="hidden" name="delta" value="-1"><button type="submit">-</button></form>
        <div class="value">Sekunde )" + twoDigits(now.second()) + R"(</div>
        <form action="/adjusttime" method="post"><input type="hidden" name="field" value="second"><input type="hidden" name="delta" value="1"><button type="submit">+</button></form>
      </div>
    </section>
    <p class="hint">WLAN bleibt noch ca. )" + String(remainingSeconds / 60UL) + R"( Minuten aktiv.</p>
  </main>
</body>
</html>
  )";

  server.send(200, "text/html; charset=utf-8", html);
}

void handleAdjustTime()
{
  if (!server.hasArg("field") || !server.hasArg("delta"))
  {
    server.send(400, "text/plain; charset=utf-8", "Feld oder Richtung fehlt.");
    return;
  }

  String field = server.arg("field");
  int delta = server.arg("delta").toInt();

  if (delta != -1 && delta != 1)
  {
    server.send(400, "text/plain; charset=utf-8", "Ungueltige Richtung.");
    return;
  }

  int deltaSeconds = 0;
  if (field == "hour")
  {
    deltaSeconds = delta * 60 * 60;
  }
  else if (field == "minute")
  {
    deltaSeconds = delta * 60;
  }
  else if (field == "second")
  {
    deltaSeconds = delta;
  }
  else
  {
    server.send(400, "text/plain; charset=utf-8", "Ungueltiges Feld.");
    return;
  }

  DateTime currentLocal = getCurrentTime();
  DateTime newLocal(currentLocal.unixtime() + deltaSeconds);
  DateTime newUtc = convertGermanLocalTimeToUtc(newLocal);
  rtc.adjust(newUtc);
  pixels.clear();
  pixels.show();

  Serial.print("time adjusted via web interface, local: ");
  Serial.println(formatDateTime(newLocal));
  Serial.print("stored utc: ");
  Serial.println(formatDateTime(newUtc));

  server.sendHeader("Location", "/");
  server.send(303, "text/plain", "");
}

void startSetupWifi()
{
  Serial.println("starting setup wifi...");
  WiFi.mode(WIFI_AP);

  bool started = WiFi.softAP(setupWifiSsid, setupWifiPassword);
  if (started)
  {
    setupWifiActive = true;
    setupWifiStartedAt = millis();

    Serial.println("setup wifi started");
    Serial.print("ssid: ");
    Serial.println(setupWifiSsid);
    Serial.print("ip: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", handleRoot);
    server.on("/adjusttime", HTTP_POST, handleAdjustTime);
    server.begin();
    Serial.println("web server started");

    if (MDNS.begin(mdnsHostname))
    {
      MDNS.addService("http", "tcp", 80);
      Serial.print("mdns started: http://");
      Serial.print(mdnsHostname);
      Serial.println(".local");
    }
    else
    {
      Serial.println("mdns could not be started");
    }

    Serial.println("setup wifi will stop automatically after 30 minutes");
  }
  else
  {
    setupWifiActive = false;
    Serial.println("setup wifi could not be started");
  }
}

void checkSetupWifiTimeout()
{
  if (setupWifiActive)
  {
    server.handleClient();
  }

  if (setupWifiActive && (millis() - setupWifiStartedAt >= setupWifiTimeoutMs))
  {
    Serial.println("setup wifi timeout reached, stopping wifi...");
    server.stop();
    MDNS.end();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    setupWifiActive = false;
    Serial.println("setup wifi stopped");
  }
}

void setup()
{
  Serial.begin(115200);

  // Pixel
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();
  pixels.show();

  // RTC
  bool rtcInitSuccessful = rtc.begin();
  if (rtcInitSuccessful)
  {
    Serial.println("rtc init successful");
  }
  else
  {
    Serial.println("rtc init not successful");
  }

  if (rtc.lostPower())
  {
    Serial.println("RTC lost power. Battery was removed or is empty.");
  }

  // DateTime time = DateTime(2022, 12, 22, 19, 11, 0);
  // rtc.adjust(time);
  startSetupWifi();
  // Wifi
  // Serial.println("server...");
  //  Responder of root page and apply page handled directly from WebServer class.
  // server.on("/", []() {
  // String content = R"(
  //<meta name="viewport" content="width=device-width, initial-scale=1.0">
  //<html>
  //<body>
  //<button onclick="window.location.href='http://wortuhr/_ac'">Wlan und Updates</button>
  //</body>
  // </html>
  //)";
  // server.send(200, "text/html", content);
  // });
  // Autoconnect configuration
  // Serial.println("auto config...");
  // config.ota = AC_OTA_BUILTIN;
  // config.autoReconnect = true;
  // config.title = "Wortuhr";
  // config.apid = "Wortuhr";
  // portal.config(config);
  // portal.begin();
  // Serial.println("get time...");
  // getTime();
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

void showEverySecondLed()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {

    if (i % 2 == 0)
    {
      setLed(i, 100, 0, 0);
    }
    else
    {
      setLed(i, 0, 0, 0);
    }
  }

  pixels.show();
  delay(500);

  for (int i = 0; i < NUM_LEDS; i++)
  {
    if (i % 2 == 0)
    {
      setLed(i, 0, 0, 0);
    }
    else
    {
      setLed(i, 100, 0, 0);
    }
  }

  pixels.show();
  delay(500);
}

void showSpecificLed()
{
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

void showRunningLights()
{
  setLed(posi1, 50, 0, 0);
  setLed(posi2, 0, 50, 0);
  setLed(posi3, 0, 0, 50);
  if (posi1 == 0)
  {
    setLed(NUM_LEDS - 1, 0, 0, 0);
  }
  else
  {
    setLed(posi1 - 1, 0, 0, 0);
  }

  pixels.show();

  posi1++;
  if (posi1 == NUM_LEDS)
  {
    posi1 = 0;
  }

  posi2++;
  if (posi2 == NUM_LEDS)
  {
    posi2 = 0;
  }

  posi3++;
  if (posi3 == NUM_LEDS)
  {
    posi3 = 0;
  }
}

void letAllShine()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    setLed(i, 10, 10, 10);
  }

  pixels.show();
}

void checkForTimeUpdate()
{
  Serial.println("waiting for time input...");
  if (Serial.available())
  {                                              // if there is data comming
    String input = Serial.readStringUntil('\n'); // read string until newline character
    // Dummy utc in epoch seconds time: 1672670580

    if (input.equals("clear"))
    {
      pixels.clear();
    }
    else if (input.length() == 10)
    {
      Serial.println("trying to adjust the rtc time to new value");
      rtc.adjust(DateTime(input.toInt()));
      pixels.clear();
    }
    else
    { // set imput as epoch seconds string
      Serial.println("unknown what to do with input: " + input);
    }
  }
}

bool isGermanDST(const DateTime &utc)
{
  int y = utc.year();

  // EU/Germany DST start: last Sunday in March at 01:00 UTC
  DateTime march31(y, 3, 31, 1, 0, 0);
  int lastSundayMarch = 31 - march31.dayOfTheWeek();
  DateTime dstStart(y, 3, lastSundayMarch, 1, 0, 0);

  // EU/Germany DST end: last Sunday in October at 01:00 UTC
  DateTime october31(y, 10, 31, 1, 0, 0);
  int lastSundayOctober = 31 - october31.dayOfTheWeek();
  DateTime dstEnd(y, 10, lastSundayOctober, 1, 0, 0);

  uint32_t t = utc.unixtime();
  return t >= dstStart.unixtime() && t < dstEnd.unixtime();
}

DateTime getCurrentTime()
{
  DateTime utc = rtc.now();
  int offset = isGermanDST(utc) ? summerTimeAdjustSeconds : wintertimeAdjustSeconds;
  return DateTime(utc.unixtime() + offset);
}

void setLedOff(int position)
{
  setLed(position, 0, 0, 0);
}

void setLedOn(int position)
{
  setLed(position, brightnessRed, brightnessGreen, brightnessBlue);
}

void printDateTime(DateTime time)
{
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
    // get time
    break;
  }

  switch (tm_min / 5)
  {

  case 0:
#if defined(DEBUG)
    Serial.println("tm_min / 5 = 0");
#endif

    for (int i = 106; i < 110; i++) // done
    {                               // Fünf
      setLedOff(i);
    }

    for (int i = 74; i < 77; i++) // done
    {                             // Vor
      setLedOff(i);
    }

    for (int i = 0; i < 3; i++) // done
    {                           // Uhr
      setLedOn(i);
    }

    break;

  case 1:
#if defined(DEBUG)
    Serial.println("tm_min / 5 = 1");
#endif

    for (int i = 0; i < 3; i++) // done
    {                           // Uhr
      setLedOff(i);
    }

    for (int i = 106; i < 110; i++) // done
    {                               // Fünf
      setLedOn(i);
    }

    for (int i = 66; i < 70; i++) // done
    {                             // Nach
      setLedOn(i);
    }
    break;
  case 2:
#if defined(DEBUG)
    Serial.println("tm_min / 5 = 2");
#endif

    for (int i = 106; i < 110; i++) // done
    {                               // Fünf
      setLedOff(i);
    }
    for (int i = 66; i < 70; i++) // done
    {                             // Nach
      setLedOff(i);
    }

    for (int i = 95; i < 99; i++) // done
    {                             // Zehn
      setLedOn(i);
    }
    for (int i = 66; i < 70; i++) // done
    {                             // Nach
      setLedOn(i);
    }
    break;

  case 3:
#if defined(DEBUG)
    Serial.println("tm_min / 5 = 3");
#endif

    for (int i = 95; i < 99; i++) // done
    {                             // Zehn
      setLedOff(i);
    }

    for (int i = 66; i < 70; i++) // done
    {                             // Nach
      setLedOff(i);
    }

    for (int i = 81; i < 88; i++) // done
    {                             // Viertel
      setLedOn(i);
    }

    break;

  case 4:
#if defined(DEBUG)
    Serial.println("tm_min / 5 = 4");
#endif

    for (int i = 81; i < 88; i++) // done
    {                             // Viertel
      setLedOff(i);
    }

    for (int i = 95; i < 99; i++) // done
    {                             // Zehn
      setLedOn(i);
    }

    for (int i = 74; i < 77; i++) // done
    {                             // Vor
      setLedOn(i);
    }

    for (int i = 55; i < 59; i++) // done
    {                             // Halb
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

    for (int i = 95; i < 99; i++) // done
    {                             // Zehn
      setLedOff(i);
    }

    for (int i = 74; i < 77; i++) // done
    {                             // Vor
      setLedOff(i);
    }

    for (int i = 55; i < 59; i++) // done
    {                             // Halb
      setLedOff(i);
    }

    for (int i = 106; i < 110; i++) // done
    {                               // Fünf
      setLedOn(i);
    }
    for (int i = 74; i < 77; i++) // done
    {                             // Vor
      setLedOn(i);
    }
    for (int i = 55; i < 59; i++) // done
    {                             // Halb
      setLedOn(i);
    }
    break;

  case 6:
#if defined(DEBUG)
    Serial.println("tm_min / 5 = 6");
#endif

    for (int i = 106; i < 110; i++) // done
    {                               // Fünf
      setLedOff(i);
    }
    for (int i = 74; i < 77; i++) // done
    {                             // Vor
      setLedOff(i);
    }

    for (int i = 55; i < 59; i++) // done
    {                             // Halb
      setLedOn(i);
    }
    break;
  case 7:
#if defined(DEBUG)
    Serial.println("tm_min / 5 = 7");
#endif

    for (int i = 106; i < 110; i++) // done
    {                               // Fünf
      setLedOn(i);
    }
    for (int i = 66; i < 70; i++) // done
    {                             // Nach
      setLedOn(i);
    }

    for (int i = 55; i < 59; i++) // done
    {                             // Halb
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
    for (int i = 106; i < 110; i++) // done
    {                               // Fünf
      setLedOff(i);
    }
    for (int i = 66; i < 70; i++) // done
    {                             // Nach
      setLedOff(i);
    }

    for (int i = 95; i < 99; i++) // done
    {                             // Zehn
      setLedOn(i);
    }
    for (int i = 66; i < 70; i++) // done
    {                             // Nach
      setLedOn(i);
    }
    for (int i = 55; i < 59; i++) // done
    {                             // Halb
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

    for (int i = 95; i < 99; i++) // done
    {                             // Zehn
      setLedOff(i);
    }
    for (int i = 66; i < 70; i++) // done
    {                             // Nach
      setLedOff(i);
    }
    for (int i = 55; i < 59; i++) // done
    {                             // Halb
      setLedOff(i);
    }

    for (int i = 77; i < 88; i++) // done
    {                             // Dreiviertel
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

    for (int i = 77; i < 88; i++) // done
    {                             // Dreiviertel
      setLedOff(i);
    }

    for (int i = 95; i < 99; i++) // done
    {                             // Zehn
      setLedOn(i);
    }
    for (int i = 74; i < 77; i++) // done
    {                             // Vor
      setLedOn(i);
    }
    break;

  case 11:
#if defined(DEBUG)
    Serial.println("tm_min / 5 = 11");
#endif

    for (int i = 106; i < 110; i++) // done
    {                               // Fünf
      setLedOn(i);
    }

    for (int i = 95; i < 99; i++) // done
    {                             // Zehn
      setLedOff(i);
    }
    for (int i = 74; i < 77; i++) // done
    {                             // Vor
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

    for (int i = 17; i < 22; i++) // done
    {                             // Zwölf
      setLedOff(i);
    }
    for (int i = 52; i < 55; i++) // done
    {                             // Ein
      setLedOn(i);
    }
    if (tm_min > 4)
    { // s for Eins
      setLedOn(51);
    }

    break;
  case 2:
#if defined(DEBUG)
    Serial.println("tm_hour = 2");
#endif

    for (int i = 51; i < 55; i++) // done
    {                             // Eins
      setLedOff(i);
    }
    for (int i = 44; i < 48; i++)
    { // Zwei
      setLedOn(i);
    }
    break;
  case 3:
#if defined(DEBUG)
    Serial.println("tm_hour = 3");
#endif

    for (int i = 44; i < 48; i++)
    { // Zwei
      setLedOff(i);
    }
    for (int i = 33; i < 37; i++)
    { // Drei
      setLedOn(i);
    }
    break;
  case 4:
    for (int i = 33; i < 37; i++)
    { // Drei
      setLedOff(i);
    }
    for (int i = 40; i < 44; i++)
    { // Vier
      setLedOn(i);
    }
    break;
  case 5:
    for (int i = 40; i < 44; i++)
    { // Vier
      setLedOff(i);
    }
    for (int i = 62; i < 66; i++)
    { // Fünf
      setLedOn(i);
    }
    break;
  case 6:
    for (int i = 62; i < 66; i++)
    { // Fünf
      setLedOff(i);
    }
    for (int i = 28; i < 33; i++)
    { // Sechs
      setLedOn(i);
    }
    break;
  case 7:
    for (int i = 28; i < 33; i++)
    { // Sechs
      setLedOff(i);
    }
    for (int i = 11; i < 17; i++)
    { // Sieben
      setLedOn(i);
    }
    break;
  case 8:

    for (int i = 11; i < 17; i++)
    { // Sieben
      setLedOff(i);
    }
    for (int i = 22; i < 26; i++)
    { // Acht
      setLedOn(i);
    }
    break;
  case 9:
    for (int i = 22; i < 26; i++)
    { // Acht
      setLedOff(i);
    }
    for (int i = 4; i < 8; i++)
    { // Neun
      setLedOn(i);
    }
    break;
  case 10:
    for (int i = 4; i < 8; i++)
    { // Neun
      setLedOff(i);
    }
    for (int i = 7; i < 11; i++)
    { // Zehn
      setLedOn(i);
    }
    break;
  case 11:
    for (int i = 7; i < 11; i++)
    { // Zehn
      setLedOff(i);
    }
    for (int i = 60; i < 63; i++)
    { // Elf
      setLedOn(i);
    }
    break;
  case 12:
#if defined(DEBUG)
    Serial.println("tm_hour = 12");
#endif

    for (int i = 60; i < 63; i++)
    { // Elf
      setLedOff(i);
    }
    for (int i = 17; i < 22; i++) // done
    {                             // Zwölf
      setLedOn(i);
    }
    break;
  }

  setLedOn(99);  // E
  setLedOn(100); // S
  setLedOn(102); // I
  setLedOn(103); // S
  setLedOn(104); // T

  pixels.show(); // Send the updated pixel colors to the hardware.
}

void testCornerLeds()
{
  setLed(113, 10, 0, 0);
  setLed(112, 10, 0, 0);
  setLed(111, 10, 0, 0);
  setLed(110, 10, 0, 0);
  pixels.show();
  delay(5000); // sleep 10 sec
  setLed(113, 0, 10, 0);
  setLed(112, 0, 10, 0);
  setLed(111, 0, 10, 0);
  setLed(110, 0, 10, 0);
  pixels.show();
  delay(5000); // sleep 10 sec
  setLed(113, 0, 0, 10);
  setLed(112, 0, 0, 10);
  setLed(111, 0, 0, 10);
  setLed(110, 0, 0, 10);
  pixels.show();
  delay(5000); // sleep 10 sec
}

void loop()
{
  checkSetupWifiTimeout();

  if (lastClockUpdateAt == 0 || millis() - lastClockUpdateAt >= clockUpdateIntervalMs)
  {
    lastClockUpdateAt = millis();
    checkForTimeUpdate();
    printDateTime(getCurrentTime());
    // letAllShine();
    displayCurrentTime();
    Serial.println("-----------------");
  }
}
