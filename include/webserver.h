#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include "relay.h"

#define HTTP_PORT           80
#define HTTP_TIMEOUT_TIME   2000
#define WIFI_TIMEOUT        10000


#define MY_NTP_SERVER "at.pool.ntp.org"           
#define MY_TZ         "<+07>-7"   

#define SCHEDULE_SIZE       36


typedef struct schedule
{
    RelayState status; //relay status 
    uint8_t hour;  // hours since midnight  0-23
    uint8_t minutes;  // minutes after the hour  0-59
    uint8_t wday;  // 00000000
} schedule;

bool wifiConnect(const char* ssid, const char* password, unsigned long wifiTimeout);
bool networkCheck(void);
bool addSchedule(uint8_t hour, uint8_t minutes, uint8_t wday, RelayState status);
void webServerInit(void);
void timeNTPInit(void);
void showTime(void);
void webServerHandler(void);

#endif