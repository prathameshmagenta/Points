#include "Arduino.h"
#include "HLW8032.h"
#include <ArduinoJson.h>
#include <BLE.h>

#define CHARGER_ID "CG0001"
#define SYNC_TIME 1000
#define U_Voltage 250.0
#define L_Voltage 180.0
#define U_Current 13.0
#define L_Current 001.0

xTaskHandle READ_METER_VALUE_HANDLE;

HLW8032 HL;
BLE myBLE;

float Current = 0;
float Voltage = 280.0;
float Power = 0;
static unsigned long start_time, end_time;
bool Start_Flag, Emergancy_Stop_flag, Charge_On, Full_Charge = false;

void Over_VI_Monitoring_Thread()
{
  if (Voltage >= U_Voltage || Voltage <= L_Voltage)
  {
    Emergancy_Stop_flag = true;
    Serial.println("System Stopped due to over voltage!");
  }
  else
  {
    Emergancy_Stop_flag = false;
  }
  if (Current >= U_Current || Current <= L_Current)
  {
    Emergancy_Stop_flag = true;
    Serial.println("System Stopped due to over voltage!");
  }
  else
  {
    Emergancy_Stop_flag = false;
  }
}

void Read_Meter_Values_Thread(void *pvParam)
{
  while (1)
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
        Serial.printf("V: %0.2fV\tI: %0.4fA\tP: %0.4fW\t BLE: %s\n", Voltage, Current, Power, myBLE.RxData());
      }
      start_time = end_time;
    }
    //Over_VI_Monitoring_Thread();
  }
}

void BLE_Approval_Thread(void *pvParam)
{
  vTaskSuspend(READ_METER_VALUE_HANDLE);
  Start_Flag = false;
  while (1)
  {
    vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
    while (!(myBLE.BLE_Connect_Status() && !Start_Flag && !Emergancy_Stop_flag))
      ;
    Serial.println("BLE Connected & Meter Reading Starting Soon!");
    Start_Flag = true;
    String ble_data = myBLE.RxData();
    Serial.println(ble_data);
    vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
    while (!(myBLE.BLE_Connect_Status() && Start_Flag && !Emergancy_Stop_flag))
      ;
    vTaskResume(READ_METER_VALUE_HANDLE);
    Serial.println("BLE Connected & Meter Reading Enabled!");
    Start_Flag = true;
    vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
    while (!(!myBLE.BLE_Connect_Status() && Start_Flag && !Emergancy_Stop_flag))
      ;
    vTaskSuspend(READ_METER_VALUE_HANDLE);
    Serial.println("BLE Disonnected Due to Meter Reading Disabled!");
    Start_Flag = 0;
  }
}

void setup()
{
  Serial.begin(115200);
  HL.begin(Serial2, 4);
  myBLE.BLE_INIT();
  Serial.println();
  Serial.println("Welcome to Magenta!");
  Serial.println("Connect to Bluetooth: CG0001");
  Serial.println("HLW-8032 Ready!");
  start_time = millis();
  xTaskCreate(&Read_Meter_Values_Thread, "READ_Meter_Values", 2048, NULL, 1, &READ_METER_VALUE_HANDLE);
  xTaskCreate(&BLE_Approval_Thread, "BLE_Approval_Thread", 2048, NULL, 2, NULL);
  disableCore0WDT();
}

void loop()
{
}
