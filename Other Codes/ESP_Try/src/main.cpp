#include <Arduino.h>
#include <BLE.h>
#include "HLW8032.h"
#include <SoftwareSerial.h>

HLW8032 HLW;
BLE myBLE;

xTaskHandle Read_VI_Task;

bool Over_Current = 0;
bool Over_Voltage = 0;
bool Bluetooth_Connected = 0;

float Current_val = 0;
float Voltage_val = 0;

void read_VI(void *pvParam)
{
  while (1)
  {
    HLW.SerialReadLoop();
    if (HLW.SerialRead)
    {
      Current_val = HLW.GetCurrent();
      Voltage_val = HLW.GetVol();
    }
  }
}

void bluetooth_connect_task(void *pvParam)
{
  while (1)
  {
    while (!myBLE.BLE_Connect_Status())
      ;
    Serial.println("BLE Connected!!!");
    Serial.println(myBLE.RxData());
    while (myBLE.BLE_Connect_Status())
      ;
    Serial.println("BLE Disonnected!!!");
    ;
  }
}

void blink(void *pvParam)
{
  while (1)
  {
    digitalWrite(2, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(2, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(9600);
  HLW.begin(Serial1, 4);
  pinMode(2, OUTPUT);
  myBLE.BLE_INIT();
  Serial.println();
  Serial.println("Welconme to MAGENTA!");
  Serial.println("The device ready to pair for charging experience!");
  // Defineing RTOS Tasks
  // xTaskCreate(read_VI, "READ_VI", 1024, NULL, 1, &Read_VI_Task);
  xTaskCreate(bluetooth_connect_task, "BLUETOOTH_CONNECT_TASK", 2048, NULL, 2, NULL);
  xTaskCreate(blink, "BLINK", 512, NULL, 1, NULL);
  // Suspending VI Read task
  vTaskSuspend(Read_VI_Task);
}

void loop()
{
}