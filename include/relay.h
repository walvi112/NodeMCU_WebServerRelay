#ifndef RELAY_H
#define RELAY_H
#include <Arduino.h>
#include <TickTwo.h>
#include <stdint.h>

#define RELAY_PIN           D1
#define BUTTON_PIN          D2

#define RELAY_TYPE        NPN

typedef enum RelayState
{
    OFF,
    ON
} RelayState;

typedef enum RelayType
{
    NPN,
    PNP,
} RelayType;

typedef enum ButtonState
{
    PRESSED = 0,
    RELEASED =1,
} ButtonState;

typedef struct Relay
{
    RelayType _type;
    RelayState _state;
} Relay;


void relayInit(RelayType type);
void relaySetState(RelayState state);
void relaySwitch(void);

void IRAM_ATTR relayButtonPressIRQ(void);
void relayButtonHandler(void);
RelayState getRelayState(void);
void blink();

#endif