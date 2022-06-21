#include "Arduino.h"
#include <Define.h>
#include <EEPROM.h>
#include <M_EEPROM.h>

void EEPROM_Init(void)
{
    pinMode(EEPROM_Reset_Pin, INPUT_PULLUP);
    if (!EEPROM.begin(EEPROM_SIZE))
        Serial.println("EEPROM Failed!");
    else
        Serial.println("EEPROM Success!");
}

void Check_Initial_EEPROM_Status(void)
{
    uint8_t Data_Flag = Read_EEPROM_Byte(EEPROM_CHARGER_DATA_FLAG_ADD);
    if (Data_Flag == 0)
    {
        Serial.println();
        Serial.println("Charger is not configured!");
        Serial.printf("Please Provide Charger ID:");
        while (!Serial.available())
            ;
        String temp = Serial.readString();
        Write_EEPROM_String(EEPROM_CHARGER_ID_ADD, temp);
        Serial.printf("\nDo you want to reset total consumption by charger? (Y/N): ");
        while (!Serial.available())
            ;
        String temp_str = Serial.readString();
        if (temp_str == "Y" || temp_str == "y")
            Write_EEPROM_Float(EEPROM_Total_KWh_ADD, 0.0);
        Write_EEPROM_Byte(EEPROM_CHARGER_DATA_FLAG_ADD, 255);
        Serial.println();
        Serial.println("Configuration Status:");
        Serial.printf("Charger ID: %s\n", Read_EEPROM_String(EEPROM_CHARGER_ID_ADD).c_str());
        Serial.printf("User ID: %s\n", Read_EEPROM_String(EEPROM_USER_ID_ADD).c_str());
        Serial.printf("KWh Usage: %0.6f\n", Read_EEPROM_Float(EEPROM_KWh_ADD));
        Serial.printf("Total KWh Usage: %0.6f\n", Read_EEPROM_Float(EEPROM_Total_KWh_ADD));
        Serial.println();
    }
}

void Write_EEPROM_Float(uint32_t _Address, float _Float_Data)
{
    if (_Float_Data != Read_EEPROM_Float(_Address))
    {
        EEPROM.writeFloat(_Address, _Float_Data);
        EEPROM.commit();
    }
}

void Write_EEPROM_String(uint32_t _Address, String _String_Data)
{
    EEPROM.writeString(_Address, _String_Data);
    EEPROM.commit();
}

void Write_EEPROM_Byte(uint32_t _Address, uint8_t _Byte_Data)
{
    if (_Byte_Data != Read_EEPROM_Float(_Address))
    {
        EEPROM.writeByte(_Address, _Byte_Data);
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

uint8_t Read_EEPROM_Byte(uint32_t _Address)
{
    return EEPROM.readByte(_Address);
}
