#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

#define HTTP_PORT           80
#define HTTP_TIMEOUT_TIME   2000

void wifiConnect(const char* ssid, const char* password);
void webServerInit(void);
void webServerHandler(void);

#endif