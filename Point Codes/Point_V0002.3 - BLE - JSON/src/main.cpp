//********************************************************************************************
// Main
//********************************************************************************************

#include "Arduino.h"
#include <Define.h>
#include "Metering.h"
#include "LED.h"
#include "MJSON.h"
#include "M_EEPROM.h"
#include "Theft.h"
#include "MDelay.h"

//********************************************************************************************
// Task Handles
//********************************************************************************************

xTaskHandle READ_METER_VALUE_HANDLE;
xTaskHandle JSON_FRAME_READ_HANDLE;
xTaskHandle LED_HANDLE;

//********************************************************************************************
// Variable Defination
//********************************************************************************************

DELAY MDelay;

uint8_t RELAY_PIN = 19;
uint8_t Charger_State = 0;
long Time_counter = 0;

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
    vTaskDelay(JSON_Packet_Time_Interval_Sec * 1000 / portTICK_PERIOD_MS);
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
  while (1)
  {
    digitalWrite(RELAY_PIN, LOW);
    Charger_State = 0;
    Serial.println("Checking for Charger Conditions!");
    while (Theft_Condion())
      ;
    Charger_State = 1;
    Serial.println("Charger Conditions Are Perfect!");
    Serial.println("Waiting for mobile device to connect!");
    while (!device_connect_status())
      ;
    Charger_State = 2;
    Serial.println("Mobile device has been paired!");
    vTaskResume(JSON_FRAME_READ_HANDLE);
    MDelay.setDelay(BLE_CONN_WAIT_DELAY);
    while (!Start_message_identification())
    {
      while (MDelay.isTimeOut())
      {
        Serial.println("Mobile Paired but no Start Received, So rebooting!");
        Serial.println();
        ESP.restart();
      }
    }
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
    vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
    Record_EEPROM();
    vTaskDelete(JSON_FRAME_READ_HANDLE);
    vTaskDelete(LED_HANDLE);
    ESP.restart();
  }
}

//********************************************************************************************
// All Initilisation
//********************************************************************************************

void All_Init(void)
{
  disableCore1WDT();
  disableCore0WDT();
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  EEPROM_Init();
  LED_Strip_Init();
  Check_Initial_EEPROM_Status();
  Meter_Init();
  AES_Init();
  ble_init();
  Theft_Init(Theft_Detection_Pin);
}

//********************************************************************************************
// Setup
//********************************************************************************************

void setup()
{
  All_Init();
  Serial.println();
  Serial.println("Welcome to Magenta!");
  Serial.printf("Connect to Bluetooth: %s \n", Read_EEPROM_String(EEPROM_CHARGER_ID_ADD).c_str());
  xTaskCreatePinnedToCore(&Main_Thread, "Main_Thread", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(&LED_Thread, "LED_Thread", 1024, NULL, 2, &LED_HANDLE, 0);
  xTaskCreatePinnedToCore(&JSON_Frame_Thread, "JSON_Frame_Thread", 4096, NULL, 2, &JSON_FRAME_READ_HANDLE, 1);
  xTaskCreatePinnedToCore(&Read_Meter_Values_Thread, "READ_Meter_Values", 4096, NULL, 2, &READ_METER_VALUE_HANDLE, 1);
  vTaskSuspend(READ_METER_VALUE_HANDLE);
  vTaskSuspend(JSON_FRAME_READ_HANDLE);
}

//********************************************************************************************
// Loop Thread
//********************************************************************************************

void loop()
{
  if (!digitalRead(EEPROM_Reset_Pin))
  {
    Write_EEPROM_Byte(EEPROM_CHARGER_DATA_FLAG_ADD, 0);
    Write_EEPROM_String(EEPROM_CHARGER_ID_ADD, "P00000");
    Serial.println("EEPROM Errased!");
    Serial.printf("Charger ID: %s\n", Read_EEPROM_String(EEPROM_CHARGER_ID_ADD).c_str());
    Serial.printf("Total KWh Usage: %0.6f\n", Read_EEPROM_Float(EEPROM_Total_KWh_ADD));
    Serial.println("Rebooting!");
    ESP.restart();
  }
}

//********************************************************************************************
// End
//********************************************************************************************
