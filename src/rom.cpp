#include <EEPROM.h>
#include "rom.h"
#include "serial.h"

static uint8_t romRelayState;
static uint8_t romNbSchedule;
static schedule romSchedules[SCHEDULE_SIZE];
static char romSSID[33] =  {'\0'};
static char romPassword[17] =  {'\0'};

void romInit()
{
    EEPROM.begin(EEP_SIZE);

    romReadState();
    romReadNbSchedule();
    romReadSchedules();
    romReadSSID();
    romReadPassword();

    if (romRelayState != 0 && romRelayState != 1)
    {
        romWriteState(0);
        romReadState();
    }

    if (romNbSchedule > SCHEDULE_SIZE)
    {
        romWriteNbSchedule(0);
        romReadNbSchedule();
    }

}

//Read functions
uint8_t romReadState()
{
    EEPROM.get(EEP_STATE, romRelayState);
    return romRelayState;
}

uint8_t romReadNbSchedule()
{
    EEPROM.get(EEP_NB_SCHEDULES, romNbSchedule);
    return romNbSchedule;
}

schedule* romReadSchedules()
{
    EEPROM.get(EEP_SCHEDULES, romSchedules);
    return romSchedules;
}

char* romReadSSID()
{
    EEPROM.get(EEP_SSID, romSSID);
    return romSSID;
}

char* romReadPassword()
{
    EEPROM.get(EEP_PWD, romPassword);
    return romPassword;
}

//Write functionss
void romWriteState(uint8_t state)
{
    romRelayState = state;
    EEPROM.put(EEP_STATE, state);
    EEPROM.commit();
}

void romWriteNbSchedule(uint8_t num)
{
    romNbSchedule = num;
    EEPROM.put(EEP_NB_SCHEDULES, num);
    EEPROM.commit();
}

void romWriteSchedules(const schedule *_schedules)
{
    memcpy(romSchedules, _schedules, sizeof(romSchedules));
    EEPROM.put(EEP_SCHEDULES, romSchedules);
    EEPROM.commit();
}

void romWriteSSID(const char *_ssid)
{
    memcpy(romSSID, _ssid, sizeof(romSSID) - 1);
    EEPROM.put(EEP_SSID, romSSID);
    EEPROM.commit();
}

void romWritePassword(const char* _pwd)
{
    memcpy(romPassword, _pwd, sizeof(romPassword) - 1);
    EEPROM.put(EEP_PWD, romPassword);
    EEPROM.commit();
}

//Get functions
uint8_t romGetState()
{
    return romRelayState;
}

uint8_t romGetNbSchedule()
{
    return romNbSchedule;
}

schedule* romGetSchedules()
{
    return romSchedules;
}

char* romGetSSID()
{
    return romSSID;
}

char* romGetPassword()
{
    return romPassword;
}