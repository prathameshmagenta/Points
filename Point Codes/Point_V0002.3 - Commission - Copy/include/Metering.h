#ifndef METERING_H
#define MEETRING_H

//********************************************************************************************
// HLW Header File
//********************************************************************************************
#include "Arduino.h"

//********************************************************************************************
// Extern Global Varibale
//********************************************************************************************
extern float_t Voltage;
extern float_t Current;
extern float_t Power;
extern float_t KWh;
extern float_t KWh_Val;
extern float_t Temp;
//extern float_t Current_Average;

//********************************************************************************************
// Extern Function
//********************************************************************************************

void Meter_Init(void);
void Read_Meter_Values(void);
void Set_KWh_Value(float _Kwh_Value);
bool Over_Load_Status(void);
bool Battery_Full_Status(void);
bool Session_Timeout_Status(void);
bool Voltage_Monitoring_Status(void);
bool Voltage_Monitoring_Status(void);

//********************************************************************************************
// End of File
//********************************************************************************************
#endif
