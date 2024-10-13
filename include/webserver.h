#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>

#define HTTP_PORT           80
#define HTTP_TIMEOUT_TIME   2000
#define WIFI_TIMEOUT        10000


#define MY_NTP_SERVER "at.pool.ntp.org"           
#define MY_TZ         "<+07>-7"   

extern time_t now;
extern struct tm tm;

bool wifiConnect(const char* ssid, const char* password, unsigned long wifiTimeout);
bool networkCheck(void);
void webServerInit(void);
void timeNTPInit(void);
void showTime(void);
void webServerHandler(void);

#endif