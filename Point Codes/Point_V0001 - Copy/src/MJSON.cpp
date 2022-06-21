#include "Arduino.h"
#include "ArduinoJson.h"
#include "MJSON.h"
#include "ble_utility.h"
#include "Metering.h"
#include "LED.h"

#define CHARGER_ID "PT0001"

String APP_ID;
char json[] = "{\"app_id\":\"PT0001\",\"charger_id\":\"PT0001\",\"ble_status\":\"OFF\",\"data\":[0,0]}";

DynamicJsonDocument doc(1024);
DynamicJsonDocument doc1(1024);

void JSON_Init(void)
{
}

void deser_JSON(char _json[])
{
    deserializeJson(doc, _json);
}

void ser_JSON(void)
{
    doc1["app_id"] = "A0001";
    doc1["charger_id"] = "P0001";
    doc1["ble_status"] = "ON";
    doc1["data"][0] = Voltage;
    doc1["data"][1] = Current;
    serializeJson(doc1, Serial);
}
