#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
#include "ble_utility.h"
#include "Metering.h"
#include "LED.h"

void Send_Over_Strip(uint8_t _Red, uint8_t _Green, uint8_t _Blue);
void colorWipe(uint32_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
void theaterChase(uint32_t c, uint8_t wait);
void theaterChaseRainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);
void Show_Red(void);
void Show_Blue(void);
void Show_Magenta(void);
void Show_Blink(uint8_t _Red, uint8_t _Green, uint8_t _Blue);

uint8_t state = 0;
uint8_t LED_PIN = 18;
uint8_t LEDs = 5;

Adafruit_NeoPixel Strip(LEDs, LED_PIN, NEO_GRB + NEO_KHZ800);

void LED_Strip_Init()
{
    Strip.begin();
    Strip.setBrightness(255);
    Strip.clear();
    // rainbowCycle(10);
}

void State_of_charger_to_LED(uint8_t _State)
{
    uint8_t State = _State;
    switch (State)
    {
    case 0:
        Show_Red();
        break;
    case 1:
        Show_Blink(0, 0, 255);
        break;
    case 2:
        Show_Blue();
        break;
    case 3:
        Show_Blink(0, 255, 0);
        break;
    case 4:
        Show_Red();
        break;
    default:
        Show_Blink(255, 0, 0);
        break;
    }
}

void Show_Red(void)
{
    Send_Over_Strip(255, 0, 0);
}

void Show_Blue(void)
{
    Send_Over_Strip(0, 0, 255);
}

void Show_Blink(uint8_t _Red, uint8_t _Green, uint8_t _Blue)
{
    Send_Over_Strip(_Red, _Green, _Blue);
    delay(500);
    Send_Over_Strip(0, 0, 0);
    delay(500);
}

void Send_Over_Strip(uint8_t _Red, uint8_t _Green, uint8_t _Blue)
{
    for (size_t i = 0; i <= LEDs; i++)
    {
        Strip.setPixelColor(i, _Red, _Green, _Blue);
    }
    Strip.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
    for (uint16_t i = 0; i < Strip.numPixels(); i++)
    {
        Strip.setPixelColor(i, c);
        Strip.show();
        delay(wait);
    }
}

void rainbow(uint8_t wait)
{
    uint16_t i, j;

    for (j = 0; j < 256; j++)
    {
        for (i = 0; i < Strip.numPixels(); i++)
        {
            Strip.setPixelColor(i, Wheel((i + j) & 255));
        }
        Strip.show();
        delay(wait);
    }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait)
{
    uint16_t i, j;
    for (j = 0; j < 256 * 5; j++)
    { // 5 cycles of all colors on wheel
        for (i = 0; i < Strip.numPixels(); i++)
        {
            Strip.setPixelColor(i, Wheel(((i * 256 / Strip.numPixels()) + j) & 255));
        }
        Strip.show();
        delay(wait);
    }
}

// Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait)
{
    for (int j = 0; j < 10; j++)
    { // do 10 cycles of chasing
        for (int q = 0; q < 3; q++)
        {
            for (uint16_t i = 0; i < Strip.numPixels(); i = i + 3)
            {
                Strip.setPixelColor(i + q, c); // turn every third pixel on
            }
            Strip.show();
            delay(wait);
            for (uint16_t i = 0; i < Strip.numPixels(); i = i + 3)
            {
                Strip.setPixelColor(i + q, 0); // turn every third pixel off
            }
        }
    }
}

// Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait)
{
    for (int j = 0; j < 256; j++)
    { // cycle all 256 colors in the wheel
        for (int q = 0; q < 3; q++)
        {
            for (uint16_t i = 0; i < Strip.numPixels(); i = i + 3)
            {
                Strip.setPixelColor(i + q, Wheel((i + j) % 255)); // turn every third pixel on
            }
            Strip.show();
            delay(wait);
            for (uint16_t i = 0; i < Strip.numPixels(); i = i + 3)
            {
                Strip.setPixelColor(i + q, 0); // turn every third pixel off
            }
        }
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85)
    {
        return Strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170)
    {
        WheelPos -= 85;
        return Strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return Strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
