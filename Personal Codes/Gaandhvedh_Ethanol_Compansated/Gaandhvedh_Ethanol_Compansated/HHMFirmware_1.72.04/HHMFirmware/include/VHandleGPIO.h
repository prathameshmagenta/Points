#ifndef __V_HANDLEGPIO_H__
#define __V_HANDLEGPIO_H__

#include <Arduino.h>

#include "VDelay.h"

enum button{
    NO_SWITCH_ENABLED = 0,

    ESP32_MODE_SW_ENABLED,
};

#define ESP32_MODE_SW_PIN   32

class Button: public DELAY
{
protected:
    byte gpio_pin;
    bool currentState;
    uint32_t b_delay;

public:
    Button(uint32_t pin, uint32_t delay);
    byte getPin();

    void setCurrentState(bool newState);
    bool getCurrentState();
};

void gpio_initGPIO();
uint32_t gpio_readGPIO();


#endif