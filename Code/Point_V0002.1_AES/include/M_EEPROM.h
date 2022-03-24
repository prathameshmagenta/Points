#ifndef _M_EEPROM_H
#define _M_EEPROM_H

#include "Arduino.h"

void EEPROM_Init(void);
void Write_EEPROM_Float(uint32_t _Address, float _Float_Data);
void Write_EEPROM_String(uint32_t _Address, String _String_Data);
float Read_EEPROM_Float(uint32_t _Address);
String Read_EEPROM_String(uint32_t _Address);

#endif