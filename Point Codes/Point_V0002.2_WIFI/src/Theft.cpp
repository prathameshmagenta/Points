#include "Arduino.h"
#include "Theft.h"

uint8_t Sensor_Pin = 0;
uint8_t Emergancy_Pin = 33;

void Theft_Init(uint8_t _Hall_Sensor_Pin)
{
    Sensor_Pin = _Hall_Sensor_Pin;
    pinMode(Sensor_Pin, INPUT);
}

bool Theft_Condion(void)
{
    float Hall_Voltage = (analogRead(Sensor_Pin) / 4095) * 3.3;
    if (Hall_Voltage < 1.5f)
        return true;
    else
        return false;
}