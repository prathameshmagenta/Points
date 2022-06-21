#include "Arduino.h"
#include "HLW8032.h"
#include <ArduinoJson.h>
#include "MJSON.h"
#include "ble_utility.h"
#include "Metering.h"
#include "LED.h"

xTaskHandle READ_METER_VALUE_HANDLE;

uint8_t RELAY_PIN = 19;
uint8_t Charger_State = 0;

void LED_Display_Thread(void *pvParam)
{
  Charger_State = 0;
  while (1)
  {
    State_of_charger_to_LED(Charger_State);
  }
}

void Read_Meter_Values_Thread(void *pvParam)
{
  while (1)
  {
    Read_Meter_Values();
  }
}

void BLE_Approval_Thread(void *pvParam)
{
  digitalWrite(RELAY_PIN, LOW);
  vTaskSuspend(READ_METER_VALUE_HANDLE);
  while (1)
  {
    Charger_State = 1;
    digitalWrite(RELAY_PIN, LOW);
    Serial.println(F("Waiting for bluetooth get connected!"));
    while (!device_connect_status())
      vTaskSuspend(READ_METER_VALUE_HANDLE);
    Charger_State = 2;
    Serial.println(F("BLE Connected!"));
    while (!KWh_Set_identification())
      vTaskSuspend(READ_METER_VALUE_HANDLE);
    while (!Start_message_identification())
      ;
    Serial.println(F("Starting Charging Experiance!"));
    Charger_State = 3;
    digitalWrite(RELAY_PIN, HIGH);
    vTaskResume(READ_METER_VALUE_HANDLE);
    while (!Stop_message_identification() && !Battery_Full_Status() && !Session_Timeout_Status() && !Over_Load_Status())
      ;
    Charger_State = 4;
    Serial.println(F("Stoping Charging Experiance!"));
    digitalWrite(RELAY_PIN, LOW);
    vTaskSuspend(READ_METER_VALUE_HANDLE);
    // while (device_connect_status())
    //   ;
    // Serial.println("BLE Disconnected!");
    Serial.println(F("Thank you for using Magenta Charging Services!"));
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP.restart();
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  Meter_Init();
  ble_init();
  LED_Strip_Init();
  Serial.println();
  Serial.println("Welcome to Magenta!");
  Serial.println("Connect to Bluetooth: CG0001");
  Serial.println("HLW-8032 Ready!");
  xTaskCreate(&Read_Meter_Values_Thread, "READ_Meter_Values", 4096, NULL, 1, &READ_METER_VALUE_HANDLE);
  xTaskCreate(&LED_Display_Thread, "LED_Display", 2046, NULL, 1, NULL);
  xTaskCreate(&BLE_Approval_Thread, "BLE_Approval_Thread", 4096, NULL, 2, NULL);
  vTaskSuspend(READ_METER_VALUE_HANDLE);
  disableCore0WDT();
  disableCore1WDT();
}

void loop()
{
}
