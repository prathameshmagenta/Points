#include "Arduino.h"
#include <Define.h>
#include <EEPROM.h>
#include <M_EEPROM.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void commission_over_bluetooth(void);

void EEPROM_Init(void)
{
    pinMode(EEPROM_Reset_Pin, INPUT_PULLUP);
    EEPROM.begin(EEPROM_SIZE);
}

void Check_Initial_EEPROM_Status(void)
{
    commission_over_bluetooth();
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
    String t_str = EEPROM.readString(_Address);
    t_str.replace("\n", "");
    t_str.replace("\r", "");
    t_str.replace("\0", "");
    return t_str;
}

uint8_t Read_EEPROM_Byte(uint32_t _Address)
{
    return EEPROM.readByte(_Address);
}

void commission_over_bluetooth(void)
{
    uint8_t Data_Flag = Read_EEPROM_Byte(EEPROM_CHARGER_DATA_FLAG_ADD);
    if (Data_Flag == 0)
    {
        SerialBT.begin(String("CGPOINT_" + Get_Chip_ID()).c_str());
        while (!SerialBT.connected())
            ;
        SerialBT.println("Welcome to Magenta!");
        SerialBT.println("Charger is not configured!");
        SerialBT.printf("Please Provide Charger ID: ");
        while (!SerialBT.available())
            ;
        String temp = SerialBT.readString();
        Write_EEPROM_String(EEPROM_CHARGER_ID_ADD, temp);
        delay(1000);
        SerialBT.printf("Please Provide Charger ID: ");
        while (!SerialBT.available())
            ;
        float tm = SerialBT.parseFloat();
        Write_EEPROM_Float(EEPROM_CHIP_ID_ADD, tm);
        delay(1000);
        SerialBT.printf("\nPlease Provide Charger Energy Rate (Rs/KWh):");
        while (!SerialBT.available())
            ;
        float temp1 = SerialBT.parseFloat();
        delay(1000);
        Write_EEPROM_Float(EEPROM_ENERGY_RATE_ADD, temp1);
        delay(1000);
        Write_EEPROM_Float(EEPROM_Total_KWh_ADD, 0.0);
        Write_EEPROM_Byte(EEPROM_USER_TABLE_POINTER_ADD, EEPROM_USER_ID_TABLE_ADD_VALUE);
        for (uint16_t i = EEPROM_USER_ID_TABLE_ADD_VALUE; i < MAX_TABLE_SIZE; i += EEPROM_USER_FIELD_ADD_OFFSET)
        {
            Write_EEPROM_String(i, "          ");
            Write_EEPROM_Float(i + OFFSET, 0.0);
            Write_EEPROM_Float(i + OFFSET + OFFSET, 0.0);
            Write_EEPROM_String(i + OFFSET + OFFSET + OFFSET, "          ");
        }
        SerialBT.println();
        SerialBT.println("Configuration Status:");
        SerialBT.printf("CORE ID (PHY): %s \n", ("CGPOINT_" + Get_Chip_ID()).c_str());
        SerialBT.printf("CORE ID (MEM): %s \n", ("CGPOINT_" + String(uint32_t(Read_EEPROM_Float(EEPROM_CHIP_ID_ADD)))).c_str());
        SerialBT.printf("CPO ID: %s \n", ("CGPOINT_" + Read_EEPROM_String(EEPROM_CHARGER_ID_ADD)).c_str());
        SerialBT.printf("Rate: %0.2f Rs/KWh\n", Read_EEPROM_Float(EEPROM_ENERGY_RATE_ADD));
        SerialBT.printf("Total KWh Usage: %0.6f KWh\n", Read_EEPROM_Float(EEPROM_Total_KWh_ADD));
        SerialBT.printf("Table Pointer: %d\n", Read_EEPROM_Byte(EEPROM_USER_TABLE_POINTER_ADD));
        for (uint16_t i = EEPROM_USER_ID_TABLE_ADD_VALUE; i < MAX_TABLE_SIZE; i += EEPROM_USER_FIELD_ADD_OFFSET)
            SerialBT.printf("User %d: %s\t KWh: %0.6f\t Session Sec: %0.2f\t Transaction ID: %s\n",
                            (i / EEPROM_USER_FIELD_ADD_OFFSET) - 1,
                            Read_EEPROM_String(i).c_str(),
                            Read_EEPROM_Float(i + OFFSET),
                            Read_EEPROM_Float(i + OFFSET + OFFSET),
                            Read_EEPROM_String(i + OFFSET + OFFSET + OFFSET).c_str());
        SerialBT.println();
        Write_EEPROM_Byte(EEPROM_CHARGER_DATA_FLAG_ADD, 255);
        delay(1000);
        SerialBT.disconnect();
        SerialBT.end();
        ESP.restart();
    }
}

String Get_Chip_ID(void)
{
    uint32_t chipId = 0;
    for (uint8_t i = 0; i < 17; i = i + 8)
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    return String(chipId);
}
