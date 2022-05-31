#ifndef _DEFINE_H
#define _DEFINE_H

#include "Arduino.h"

//********************************************************************************************
// Charger Credential
//********************************************************************************************

#define Charger_Version "0.0.1"

//********************************************************************************************
// WiFi Credential
//********************************************************************************************

// #define ssid "POINT"
// #define password "12345678"

//********************************************************************************************
// JSON Credential
//********************************************************************************************

#define JSON_Packet_Time_Interval_Sec 1
#define Fragment_Frame_Len 256

//********************************************************************************************
// EEPROM Credential
//********************************************************************************************

#define EEPROM_SIZE 512
#define NO_OF_USER_RECORD 5
#define EEPROM_CHARGER_DATA_FLAG_ADD 0
#define EEPROM_USER_TABLE_POINTER_ADD 1
#define EEPROM_Total_KWh_ADD 5
#define EEPROM_ENERGY_RATE_ADD 10
#define EEPROM_CHARGER_ID_ADD 20
#define EEPROM_CHIP_ID_ADD 35
#define EEPROM_CHARGER_TYPE_ADD 40
#define EEPROM_USER_ID_TABLE_ADD_VALUE 50
#define EEPROM_USER_FIELD_ADD_OFFSET 50
#define OFFSET 12
#define MAX_TABLE_SIZE (EEPROM_USER_ID_TABLE_ADD_VALUE + (NO_OF_USER_RECORD * EEPROM_USER_FIELD_ADD_OFFSET))
#define EEPROM_Reset_Pin 14

//********************************************************************************************
// Metering Credential
//********************************************************************************************

#define TEMP_SENSOR_PIN 33
#define Metering_Time_Interval_mSec 100
#define Average_Sample_Time 15
#define Number_Of_Samples_Per_Sec (1000 / Metering_Time_Interval_mSec)
#define Total_Number_Of_Samples (Average_Sample_Time * Number_Of_Samples_Per_Sec)
#define Time_Multiplier 60
#define U_Voltage 280.0
#define L_Voltage 180.0
#define U_Current 10.0
#define L_Current 0.1
#define U_Temp 50
#define L_Temp 10

//********************************************************************************************
// Theft Credential
//********************************************************************************************

#define Theft_Detection_Pin 32

//********************************************************************************************
// End
//********************************************************************************************

#endif