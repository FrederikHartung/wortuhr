#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "config.h"

// Web server objects
extern WebServer server;

// Web server state
extern unsigned long webServerStartTime;
extern bool webServerActive;

// WiFi credentials - set these in config.h or define them here
#ifndef WIFI_SSID
#define WIFI_SSID "Wortuhr"  // Default AP name
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "wortuhr123"  // Default AP password (min 8 chars)
#endif

// Function declarations
void startWebServer();
void handleRoot();
void handleSetTime();
void handleStatus();
void checkWebServerTimeout();

#endif