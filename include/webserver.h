#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include "relay.h"

#define HTTP_PORT           80
#define HTTP_TIMEOUT_TIME   2000
#define WIFI_TIMEOUT        10000
#define HTTP_BODY_BUFFER    200
#define HTTP_HEADER_BUFFER  700

#define MY_NTP_SERVER      "at.pool.ntp.org"           
#define MY_TZ              "<+07>-7"   

#define SCHEDULE_SIZE       36

typedef enum RequestType
{
  GET_R,
  GET_R_ON,
  GET_R_OFF,
  GET_SCHEDULE_DISPLAY,
  POST_SCHEDULE_SET,
  POST_SCHEDULE_DELETE,
} RequestType;


typedef struct schedule
{
    RelayState status; //relay status 
    int hour;  // hours since midnight  0-23
    int minutes;  // minutes after the hour  0-59
    int wday;  // 00000000 0sftwtms 
} schedule;


extern bool timeSet;
extern TickTwo timerSchedule;

bool wifiConnect(const char* ssid, const char* password, unsigned long wifiTimeout);
bool networkCheck(void);
void wifiSet(const char* _ssid, const char* _pwd);
bool addSchedule(uint8_t hour, uint8_t minutes, uint8_t wday, RelayState status);
bool deleteSchedule(uint8_t index);
void logSchedule();
void updateNextSchedule();
void webServerInit(void);
void timeNTPInit(void);
void showTime(void);
void webServerHandler(void);
struct tm getTime(void);

#endif