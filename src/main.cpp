#include <Arduino.h>
#include "relay.h"
#include "serial.h"
#include "webserver.h"
#include "rom.h"

#define LOG_ON          1


void setup() {
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), relayButtonPressIRQ, CHANGE);
  serialInit(LOG_ON);
  romInit();
  relayInit(RELAY_TYPE); 
  timeNTPInit();
  wifiConnect(romReadSSID(), romReadPassword(), WIFI_TIMEOUT);
  updateNextSchedule();
  
}

void loop() {
  timerSchedule.update();
  relayButtonHandler();
  serialHandler();

  if(networkCheck())
    webServerHandler();
}

