//********************************************************************************************
// Magenta JSON 
//********************************************************************************************

#include "Arduino.h"
#include <Define.h>
#include "ArduinoJson.h"
#include "ble_utility.h"
#include "Metering.h"

//********************************************************************************************
// JSON Object Creation
//********************************************************************************************

DynamicJsonDocument doc(384);
DynamicJsonDocument doc1(384);

//********************************************************************************************
// Internal Calling Functions Declaration
//********************************************************************************************

void Set_flags_As_Per_Identifier(void);
void Read_JSON_Frame(void);
void deser_JSON(char _json[]);
void ser_JSON(void);

//********************************************************************************************
// Local Variables
//********************************************************************************************

const char *APP_ID;
uint8_t Start_Command = 0;
bool Set_KWh_Value_Flag = false;
bool Start_Charge = false;
float KWh_from_App = 0.0;

String Rx_JSON_Packet;
String Tx_JSON_Packet;

//********************************************************************************************
// Receiving JSON Packet
//********************************************************************************************

void RX_Json_Packet_Field_Identifier(void)
{
    Read_JSON_Frame();
    APP_ID = doc["app_id"];
    KWh_from_App = doc["KWh"];
    Start_Command = doc["start"];
    Set_flags_As_Per_Identifier();
}

//********************************************************************************************
// Setting Flags
//********************************************************************************************

void Set_flags_As_Per_Identifier(void)
{
    if (Start_Command == 1)
    {
        Set_KWh_Value(KWh_from_App);
        Start_Charge = true;
    }
    else
        Start_Charge = false;
}

//********************************************************************************************
// Deserialised JSON Packet
//********************************************************************************************

void Read_JSON_Frame(void)
{
    Rx_JSON_Packet = Scan_JSON_On_BLE();
    if (!Rx_JSON_Packet.isEmpty())
    {
        deser_JSON((char *)Rx_JSON_Packet.c_str());
        Rx_JSON_Packet.clear();
    }
}

void deser_JSON(char _json[])
{
    deserializeJson(doc, _json);
}

//********************************************************************************************
// Serialised JSON Packet
//********************************************************************************************

void ser_JSON(void)
{
    doc1["app_id"] = "A0001";
    doc1["charger_id"] = Charger_Name;
    if (!device_connect_status())
        doc1["ble_status"] = "OFF";
    else
        doc1["ble_status"] = "ON";
    if (Start_Charge && !Over_Load_Status() && !Battery_Full_Status() && !Session_Timeout_Status())
        doc1["start"] = 1;
    else
        doc1["start"] = 0;
    doc1["KWh"] = KWh_from_App;
    doc1["meter_data"][0] = Voltage;
    doc1["meter_data"][1] = Current;
    doc1["meter_data"][2] = Power;
    doc1["meter_data"][3] = KWh;
    doc1["meter_data"][4] = Temp;
    if (!Over_Load_Status())
        doc1["overload"] = "OFF";
    else
        doc1["overload"] = "ON";
    serializeJson(doc1, Tx_JSON_Packet);
}

//********************************************************************************************
// Send JSON Packet
//********************************************************************************************

void Send_JSON_Frame(void)
{
    ser_JSON();
    send_data_app((uint8_t *)Tx_JSON_Packet.c_str(), (uint16_t)Tx_JSON_Packet.length());
    Tx_JSON_Packet.clear();
}

//********************************************************************************************
// Start Flag
//********************************************************************************************

bool Start_message_identification(void)
{
    return Start_Charge;
}
