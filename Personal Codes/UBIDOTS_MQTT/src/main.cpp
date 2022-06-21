#include <Arduino.h>
/******************************************
 *
 * This example works for both Industrial and STEM users.
 *
 * Developed by Jose Garcia, https://github.com/jotathebest/
 *
 * ****************************************/

/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsEsp32Mqtt.h"

/****************************************
 * Define Constants
 ****************************************/
const char *UBIDOTS_TOKEN = "BBFF-BBJAlMHX115eT2X2mk3InJD1WSA0C2"; // Put here your Ubidots TOKEN
const char *WIFI_SSID = "Point";                                   // Put here your Wi-Fi SSID
const char *WIFI_PASS = "12345678";                                // Put here your Wi-Fi password
const char *DEVICE_LABEL = "point";                                // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL = "Analog_Read";                        // Put here your Variable label to which data  will be published

const int PUBLISH_FREQUENCY = 1000; // Update rate in milliseconds

unsigned long timer;
uint8_t analogPin = 34; // Pin used to read data from GPIO34 ADC_CH6.
float value = 0;

Ubidots ubidots(UBIDOTS_TOKEN);

/****************************************
 * Auxiliar Functions
 ****************************************/

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void MQTT(void *param)
{
  while (1)
  {
  }
}

/****************************************
 * Main Functions
 ****************************************/

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  // ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  timer = millis();
  xTaskCreate(&MQTT, "MQTT", 2048, NULL, 2, NULL);
}

void loop()
{
  // put your main code here, to run repeatedly:
    if (!ubidots.connected())
    {
      ubidots.reconnect();
    }
    if (abs(millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
    {
      value += 0.1;
      if (value > 3.3f)
        value = 0;
      ubidots.add(VARIABLE_LABEL, value); // Insert your variable Labels and the value to be sent
      ubidots.publish(DEVICE_LABEL);
      timer = millis();
    }
    ubidots.loop();
}