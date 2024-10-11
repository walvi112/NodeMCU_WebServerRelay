#include "serial.h"

Logger m_logger;

void serialInit(bool _log_on) 
{
  Serial.begin(9600);
  delay(200);
  if (Serial && _log_on) 
    m_logger.logOn();
}

Logger *logger()
{
    return &m_logger;
}