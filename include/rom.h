#ifndef ROM_H
#define ROM_H
#include "webserver.h"

#define EEP_SSID            0   //SSID 33 byte with null terminated
#define EEP_PWD             33  //Password 17 byte with null terminated
#define EEP_STATE           50
#define EEP_NB_SCHEDULES    51
#define EEP_SCHEDULES       52

#define EEP_SIZE            628

// Initialization
void romInit();

// Read functions
uint8_t romReadState();
uint8_t romReadNbSchedule();
schedule* romReadSchedules();
char* romReadSSID();
char* romReadPassword();

// Write functions
void romWriteState(uint8_t state);
void romWriteNbSchedule(uint8_t num);
void romWriteSchedules(const schedule* _schedules);
void romWriteSSID(const char* _ssid);
void romWritePassword(const char* _pwd);

// Get functions
uint8_t romGetState();
uint8_t romGetNbSchedule();
schedule* romGetSchedules();
char* romGetSSID();
char* romGetPassword();



#endif