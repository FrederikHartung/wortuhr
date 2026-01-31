#include "web_interface.h"
#include "clock_core.h"
#include <RTClib.h>

// Web server objects
WebServer server(80);

// Web server state
unsigned long webServerStartTime = 0;
bool webServerActive = false;

void startWebServer() {
  Serial.println("Starting WiFi and Web Server...");

  // Start WiFi in Access Point mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

  // Print connection info
  IPAddress IP = WiFi.softAPIP();
  Serial.println("Access Point started");
  Serial.print("AP SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("Access via: http://");
  Serial.println(IP);
  Serial.println("Default IP is usually: http://192.168.4.1");

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/settime", HTTP_POST, handleSetTime);
  server.on("/status", handleStatus);

  // Start the web server
  server.begin();
  webServerActive = true;
  webServerStartTime = millis();

  // Start mDNS responder
  if (MDNS.begin("wortuhr")) {
    Serial.println("mDNS responder started");
    Serial.println("Access via: http://wortuhr.local");
  } else {
    Serial.println("Error setting up mDNS responder!");
  }

  Serial.println("Web server started successfully");
}

void handleRoot() {
  DateTime now = getCurrentTime();
  
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wortuhr - Zeiteinstellung</title>
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
        <h1>🕐 Wortuhr Konfiguration</h1>
        
        <div class="time-display">
            <div>Aktuelle Uhrzeit:</div>
            <div class="current-time">)" + String(now.day()) + "." + String(now.month()) + "." + String(now.year()) + R"( )" + 
            String(now.hour()) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute()) + ":" + (now.second() < 10 ? "0" : "") + String(now.second()) + R"(</div>
        </div>
        
        <form action="/settime" method="POST">
            <h3>Neue Uhrzeit einstellen:</h3>
            <input type="date" name="date" value=")" + String(now.year()) + "-" + (now.month() < 10 ? "0" : "") + String(now.month()) + "-" + (now.day() < 10 ? "0" : "") + String(now.day()) + R"(" required>
            <input type="time" name="time" value=")" + (now.hour() < 10 ? "0" : "") + String(now.hour()) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute()) + R"(" required>
            <button type="submit">Uhrzeit setzen</button>
        </form>
        
        <div class="status">
            Der Webserver wird automatisch in <span id="remainingTime">)" + String((WEB_SERVER_TIMEOUT - (millis() - webServerStartTime)) / 60000) + R"(</span> Minuten heruntergefahren
        </div>
    </div>

    <script>
        // Update remaining time every minute
        setInterval(async function() {
            try {
                const response = await fetch('/status');
                const data = await response.json();
                const remainingMinutes = Math.floor(data.remaining_seconds / 60);
                document.getElementById('remainingTime').textContent = remainingMinutes;
            } catch (error) {
                console.error('Error updating time:', error);
            }
        }, 60000); // Update every 60 seconds
    </script>
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
    setRTCTime(year, month, day, hour, minute, 0);
    
    Serial.println("Time updated via web interface:");
    Serial.println("Date: " + dateStr + " Time: " + timeStr);
    
    // Clear LEDs and redirect
    clearAllLEDs();
    
    String response = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta http-equiv="refresh" content="2; url=/">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Uhrzeit erfolgreich gesetzt</title>
    <style>
        body { font-family: Arial; text-align: center; margin: 50px; background: #f0f0f0; }
        .success { background: #d4edda; color: #155724; padding: 20px; border-radius: 10px; display: inline-block; }
    </style>
</head>
<body>
    <div class="success">
        <h2>✅ Uhrzeit erfolgreich gesetzt!</h2>
        <p>Neue Uhrzeit: )" + dateStr + " " + timeStr + R"(</p>
        <p>Wird weitergeleitet...</p>
    </div>
</body>
</html>
    )";
    
    server.send(200, "text/html", response);
  } else {
    server.send(400, "text/plain", "Fehlende Datums- oder Zeitangabe");
  }
}

void handleStatus() {
  unsigned long remainingTime = (WEB_SERVER_TIMEOUT - (millis() - webServerStartTime)) / 1000;

  String json = "{";
  json += "\"remaining_seconds\": " + String(remainingTime);
  json += "}";

  server.send(200, "application/json", json);
}

void checkWebServerTimeout() {
  if (webServerActive) {
    // Handle client requests
    server.handleClient();

    // Check for timeout
    if ((millis() - webServerStartTime) > WEB_SERVER_TIMEOUT) {
      Serial.println("Web server timeout reached - shutting down to save power");
      server.stop();
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_OFF);
      webServerActive = false;
      Serial.println("WiFi and web server stopped");
    }
  }
}