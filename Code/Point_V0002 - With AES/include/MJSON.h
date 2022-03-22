#ifndef _MJSON_H
#define _MJSON_H

#include "Arduino.h"
#include "ArduinoJson.h"
#include "ble_utility.h"
#include "Metering.h"
#include "LED.h"

bool Start_message_identification(void);
void RX_Json_Packet_Field_Identifier(void);
void Send_JSON_Frame(void);
void ser_JSON(void);
void AES_Init(void);
void Record_EEPROM(void);

#endif