//********************************************************************************************
// Magenta JSON
//********************************************************************************************

#include "Arduino.h"
#include <Define.h>
#include "ArduinoJson.h"
#include "ble_utility.h"
#include "Metering.h"
#include "AESLib.h"

//********************************************************************************************
// JSON Object Creation
//********************************************************************************************

AESLib aesLib;
DynamicJsonDocument doc(384);
DynamicJsonDocument doc1(384);

//********************************************************************************************
// Internal Calling Functions Declaration
//********************************************************************************************

void Set_flags_As_Per_Identifier(void);
void Read_JSON_Frame(void);
void deser_JSON(char _json[]);
void ser_JSON(void);
String encrypt(char *msg, uint16_t msgLen, byte iv[]);

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

char cleartext[256];
char ciphertext[512];
// AES Encryption Key
byte aes_key[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
// General initialization vector (you must use your own IV's in production for full security!!!)
byte aes_iv[N_BLOCK] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
    Serial.println(encrypt(strdup(Tx_JSON_Packet.c_str()), Tx_JSON_Packet.length(), aes_iv));
    Tx_JSON_Packet.clear();
}

//********************************************************************************************
// Start Flag
//********************************************************************************************

bool Start_message_identification(void)
{
    return Start_Charge;
}

//********************************************************************************************
// AES Encryption
//********************************************************************************************

String encrypt(char *msg, uint16_t msgLen, byte iv[])
{
    int cipherlength = aesLib.get_cipher64_length(msgLen);
    char encrypted[cipherlength]; // AHA! needs to be large, 2x is not enough
    aesLib.encrypt64(msg, msgLen, encrypted, aes_key, sizeof(aes_key), iv);
    Serial.print("encrypted = ");
    Serial.println(encrypted);
    return String(encrypted);
}

void AES_Init(void)
{
    aesLib.set_paddingmode(paddingMode::CMS);
    //
    // verify with https://cryptii.com
    // previously: verify with https://gchq.github.io/CyberChef/#recipe=To_Base64('A-Za-z0-9%2B/%3D')
    //
    char b64in[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char b64out[base64_enc_len(sizeof(aes_iv))];
    base64_encode(b64out, b64in, 16);
    char b64enc[base64_enc_len(10)];
    base64_encode(b64enc, (char *)"0123456789", 10);
    char b64dec[base64_dec_len(b64enc, sizeof(b64enc))];
    base64_decode(b64dec, b64enc, sizeof(b64enc));
}
