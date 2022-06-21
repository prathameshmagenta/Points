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
#include "FOTA.h"

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

uint8_t RELAY_PIN = 2;
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
    Charger_State = 0;
    Serial.println("Checking for Charger Conditions!");
    // while (Theft_Condion())
    //   ;
    Charger_State = 1;
    Serial.println("Charger Conditions Are Perfect!");
    Serial.println("Waiting for mobile device to connect!");
    while (!device_connect_status())
      ;
    Charger_State = 2;
    Serial.println("Mobile device has been paired!");
    set_ble_name(String(Get_Charger_Name() + "_O"));
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
    vTaskDelay(100 / portTICK_PERIOD_MS);
    digitalWrite(RELAY_PIN, HIGH);
    while (Start_message_identification() && !Over_Load_Status() && !Battery_Full_Status() && !Session_Timeout_Status())
      ;
    Charger_State = 4;
    Serial.println("Ending charging experience!");
    digitalWrite(RELAY_PIN, LOW);
    vTaskDelete(READ_METER_VALUE_HANDLE);
    Record_EEPROM();
    vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
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
  Serial.println();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  EEPROM_Init();
  LED_Strip_Init();
  Check_Initial_EEPROM_Status();
  Meter_Init();
  AES_Init();
  ble_init();
  Theft_Init(Theft_Detection_Pin);
  init_WiFi();
  Serial.printf("\nESP32 Chip model: %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  String cid = String(uint32_t(ESP.getEfuseMac() >> 32), HEX) + String(uint32_t(ESP.getEfuseMac()), HEX);
  cid.toUpperCase();
  Serial.printf("MAC ID: %s\n", cid.c_str());
  Serial.printf("Chip ID: %s\n", Get_Chip_ID().c_str());
  Serial.printf("CPO ID: %s\n", Get_Charger_Name().c_str());
  Serial.printf("Firmware Version: %s\n\n", Charger_Version);
  Serial.println("Welcome to Magenta!");
}

//********************************************************************************************
// Setup
//********************************************************************************************

void setup()
{
  All_Init();
  xTaskCreateUniversal(&Main_Thread, "Main_Thread", 4096, NULL, 1, NULL, 0);
  xTaskCreateUniversal(&LED_Thread, "LED_Thread", 4096, NULL, 1, &LED_HANDLE, 0);
  xTaskCreateUniversal(&JSON_Frame_Thread, "JSON_Frame_Thread", 4096, NULL, 1, &JSON_FRAME_READ_HANDLE, 1);
  xTaskCreateUniversal(&Read_Meter_Values_Thread, "READ_Meter_Values", 4096, NULL, 1, &READ_METER_VALUE_HANDLE, 1);
  vTaskSuspend(JSON_FRAME_READ_HANDLE);
}

//********************************************************************************************
// Loop Thread
//********************************************************************************************

void loop()
{
  Check_WiFi();
  if (!digitalRead(EEPROM_Reset_Pin))
  {
    Write_EEPROM_Byte(EEPROM_CHARGER_DATA_FLAG_ADD, 0);
    Write_EEPROM_Byte(EEPROM_CHARGER_TYPE_ADD, 0);
    Write_EEPROM_String(EEPROM_CHARGER_ID_ADD, "           ");
    Serial.println("EEPROM Errased!");
    // Serial.printf("CPO ID: %s\n", Read_EEPROM_String(EEPROM_CHARGER_ID_ADD).c_str());
    Serial.printf("Total KWh Usage: %0.6f\n", Read_EEPROM_Float(EEPROM_Total_KWh_ADD));
    Serial.println("Rebooting!");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP.restart();
  }
}

//********************************************************************************************
// End
//********************************************************************************************
