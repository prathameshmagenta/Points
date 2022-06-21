#include <Arduino.h>
#include "EmonLib.h"
#include "EEPROM.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#define emonTxV3

const char *ssid = "Point";
const char *password = "12345678";

AsyncWebServer server(80);
EnergyMonitor EMON;

uint64_t MAC;
uint64_t chipId;
uint64_t CID = 31055450755601;

float Old_Irms = 0.0;
float Old_Vrms = 0.0;
float Old_P = 0.0;

// void init_WiFi(void)
// {
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(ssid, password);
//   Serial.print("Connecting to network");
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println();
//   Serial.print("Connected to ");
//   Serial.println(ssid);
//   Serial.print("IP address: ");
//   Serial.println(WiFi.localIP());
//   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
//             { request->send(200, "text/plain", "ESP-32 OTA Page!"); });

//   AsyncElegantOTA.begin(&server); // Start AsyncElegantOTA
//   server.begin();
//   Serial.println("HTTP server started!");
// }

void setup()
{
  // put your setup code here, to run once:
  disableCore1WDT();
  disableCore0WDT();
  Serial.begin(115200);
  ledcSetup(0, 1000.0, 8);
  ledcAttachPin(13, 0);
  pinMode(A0, INPUT);
  pinMode(A3, INPUT);
  EMON.current(A0, 19.75 / 3);
  EMON.voltage(A3, 4550.0 * 2, 0);
  Serial.println("ESP-32 Details:");
  MAC = ESP.getEfuseMac();
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
  ledcWrite(0, 128);
  float PWM_Freq = ledcReadFreq(0);
  Serial.printf("PWM Channel Frequency: %0.2f\n\n", PWM_Freq);
  // init_WiFi();
}

void loop()
{
  // put your main code here, to run repeatedly:
  EMON.calcVI(30000, 500);
  float Irms = EMON.Irms;
  float Vrms = EMON.Vrms;
  Irms = (Irms < 0.15) ? (0.0) : (Irms);
  float P = Vrms * Irms;
  // if (P != Old_P)
  // {
  Serial.printf("Voltage: %0.2f V\tCurrent: %0.4f A\t Power: %0.2f W\n", Vrms, Irms, P);
  //   Old_P = P;
  // }
  // delay(1000);
}