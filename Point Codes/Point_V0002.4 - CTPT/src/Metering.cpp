//********************************************************************************************
// Meter
//********************************************************************************************

#include "Arduino.h"
#include <Define.h>
#include "EmonLib.h"
// #include "HLW8032.h"
#include "Metering.h"
#include "MJSON.h"

//********************************************************************************************
// HLW-8012 Object Creation
//********************************************************************************************
// #define emonTxV3

// HLW8032 HL;
EnergyMonitor EMON;

//********************************************************************************************
// Variables
//********************************************************************************************

float Current = 0.0;
float Voltage = 230.0;
float Power = 0.0;
float Temp = 30.0;
float Current_Average = 0;
float KWh = 0.0;
float KWh_Value = 0.0;
unsigned long start_time;
unsigned long end_time;
unsigned long Session_Counter;
bool Overload_Flag = false;
bool Battery_Full_Flag = false;
bool Session_Time_Out = false;
bool No_Voltage_Flag = true;

uint32_t i = 1;
uint32_t smp = 0;

//********************************************************************************************
// Initilise Meter
//********************************************************************************************

void Meter_Init()
{
    pinMode(CT_Pin, INPUT);
    pinMode(PT_Pin, INPUT);
    EMON.current(CT_Pin, 20.0);
    EMON.voltage(PT_Pin, 4890.0, 1.5);
    // HL.begin(Serial2, 4);
    // start_time = millis();
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
// No Voltage Indicator
//********************************************************************************************

bool No_Voltage_Status(void)
{
    return No_Voltage_Flag;
}

//********************************************************************************************
// Session KWh Monitoring
//********************************************************************************************

void Session_KWh_Monitorig(void)
{
    if (KWh / KWh_Value >= 1.0f)
    {
        Session_Time_Out = true;
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
    if (i % Total_Number_Of_Samples == 0)
    {
        Current_Average = Current_Average / Total_Number_Of_Samples;
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
        Current_Average += Current;
    }
}

//********************************************************************************************
// Overload Monitoring
//********************************************************************************************

void Over_VI_Monitoring()
{
    if (/*Voltage > U_Voltage ||*/ Voltage < L_Voltage || Current > U_Current /*|| Temp > U_Temp*/)
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
// Initial Voltage Watch
//********************************************************************************************

void No_Voltage_Monitoring(void)
{
    delay(3000);
    float Monitoring_Voltage = 0.0;
    for (uint8_t j = 0; j < 7; j++)
    {
        Monitoring_Voltage += Voltage;
        delay(1000);
    }
    Monitoring_Voltage = Monitoring_Voltage / 7;
    Serial.printf("Starting Voltage: %0.2f\n", Monitoring_Voltage);
    if (Monitoring_Voltage <= L_Voltage)
        No_Voltage_Flag = true;
    else
        No_Voltage_Flag = false;
}

//********************************************************************************************
// Temperature Watch
//********************************************************************************************

void Temp_Measure(void)
{
    Temp = (analogRead(TEMP_SENSOR_PIN) * 3.3 / 4095.0);
    Temp = ((3.3 * (10.0 / Temp)) - 10) * 1000.0;
    Temp = log(Temp);
    Temp = (1 / (0.001129148 + (0.000234125 * Temp) +
                 (0.0000000876741 * Temp * Temp * Temp)));
    Temp = Temp - 273.15;
}

//********************************************************************************************
// Meter Values Read at Sampling Rate
//********************************************************************************************

void Read_Meter_Values(void)
{
    // end_time = millis();
    // HL.SerialReadLoop();
    // if (end_time - start_time >= Metering_Time_Interval_mSec)
    // {
    // if (HL.SerialRead == 1)
    // {
    //     Voltage = (HL.GetVol() / 1000.0);
    //     Current = (HL.GetCurrent());
    //     Power = (HL.GetInspectingPower() * 1e-6);
    //     KWh = KWh + (Power / (Time_Multiplier * Time_Multiplier * Number_Of_Samples_Per_Sec));
    //     Temp_Measure();
    // }
    EMON.calcVI(20000, Metering_Time_Interval_mSec);
    Current = EMON.Irms;
    Voltage = EMON.Vrms;
    Current = (Current < 0.15) ? (0.0) : (Current);
    Power = (Voltage * Current) / 1000.0;
    Temp_Measure();
    Serial.printf("V: %0.2fV\tI: %0.4fA\tP: %0.4fKW\tE: %0.8fKWh\tT: %0.2f??C\n\r",
                  Voltage, Current, Power, KWh, Temp);
    if (Start_message_identification() && !Battery_Full_Flag && !Overload_Flag && !Session_Time_Out)
    {
        KWh = KWh + (Power / (Time_Multiplier * Time_Multiplier * Number_Of_Samples_Per_Sec));
        Running_Current_Average();
        Session_KWh_Monitorig();
        Over_VI_Monitoring();
    }
    // start_time = end_time;
    // }
}
