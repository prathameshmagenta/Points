#include "Arduino.h"
#include <Define.h>
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <esp32fota.h>
#include "MDelay.h"
#include "M_EEPROM.h"

const char *ssid = "Point";
const char *password = "12345678";
bool Old_Connection = false;

String Charger_Version = "0.0.1";

esp32FOTA FOTA("esp32-fota-http", Charger_Version, false);

void init_WiFi(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setTxPower(WIFI_POWER_19dBm);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.setHostname(Get_Charger_Name().c_str());
  SPIFFS.begin(true);
  FOTA.checkURL = "http://52.188.122.113/fota/CG_POINT/Stage/Point.json";
}

void Check_WiFi(void)
{
  while (WiFi.isConnected() && !Old_Connection)
  {
    Serial.println();
    Serial.print("Connected to: ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Old_Connection = true;
    bool updatedNeeded = FOTA.execHTTPcheck();
    if (updatedNeeded)
    {
      Serial.println("FOTA NEEDED! PLEASE WAIT!");
      FOTA.execOTA();
    }
    else
    {
      Serial.println("FOTA NOT NEEDED!");
    }
  }
  while (!WiFi.isConnected() && Old_Connection)
  {
    Serial.println("Disconnected from WiFi!");
    Old_Connection = false;
    WiFi.reconnect();
  }
}
