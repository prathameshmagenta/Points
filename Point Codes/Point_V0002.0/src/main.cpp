//********************************************************************************************
// Main
//********************************************************************************************

#include "Arduino.h"
#include <Define.h>
#include "Metering.h"
#include "LED.h"
#include "MJSON.h"

//********************************************************************************************
// Task Handles
//********************************************************************************************

xTaskHandle READ_METER_VALUE_HANDLE;
xTaskHandle JSON_FRAME_READ_HANDLE;
xTaskHandle LED_HANDLE;

//********************************************************************************************
// Variable Defination
//********************************************************************************************

uint8_t RELAY_PIN = 19;
uint8_t Charger_State = 0;

//********************************************************************************************
// JSON Thread
//********************************************************************************************

void JSON_Frame_Thread(void *pvParam)
{
  while (1)
  {
    RX_Json_Packet_Field_Identifier();
    if (Start_message_identification())
      Send_JSON_Frame();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

//********************************************************************************************
// Meter Read Thread
//********************************************************************************************

void Read_Meter_Values_Thread(void *pvParam)
{
  while (1)
  {
    Read_Meter_Values();
  }
}

//********************************************************************************************
// Meter Read Thread
//********************************************************************************************

void LED_Thread(void *pvParam)
{
  while (1)
  {
    State_of_charger_to_LED(Charger_State);
  }
}

//********************************************************************************************
// Main Thread
//********************************************************************************************

void Main_Thread(void *pvParam)
{
  digitalWrite(RELAY_PIN, LOW);
  while (1)
  {
    Charger_State = 1;
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Waiting for mobile device to connect!");
    while (!device_connect_status())
      ;
    Charger_State = 2;
    Serial.println("Mobile device has been paired!");
    vTaskResume(JSON_FRAME_READ_HANDLE);
    while (!Start_message_identification())
      ;
    Charger_State = 3;
    Serial.println("Charging experience started!");
    digitalWrite(RELAY_PIN, HIGH);
    vTaskResume(READ_METER_VALUE_HANDLE);
    while (Start_message_identification() && !Over_Load_Status() && !Battery_Full_Status() && !Session_Timeout_Status())
      ;
    Charger_State = 4;
    Serial.println("Ending charging experience!");
    digitalWrite(RELAY_PIN, LOW);
    vTaskDelete(READ_METER_VALUE_HANDLE);
    vTaskDelay(5*1000 / portTICK_PERIOD_MS);
    vTaskDelete(JSON_FRAME_READ_HANDLE);
    vTaskDelete(LED_HANDLE);
    vTaskDelay(5*1000 / portTICK_PERIOD_MS);
    ESP.restart();
  }
}

//********************************************************************************************
// Setup
//********************************************************************************************

void setup()
{
  disableCore1WDT();
  disableCore0WDT();
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  Meter_Init();
  ble_init();
  LED_Strip_Init();
  Serial.println();
  Serial.println("Welcome to Magenta!");
  Serial.println("Connect to Bluetooth: CG0001");
  xTaskCreatePinnedToCore(&Main_Thread, "Main_Thread", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(&LED_Thread, "LED_Thread", 1024, NULL, 2, &LED_HANDLE, 0);
  xTaskCreatePinnedToCore(&JSON_Frame_Thread, "JSON_Frame_Thread", 4096, NULL, 2, &JSON_FRAME_READ_HANDLE, 1);
  xTaskCreatePinnedToCore(&Read_Meter_Values_Thread, "READ_Meter_Values", 4096, NULL, 1, &READ_METER_VALUE_HANDLE, 1);
  vTaskSuspend(READ_METER_VALUE_HANDLE);
  vTaskSuspend(JSON_FRAME_READ_HANDLE);
}

//********************************************************************************************
// Loop Thread
//********************************************************************************************

void loop()
{
}

//********************************************************************************************
// End
//********************************************************************************************
