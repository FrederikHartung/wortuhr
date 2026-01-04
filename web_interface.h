#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include "config.h"

// Web server objects
extern WebServer server;
extern AutoConnect portal;
extern AutoConnectConfig config;

// Web server state
extern unsigned long webServerStartTime;
extern bool webServerActive;

// Function declarations
void startWebServer();
void handleRoot();
void handleSetTime();
void handleStatus();
void checkWebServerTimeout();

#endif