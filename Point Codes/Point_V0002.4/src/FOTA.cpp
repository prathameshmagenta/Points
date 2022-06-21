#include "Arduino.h"
#include <Define.h>
#include "FOTA.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "MDelay.h"
#include "M_EEPROM.h"

const char *ssid = "Point";
const char *password = "12345678";
bool Old_Connection = false;

AsyncWebServer server(80);

void init_WiFi(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setTxPower(WIFI_POWER_19dBm);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.setHostname(String("CGPOINT_" + Read_EEPROM_String(EEPROM_CHARGER_ID_ADD)).c_str());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "ESP-32 OTA Page! Please put extension of '/update' in URL!"); });

  AsyncElegantOTA.begin(&server); // Start AsyncElegantOTA
  server.begin();
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
  }
  while (!WiFi.isConnected() && Old_Connection)
  {
    Serial.println("Disconnected to WiFi!");
    Old_Connection = false;
    WiFi.reconnect();
  }
}
