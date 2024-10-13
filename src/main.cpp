#include <Arduino.h>
#include "relay.h"
#include "serial.h"
#include "webserver.h"

#define LOG_ON            1

const char* ssid     = " ";
const char* password = " ";


void setup() {
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), relayButtonPressIRQ, CHANGE);
  serialInit(LOG_ON);
  relayInit(RELAY_TYPE); 
  timeNTPInit();
  if (wifiConnect(ssid, password, WIFI_TIMEOUT))
    webServerInit();

}

void loop() {
  relayButtonHandler();
  if(networkCheck())
    webServerHandler();

}

