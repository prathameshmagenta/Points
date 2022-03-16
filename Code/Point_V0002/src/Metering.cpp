//********************************************************************************************
// Meter
//********************************************************************************************

#include "Arduino.h"
#include <Define.h>
#include "HLW8032.h"
#include "Metering.h"

//********************************************************************************************
// HLW-8012 Object Creation
//********************************************************************************************

HLW8032 HL;

//********************************************************************************************
// Defining Parameters
//********************************************************************************************

#define U_Voltage 280.0
#define L_Voltage 180.0
#define U_Current 13.0
#define L_Current 0.1
#define U_Temp 55
#define L_Temp 10
#define AVERAGE_SAMPLES 15

//********************************************************************************************
// Variables
//********************************************************************************************

void Convert_Values_In_String(void);

float Current = 0.0;
float Voltage = 230.0;
float Power = 0.0;
float Temp = 30.0;
float Voltage_Average = 0;
float Current_Average = 0;
float KWh = 0.0;
float KWh_Value = 0.0;
unsigned long SYNC_TIME = 1000;
unsigned long start_time;
unsigned long end_time;
unsigned long Session_Counter;
bool Overload_Flag = false;
bool Battery_Full_Flag = false;
bool Session_Time_Out = false;
uint8_t i = 1;

//********************************************************************************************
// Initilise Meter
//********************************************************************************************

void Meter_Init()
{
    HL.begin(Serial2, 4);
    start_time = millis();
    pinMode(2, INPUT);
}

//********************************************************************************************
// Overload Indicator
//********************************************************************************************

bool Over_Load_Status(void)
{
    return Overload_Flag;
}

//********************************************************************************************
// Battery Indicator
//********************************************************************************************

bool Battery_Full_Status(void)
{
    return Battery_Full_Flag;
}

//********************************************************************************************
// Session Time Out Indicator
//********************************************************************************************

bool Session_Timeout_Status(void)
{
    return Session_Time_Out;
}

//********************************************************************************************
// Session KWh Monitoring
//********************************************************************************************

void Session_KWh_Monitorig(void)
{
    if (KWh / KWh_Value >= 1.0f)
    {
        Session_Time_Out = true;
        delay(1000);
        KWh = 0.0;
    }
    else
    {
        Session_Time_Out = false;
    }
}

//********************************************************************************************
// Setting KWh
//********************************************************************************************

void Set_KWh_Value(float _Kwh_Value)
{
    KWh_Value = _Kwh_Value;
}

//********************************************************************************************
// Ongoing Current Average at 15 Samples
//********************************************************************************************

void Running_Current_Average(void)
{
    if (i % AVERAGE_SAMPLES == 0)
    {
        Voltage_Average = Voltage_Average / AVERAGE_SAMPLES;
        Current_Average = Current_Average / AVERAGE_SAMPLES;
        if (Current_Average < L_Current)
        {
            Battery_Full_Flag = true;
            Serial.println();
            Serial.println("Battery Fully Charged!");
        }
        else
            Battery_Full_Flag = false;
        // Serial.printf("Voltage Average: %0.4fV Current Average: %0.4fA Battery Full: %d\n",
        //    Voltage_Average, Current_Average, Battery_Full_Flag);
        i = 1;
    }
    else
    {
        i++;
        Voltage_Average += Voltage;
        Current_Average += Current;
    }
}

//********************************************************************************************
// Overload Monitoring
//********************************************************************************************

void Over_VI_Monitoring()
{
    if (Voltage > U_Voltage || Voltage < L_Voltage || Current > U_Current || Temp > U_Temp)
    {
        Overload_Flag = true;
        Serial.println("System Stopped due to over load condition!");
    }
    else
    {
        Overload_Flag = false;
    }
}

//********************************************************************************************
// Temperature Watch
//********************************************************************************************

void Temp_Measure(void)
{
    Temp = (analogRead(2) * 3.3 / 4095.0);
    Temp = ((3.3 * (10.0 / Temp)) - 10) * 1000.0;
    Temp = log(Temp);
    Temp = (1 / (0.001129148 + (0.000234125 * Temp) +
                 (0.0000000876741 * Temp * Temp * Temp)));
    Temp = Temp - 273.15;
}

//********************************************************************************************
// Meter Values Read at 1 sec.
//********************************************************************************************

void Read_Meter_Values(void)
{
    end_time = millis();
    HL.SerialReadLoop();
    if (end_time - start_time > SYNC_TIME)
    {
        if (HL.SerialRead == 1)
        {
            Voltage = (HL.GetVol() / 1000.0);
            Current = (HL.GetCurrent());
            Power = (HL.GetInspectingPower() * 1e-6);
            KWh = KWh + (Power / 3600.0);
            Temp_Measure();
        }
        Serial.printf("V: %0.2fV\tI: %0.4fA\tP: %0.4fKW\tE: %0.8fKWh\tT: %0.2f°C\n", Voltage, Current, Power, KWh, Temp);
        //Convert_Values_In_String();
        Running_Current_Average();
        Session_KWh_Monitorig();
        Over_VI_Monitoring();
        start_time = end_time;
    }
}
