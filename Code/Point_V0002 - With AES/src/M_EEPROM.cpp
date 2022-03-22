#include "Arduino.h"
#include <Define.h>
#include <EEPROM.h>
#include <M_EEPROM.h>

void EEPROM_Init(void)
{
    pinMode(EEPROM_Reset_Pin, INPUT_PULLUP);
    EEPROM.begin(EEPROM_SIZE);
}

void Write_EEPROM_Float(uint32_t _Address, float _Float_Data)
{
    if (!digitalRead(EEPROM_Reset_Pin))
    {
        EEPROM.put(_Address, 0);
        EEPROM.commit();
    }
    else
    {
        _Float_Data += EEPROM.readFloat(_Address);
        EEPROM.put(_Address, _Float_Data);
        EEPROM.commit();
    }
}

void Write_EEPROM_String(uint32_t _Address, String _String_Data)
{
    if (!digitalRead(EEPROM_Reset_Pin))
    {
        EEPROM.writeString(_Address, "P0000");
        EEPROM.commit();
    }
    else
    {
        EEPROM.writeString(_Address, _String_Data);
        EEPROM.commit();
    }
}

float Read_EEPROM_Float(uint32_t _Address)
{
    return EEPROM.readFloat(_Address);
}

String Read_EEPROM_String(uint32_t _Address)
{
    return String(EEPROM.readString(_Address));
}