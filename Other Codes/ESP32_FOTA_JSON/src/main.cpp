#include <Arduino.h>
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <esp32fota.h>

// Change to your WiFi credentials
const char *ssid = "Point";
const char *password = "12345678";
String MAC;

// esp32fota esp32fota("<Type of Firme for this device>", <this version>, <validate signature>);
esp32FOTA FOTA("esp32-fota-http", 1, false);

void PWM_Setup(uint8_t _Ch, uint8_t _Pin, float _frq)
{
  ledcSetup(_Ch, _frq, 8);
  ledcAttachPin(_Pin, 0);
  ledcWrite(_Ch, 128);
  Serial.printf("PWM Channel: %d\nPWM Pin: %d\nPWM Frequency: %0.2f Hz\n",
                _Ch, _Pin, _frq);
}

void setup_wifi()
{
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  WiFi.setHostname(String("CG_POINT_" + MAC).c_str());
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  // Provide spiffs with root_ca.pem to validate server certificate
  SPIFFS.begin(true);
  FOTA.checkURL = "https://server/fota/fota.json";
  Serial.begin(115200);
  MAC = String((uint16_t)(ESP.getEfuseMac() >> 32), HEX) + String((uint32_t)ESP.getEfuseMac(), HEX);
  Serial.println();
  Serial.printf("%s Version %d with %d Core/s\n", ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
  Serial.printf("ESP32 MAC ID = %s\n", MAC.c_str());
  setup_wifi();
  PWM_Setup(0, 13, 1000.0);
}

void loop()
{

  // bool updatedNeeded = FOTA.execHTTPcheck();
  // if (updatedNeeded)
  //   FOTA.execOTA();
  while (!WiFi.isConnected())
    WiFi.reconnect();
  delay(1000);
}