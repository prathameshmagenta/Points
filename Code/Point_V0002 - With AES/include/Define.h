#ifndef _DEFINE_H
#define _DEFINE_H

#include "Arduino.h"

#define Charger_Name "P0001"

#define JSON_Packet_Time_Interval_Sec 1

#define EEPROM_SIZE 10
#define EEPROM_START_ADD 0
#define EEPROM_Reset_Pin 12

#define Metering_Time_Interval_mSec 100
#define Average_Sample_Time 15
#define Number_Of_Samples_Per_Sec (1000 / Metering_Time_Interval_mSec)
#define Total_Number_Of_Samples (Average_Sample_Time * Number_Of_Samples_Per_Sec)
#define Time_Multiplier 60
#define U_Voltage 280.0
#define L_Voltage 180.0
#define U_Current 13.0
#define L_Current 0.1
#define U_Temp 50
#define L_Temp 10

#define Theft_Detection_Pin 35

#endif