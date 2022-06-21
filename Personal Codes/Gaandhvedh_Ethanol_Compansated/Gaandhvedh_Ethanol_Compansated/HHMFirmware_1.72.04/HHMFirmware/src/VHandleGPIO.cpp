
#include "VHandleGPIO.h"
#include "VDefines.h"



#define LONG_PRESS      5000    //@ 5 Sec(in MS)

Button::Button(uint32_t pin, uint32_t delay){
    gpio_pin = pin;
    currentState = HIGH;
    b_delay = delay;
}

byte Button::getPin(){
    return gpio_pin;
}

void Button::setCurrentState(bool newState){
    if((currentState == HIGH) &&
        (newState == LOW)){
        this->setDelay(b_delay); 
    }
    currentState = newState;
}
    
bool Button::getCurrentState(){
    return currentState;
}

Button ESP32_MODE_SW(ESP32_MODE_SW_PIN, LONG_PRESS);


void gpio_initGPIO(){
    pinMode(ESP32_MODE_SW.getPin(), INPUT_PULLUP);

    // Set the pin modes for Input/Output pin
    pinMode(COMM_LED, OUTPUT);
    digitalWrite(COMM_LED, 1);

    pinMode(RED_LIGHT_PIN, OUTPUT);
    digitalWrite(RED_LIGHT_PIN, 0);

    pinMode(GREEN_LIGHT_PIN, OUTPUT);
    digitalWrite(GREEN_LIGHT_PIN, 1);

    pinMode(BLUE_LIGHT_PIN, OUTPUT);
    digitalWrite(BLUE_LIGHT_PIN, 1);
}

uint32_t gpio_readGPIO(){

    ESP32_MODE_SW.setCurrentState(digitalRead(ESP32_MODE_SW.getPin()));
  
    if ((ESP32_MODE_SW.getCurrentState() == LOW) && 
                            ESP32_MODE_SW.isTimeOut()){
        return ESP32_MODE_SW_ENABLED;
    }
    return NO_SWITCH_ENABLED;
}
