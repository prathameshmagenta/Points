//********************************************************************************************
// LED
//********************************************************************************************

#include "Arduino.h"
#include <Define.h>
#include "Adafruit_NeoPixel.h"

//********************************************************************************************
// LED Inter Calls
//********************************************************************************************

void Send_Over_Strip(uint8_t _Red, uint8_t _Green, uint8_t _Blue);
void Show_LED(uint8_t _Red, uint8_t _Green, uint8_t _Blue);
void Show_Blink(uint8_t _Red, uint8_t _Green, uint8_t _Blue);
void Show_Flow(uint8_t _Red, uint8_t _Green, uint8_t _Blue);

//********************************************************************************************
// Pins Declaration
//********************************************************************************************

uint8_t LED_PIN = 15;
uint8_t LEDs = 5;
uint8_t Charger_State;

//********************************************************************************************
// Object Creation
//********************************************************************************************

Adafruit_NeoPixel Strip(LEDs, LED_PIN, NEO_GRB + NEO_KHZ800);

void LED_Strip_Init()
{
    Strip.begin();
    Strip.clear();
    Strip.setBrightness(255);
    // Show_LED(0, 0, 0);
}

//********************************************************************************************
// Sending State of Charger to LED
//********************************************************************************************

void State_of_charger_to_LED(uint8_t _State)
{
    switch (_State)
    {
    case 0:
        Show_Blink(255, 0, 0); // Error State
        break;
    case 1:
        Show_Blink(255, 255, 255); // Idle State
        break;
    case 2:
        Show_LED(255, 50, 0); // Connected/Prepare State
        break;
    case 3:
        Show_Blink(0, 255, 0); // Charging State
        break;
    case 4:
        Show_Blink(0, 0, 255); // Completed State
        break;
    case 5:
        Show_Blink(0, 255, 255); // Commission State
        break;
    default:
        Show_Blink(0, 0, 0); // Null State
        break;
    }
}

//********************************************************************************************
// Show Constant Colour
//********************************************************************************************

void Show_LED(uint8_t _Red, uint8_t _Green, uint8_t _Blue)
{
    Strip.setBrightness(255);
    Send_Over_Strip(_Red, _Green, _Blue);
}

//********************************************************************************************
// Show Blinking Colour
//********************************************************************************************

void Show_Blink(uint8_t _Red, uint8_t _Green, uint8_t _Blue)
{
    Strip.setBrightness(255);
    Send_Over_Strip(_Red, _Green, _Blue);
    delay(750);
    Send_Over_Strip(0, 0, 0);
    delay(250);
    // for (uint8_t i = 0; i <= 250; i = i + 1)
    // {
    //     Send_Over_Strip(_Red, _Green, _Blue);
    //     Strip.setBrightness(i);
    //     delay(4);
    // }
}

//********************************************************************************************
// Show Flow Colour
//********************************************************************************************

void Show_Flow(uint8_t _Red, uint8_t _Green, uint8_t _Blue)
{
    for (uint8_t i = 0; i < LEDs + (LEDs / 2); i++)
    {
        Strip.setPixelColor(i, _Red, _Green, _Blue);
        Strip.setPixelColor(i - (LEDs / 2), 0);
        delay(1000 / LEDs);
        Strip.show();
    }
}

//********************************************************************************************
// Show Colour
//********************************************************************************************

void Send_Over_Strip(uint8_t _Red, uint8_t _Green, uint8_t _Blue)
{
    for (size_t i = 0; i <= LEDs; i++)
    {
        Strip.setPixelColor(i, _Red, _Green, _Blue);
    }
    Strip.show();
}

//********************************************************************************************
// End
//********************************************************************************************
