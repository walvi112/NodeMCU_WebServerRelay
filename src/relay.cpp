#include "relay.h"
#include "serial.h"

static TickTwo timerLED(blink, 500, 4, MILLIS);

static Relay relay;

static ButtonState currentBtnState = RELEASED;
static bool btnStateChanged = false;
static bool longPressed = false;

static unsigned long lastDebounceTime = 0;
static unsigned long debounceDelay = 50;
static unsigned long longPressDelay = 2000;

static bool ledState = 0; //on state of builtin led

void relayInit(RelayType type)
{
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    ledState = 0;
    digitalWrite(LED_BUILTIN, ledState); //turn on builin led
    relay._type = type;
    relaySetState(OFF);
}

void relaySetState(RelayState state)
{
    Serial.println("Relay changes state.");
    if (relay._state != state)
    {
        relay._state = state;
        digitalWrite(RELAY_PIN, relay._state ^ relay._type);
    }
    
}

void relaySwitch(void)
{
    Serial.println("Relay changes state.");
    relay._state = (RelayState) ((uint8_t) relay._state ^ 1);
    digitalWrite(RELAY_PIN, relay._state ^ relay._type);
}

void relayButtonHandler(void)
{
    currentBtnState = (ButtonState) digitalRead(BUTTON_PIN);
    timerLED.update();
    if (btnStateChanged)
    {
        switch (currentBtnState)
        {
            case RELEASED:
                if (longPressed)
                {
                    longPressed = false;
                }
                else
                {
                    relaySwitch();
                    logger()->println("Normal press");
                }
                btnStateChanged = false;
                break;
            case PRESSED:
                if ((millis() - lastDebounceTime > longPressDelay))
                {
                    logger()->println("Long pressed");
                    timerLED.start();
                    longPressed = true;
                    btnStateChanged = false;        
                }
        } 
    }
    
        
}

void IRAM_ATTR relayButtonPressIRQ(void)
{
    if ((millis() - lastDebounceTime) > debounceDelay)
        {
            btnStateChanged = true;
        }
    lastDebounceTime = millis();
}   


RelayState getRelayState(void)
{
    return relay._state;
}

void blink()
{
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState);
}