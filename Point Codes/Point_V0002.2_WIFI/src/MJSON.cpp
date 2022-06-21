//********************************************************************************************
// Magenta JSON
//********************************************************************************************

#include "Arduino.h"
#include <Define.h>
#include "ArduinoJson.h"
#include <MJSON.h>
#include "ble_utility.h"
#include "Metering.h"
#include "M_EEPROM.h"
#include "AESLib.h"

//********************************************************************************************
// JSON Object Creation
//********************************************************************************************

AESLib aesLib;
StaticJsonDocument<512> doc;
StaticJsonDocument<512> doc1;

//********************************************************************************************
// Internal Calling Functions Declaration
//********************************************************************************************

void Set_flags_As_Per_Identifier(void);
void Read_JSON_Frame(void);
void deser_JSON(char _json[]);
void ser_JSON(void);
String encrypt(char *msg, uint16_t msgLen, byte iv[]);
String decrypt(char *msg, uint16_t msgLen, byte iv[]);

//********************************************************************************************
// Local Variables
//********************************************************************************************

String USER_ID;
uint8_t Start_Command = 0;
bool Set_KWh_Value_Flag = false;
bool Start_Charge = false;
float KWh_from_App = 0.0;
long Session_Count = 0;

String Rx_JSON_Packet;
String Tx_JSON_Packet;
String Crypto;

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
    USER_ID = (const char *)doc["user_id"];
    KWh_from_App = doc["KWh_app"];
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
    doc1["type"] = "Pointz";
    doc1["user_id"] = USER_ID;
    doc1["charger_id"] = Read_EEPROM_String(EEPROM_CHARGER_ID_ADD);
    if (!device_connect_status())
        doc1["ble_status"] = "OFF";
    else
        doc1["ble_status"] = "ON";
    if (Start_Charge && !Over_Load_Status() && !Battery_Full_Status() && !Session_Timeout_Status())
        doc1["start"] = 1;
    else
        doc1["start"] = 0;
    doc1["KWh_app"] = KWh_from_App;
    doc1["voltage"] = Voltage;
    doc1["current"] = Current;
    doc1["power"] = Power;
    doc1["KWh"] = KWh;
    doc1["temp"] = Temp;
    doc1["session_sec"] = Session_Count;
    doc1["prev_user_id"] = Read_EEPROM_String(EEPROM_USER_ID_ADD);
    doc1["prev_user_KWh"] = Read_EEPROM_Float(EEPROM_KWh_ADD);
    doc1["total_KWh"] = Read_EEPROM_Float(EEPROM_Total_KWh_ADD);
    if (!Over_Load_Status())
        doc1["overload"] = "OFF";
    else
        doc1["overload"] = "ON";
    serializeJson(doc1, Tx_JSON_Packet);
    Crypto = encrypt(strdup(Tx_JSON_Packet.c_str()), Tx_JSON_Packet.length(), aes_iv);
    // Serial.println(Crypto);
    Serial.println(decrypt(strdup(Crypto.c_str()),Crypto.length(), aes_iv));
}

//********************************************************************************************
// Send JSON Packet
//********************************************************************************************

void Send_JSON_Frame(void)
{
    if (Start_message_identification() && !Over_Load_Status() && !Battery_Full_Status() && !Session_Timeout_Status())
        Session_Count++;
    ser_JSON();
    send_data_app((uint8_t *)Crypto.c_str(), (uint16_t)Crypto.length());
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
    // Serial.print("encrypted = ");
    // Serial.println(encrypted);
    return String(encrypted);
}

//********************************************************************************************
// AES Decryption
//********************************************************************************************

String decrypt(char *msg, uint16_t msgLen, byte iv[])
{
    char decrypted[msgLen];
    aesLib.decrypt64(msg, msgLen, decrypted, aes_key, sizeof(aes_key), iv);
    return String(decrypted);
}

//********************************************************************************************
// AES Encryption Initilisation
//********************************************************************************************

void AES_Init(void)
{
    aesLib.gen_iv(aes_iv);
    aesLib.set_paddingmode(paddingMode::Null);
    //
    // verify with https://cryptii.com
    // previously: verify with https://gchq.github.io/CyberChef/#recipe=To_Base64('A-Za-z0-9%2B/%3D')
    //
    // char b64in[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // char b64out[base64_enc_len(sizeof(aes_iv))];
    // base64_encode(b64out, b64in, 16);
    // char b64enc[base64_enc_len(10)];
    // base64_encode(b64enc, (char *)"0123456789", 10);
    // char b64dec[base64_dec_len(b64enc, sizeof(b64enc))];
    // base64_decode(b64dec, b64enc, sizeof(b64enc));
}

//********************************************************************************************
// Record Session in EEPROM
//********************************************************************************************

void Record_EEPROM(void)
{
    Write_EEPROM_String(EEPROM_USER_ID_ADD, USER_ID);
    Write_EEPROM_Float(EEPROM_KWh_ADD, KWh);
    Write_EEPROM_Float(EEPROM_Total_KWh_ADD, KWh + Read_EEPROM_Float(EEPROM_Total_KWh_ADD));
    Serial.printf("Last Session User ID: %s\n", Read_EEPROM_String(EEPROM_USER_ID_ADD).c_str());
    Serial.printf("Last Session KWh Usage: %0.6f\n", Read_EEPROM_Float(EEPROM_KWh_ADD));
    Serial.printf("Total KWh Usage: %0.6f\n", Read_EEPROM_Float(EEPROM_Total_KWh_ADD));
}
