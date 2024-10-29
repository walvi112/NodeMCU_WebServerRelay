#include "serial.h"
#include "relay.h"
#include "webserver.h"

Logger m_logger;

String data = "";

void serialInit(bool _log_on) 
{
  Serial.begin(9600);
  delay(200);
  if (Serial && _log_on) 
    m_logger.logOn();
}

void serialHandler(void)
{
  if (Serial.available())
  {
    data = Serial.readStringUntil('\n');
    Serial.print(">");
    Serial.println(data);
    //Log commands
    if (data.substring(0, 6) == "log on")
    {
      m_logger.logOn();
      logger()->println("Log is on.");
    }
    else if(data.substring(0, 7) == "log off")
    {      
      logger()->println("Log is off.");
      m_logger.logOff();
    }
    //Wifi set commands
    else if(data.substring(0, 9) == "wifi set ")
    {
      data.remove(0, 9);
      data.replace('\r', '\0');
      data.replace('\n', '\0');
      int8_t seperateIndex = data.indexOf('+');
      if (seperateIndex < 0)
      {
        logger()->println("Wrong format. Format should be: wifi set ssid pwd");
      }
      else
      {
        logger()->println("Setting wifi.");
        wifiSet(data.substring(0, seperateIndex).c_str(), data.substring(seperateIndex +1).c_str());
      }
    }
    //Relay commands
    else if(data.substring(0, 8) == "relay on")
    {
      relaySetState(ON);
    }
    else if(data.substring(0, 9) == "relay off")
    {
      relaySetState(OFF);
    }
    else if(data.substring(0, 12) == "relay status")
    {
      if(getRelayState())
        logger()->println("Relay is on.");
      else
        logger()->println("Relay is off.");
    }
    //Schedule commands
    else if(data.substring(0, 13) == "schedule add ")
    {
      if (data.charAt(15) != '/' ||  data.charAt(18) != '/' || data.charAt(20) != '/')
        logger()->println("Wrong format. Format should be: add schedule hour/min/state/day");
      else
        addSchedule(data.substring(13,15).toInt(), data.substring(16,18).toInt(), data.substring(21,23).toInt(), (RelayState) data.substring(19,20).toInt());
    }
    else if (data.substring(0, 16) == "schedule delete ")
    {
      data.remove(0, 16);
      deleteSchedule(data.toInt());
    }
    else if (data.substring(0, 12) == "schedule get")
    {
      logger()->println("Id/Hour/Min/State/Day");
      logSchedule();
    }
    //Reset
    else if (data.substring(0, 12) == "device reset")
    {
      logger()->println("Reset device.");
      ESP.reset();
    }
    return;
  }
}


Logger *logger()
{
    return &m_logger;
}