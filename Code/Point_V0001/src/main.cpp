#include "Arduino.h"
#include "HLW8032.h"

HLW8032 HL;

#define SYNC_TIME 1000

float Voltage_Val;
float Current_Val;
float Prev_Current_Val = -1;
float Watt_Val;
static unsigned long start_time, end_time;

void setup()
{
  HL.begin(Serial2, 4);
  Serial.begin(9600);
  Serial.println();
  Serial.println("Welcome to Magenta!");
  Serial.println("HLW-8032 Ready!");
  start_time = millis();
}

void loop()
{
  end_time = millis();
  HL.SerialReadLoop();
  if (end_time - start_time > SYNC_TIME)
  {
    if (HL.SerialRead == 1)
    {
      Voltage_Val = (HL.GetVol() / 1000.0);
      Current_Val = (HL.GetCurrent());
      Watt_Val = (HL.GetInspectingPower() * 1E-6);
    }
    start_time = end_time;
    if (Current_Val != Prev_Current_Val)
    {
      Serial.printf("V: %0.2f V\tI: %0.4f Amp.\tW: %0.4f KW\n", Voltage_Val, Current_Val, Watt_Val);
      Prev_Current_Val = Current_Val;
    }
  }
}
