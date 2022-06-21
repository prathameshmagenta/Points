#ifndef _THEFT_H
#define _THEFT_H

#include "Arduino.h"

void Theft_Init(uint8_t _Hall_Sensor_Pin);
bool Theft_Condion(void);

#endif