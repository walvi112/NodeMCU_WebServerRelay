#include <Arduino.h>
#include "relay.h"
#include "serial.h"
#include "webserver.h"

#define LOG_ON            1
#define RELAY_TYPE        NPN

const char* ssid     = " ";
const char* password = " ";


void setup() {
  serialInit(LOG_ON);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), relayButtonPressIRQ, CHANGE);
  relayInit(RELAY_TYPE); 
  wifiConnect(ssid, password);
  webServerInit();
}

void loop() {
  relayButtonHandler();
  webServerHandler();
  delay(1);
}

