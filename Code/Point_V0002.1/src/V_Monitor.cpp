//********************************************************************************************
// Meter
//********************************************************************************************

#include "Arduino.h"
#include <Define.h>
#include "HLW8032.h"
#include "V_Monitor.h"

//********************************************************************************************
// HLW-8012 Object Creation
//********************************************************************************************

HLW8032 HLW2;

float Monitoring_Voltage = 0;

//********************************************************************************************
// Voltage Values for line up indication
//********************************************************************************************

bool Voltage_Monitoring_Status(void)
{
    for (uint8_t i = 0; i < 5; i++)
    {
        if (HLW2.SerialRead == 1)
            Monitoring_Voltage += (HLW2.GetVol() / 1000.0);
        delay(1000);
    }
    Monitoring_Voltage = Monitoring_Voltage / 5;
    if (Monitoring_Voltage <= 0.0f)
        return true;
    else
        return false;
}