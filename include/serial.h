#ifndef SERIAL_H
#define SERIAL_H
#include <Arduino.h>

class Logger
{
    public:
        template<class T>  void print(T msg) 
        {
            if (log_on)
                Serial.print(msg);
        }

        template<class T>  void println(T msg) 
        {
            if (log_on)
                Serial.println(msg);
        }

        template<class T>  void printHEX(T msg) 
        {
            if (log_on)
            {
                String numHEX = msg;
                Serial.print("0x");
                for (int i = 0; i < numHEX.length(); i++) 
                {
                    Serial.print(numHEX.charAt(i), HEX);
                }
                Serial.println();
            }
        }

        void logOn() 
        {
            log_on = true;
        }
        void logOff() 
        {
            log_on = false;
        }
    private:
        bool log_on = false;

};

void serialInit(bool _log_on);
Logger *logger();

#endif