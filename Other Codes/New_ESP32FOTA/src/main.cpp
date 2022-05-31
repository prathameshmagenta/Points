#include <Arduino.h>
#include "Arduino.h"
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <esp32fota.h>

const char *ssid = "Point";
const char *password = "12345678";
bool Old_Connection = false;

esp32FOTA FOTA("esp32-fota-http", 1, false, true);

void init_WiFi(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setTxPower(WIFI_POWER_19dBm);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.setHostname("Magenta_CG_Point_Demo");
  if (SPIFFS.begin(true))
    Serial.println("ESP32 SPIFF Begin Successfully!");
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
      Serial.println("FIRMWARE UPGRADING!");
      FOTA.execOTA();
      Serial.println("FIRMWARE UPGRADED!");
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  init_WiFi();
}

void loop() {
  // put your main code here, to run repeatedly:
  Check_WiFi();
}