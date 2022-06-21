#ifndef _M_EEPROM_H
#define _M_EEPROM_H

#include "Arduino.h"

void EEPROM_Init(void);
void Check_Initial_EEPROM_Status(void);
void Write_EEPROM_Float(uint32_t _Address, float _Float_Data);
void Write_EEPROM_String(uint32_t _Address, String _String_Data);
void Write_EEPROM_Byte(uint32_t _Address, uint8_t _Byte_Data);
void Record_EEPROM(void);
float Read_EEPROM_Float(uint32_t _Address);
String Read_EEPROM_String(uint32_t _Address);
uint8_t Read_EEPROM_Byte(uint32_t _Address);
String Get_Chip_ID(void);

#endif