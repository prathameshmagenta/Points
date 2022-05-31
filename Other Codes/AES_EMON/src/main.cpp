#include <Arduino.h>
#include "EmonLib.h"

EnergyMonitor emon;

uint32_t chipId = 0;

void ESP_Details(void)
{
  Serial.println();
  Serial.println("ESP-32 Details:");
  for (uint8_t i = 0; i < 17; i = i + 8)
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("This chip has %d cores\n", ESP.getChipCores());
  Serial.print("MAC ID (Numeric): ");
  Serial.println(ESP.getEfuseMac());
  Serial.print("MAC ID (Hex): ");
  Serial.println(ESP.getEfuseMac(), HEX);
  Serial.print("Chip ID: ");
  Serial.println(chipId);
}

void setup()
{
  // put your setup code here, to run once:
  disableCore1WDT();
  disableCore0WDT();
  Serial.begin(115200);
  ESP_Details();
  emon.voltage(A0, 234.26, 1.7);
  emon.current(A3, 111.1);
}

void loop()
{
  // put your main code here, to run repeatedly:
}