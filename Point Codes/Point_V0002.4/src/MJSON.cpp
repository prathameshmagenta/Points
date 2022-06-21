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
StaticJsonDocument<1024> doc;
StaticJsonDocument<2048> doc1;

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
String Transaction_ID;
String Rate_Update;
uint8_t Start_Command = 0;
float Table_pointer = 0;
bool Set_KWh_Value_Flag = false;
bool Start_Charge = false;
float KWh_from_App = 0.0;
float Transaction_Ammount = 0.0;
float Transaction_Rate = 0.0;
long Session_Count = 0;

String Rx_JSON_Packet;
String Tx_JSON_Packet;
String Crypto;

char cleartext[128];
char ciphertext[256];
// AES Encryption Key
// byte aes_key[] = {"ABCDEFGHIJKLMNOP"};
byte aes_key[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
// General initialization vector (you must use your own IV's in production for full security!!!)
byte aes_iv[N_BLOCK] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//********************************************************************************************
// Receiving JSON Packet
//********************************************************************************************

void RX_Json_Packet_Field_Identifier(void)
{
    Read_JSON_Frame();
    USER_ID = (const char *)doc["user"];
    Transaction_ID = (const char *)doc["tran"];
    Transaction_Ammount = doc["ammt"];
    Transaction_Rate = doc["rt"];
    Rate_Update = (const char *)doc["rtup"];
    Start_Command = doc["stsp"];

    if (Rate_Update == "Yes")
        Write_EEPROM_Float(EEPROM_ENERGY_RATE_ADD, Transaction_Rate);

    if (Transaction_Rate == 0)
        KWh_from_App = 10;
    else
        KWh_from_App = (Transaction_Ammount / Transaction_Rate);
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
    char str[20];

    doc1["comp"] = "Magenta";
    doc1["type"] = "Point";
    doc1["chrg"] = String("CGPOINT_" + Read_EEPROM_String(EEPROM_CHARGER_ID_ADD)).c_str();
    doc1["user"] = USER_ID;
    doc1["tran"] = Transaction_ID;
    doc1["ammt"] = Transaction_Ammount;
    doc1["rt"] = Transaction_Rate;
    doc1["rtup"] = Rate_Update;
    if (!Start_message_identification())
        doc1["stat"] = uint8_t(1);
    else if (Start_message_identification())
        doc1["stat"] = uint8_t(2);
    if (Battery_Full_Status() || Session_Timeout_Status())
        doc1["stat"] = uint8_t(3);
    if (Start_message_identification() && !Over_Load_Status() && !Battery_Full_Status() && !Session_Timeout_Status())
        doc1["stsp"] = uint8_t(1);
    else
        doc1["stsp"] = uint8_t(0);
    sprintf(str, "%0.1f", Voltage);
    doc1["mtr"][0] = str;
    sprintf(str, "%0.3f", Current);
    doc1["mtr"][1] = str;
    sprintf(str, "%0.3f", KWh);
    doc1["mtr"][2] = str;
    if (Start_message_identification() && !Over_Load_Status() && !Battery_Full_Status() && !Session_Timeout_Status())
        Session_Count++;
    doc1["sess"] = Session_Count;
    if (!Over_Load_Status())
        doc1["err"] = uint8_t(0);
    else if (Over_Load_Status() && Voltage > U_Voltage)
        doc1["err"] = uint8_t(10);
    else if (Over_Load_Status() && Voltage > L_Voltage)
        doc1["err"] = uint8_t(11);
    else if (Over_Load_Status() && Current > U_Current)
        doc1["err"] = uint8_t(12);
    else if (Over_Load_Status() && Voltage == 0.0f)
        doc1["err"] = uint8_t(15);
    else if (Over_Load_Status() && Temp > U_Temp)
        doc1["err"] = uint8_t(18);

    doc1["tKWh"] = Read_EEPROM_Float(EEPROM_Total_KWh_ADD);
    for (uint16_t i = EEPROM_USER_ID_TABLE_ADD_VALUE; i < MAX_TABLE_SIZE; i += EEPROM_USER_FIELD_ADD_OFFSET)
    {
        doc1["rec"][(4 * (i - EEPROM_USER_ID_TABLE_ADD_VALUE) / EEPROM_USER_FIELD_ADD_OFFSET)] =
            Read_EEPROM_String(i);
        doc1["rec"][(4 * (i - EEPROM_USER_ID_TABLE_ADD_VALUE) / EEPROM_USER_FIELD_ADD_OFFSET) + 1] =
            Read_EEPROM_String(i + OFFSET + OFFSET + OFFSET);
        doc1["rec"][(4 * (i - EEPROM_USER_ID_TABLE_ADD_VALUE) / EEPROM_USER_FIELD_ADD_OFFSET) + 2] =
            Read_EEPROM_Float(i + OFFSET);
        doc1["rec"][(4 * (i - EEPROM_USER_ID_TABLE_ADD_VALUE) / EEPROM_USER_FIELD_ADD_OFFSET) + 3] =
            Read_EEPROM_Float(i + OFFSET + OFFSET);
    }
    serializeJson(doc1, Tx_JSON_Packet);
    // Crypto = encrypt(strdup(Tx_JSON_Packet.c_str()), Tx_JSON_Packet.length(), aes_iv);
    // Serial.printf("\nEncrypted Data: %s\nLength: %d\n", Crypto.c_str(), Crypto.length());
    // Serial.println(decrypt(strdup(Crypto.c_str()), Crypto.length(), aes_iv));
}

//********************************************************************************************
// Send JSON Packet
//********************************************************************************************

void Send_JSON_Frame(void)
{
    ser_JSON();
    send_data_app((uint8_t *)Tx_JSON_Packet.c_str(), (uint16_t)Tx_JSON_Packet.length());
    Serial.println("Tx JSON Packet: " + Tx_JSON_Packet);
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
    aesLib.set_paddingmode(paddingMode::Space);
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

//********************************************************************************************
// Record Session in EEPROM
//********************************************************************************************

void Record_EEPROM(void)
{
    Table_pointer = Read_EEPROM_Float(EEPROM_USER_TABLE_POINTER_ADD);
    if (Table_pointer < EEPROM_USER_ID_TABLE_ADD_VALUE || Table_pointer >= MAX_TABLE_SIZE || isnan(Table_pointer))
    {
        Table_pointer = EEPROM_USER_ID_TABLE_ADD_VALUE;
        Write_EEPROM_Float(EEPROM_USER_TABLE_POINTER_ADD, EEPROM_USER_ID_TABLE_ADD_VALUE);
    }
    if (!isinf(KWh))
        Write_EEPROM_Float(EEPROM_Total_KWh_ADD, KWh + Read_EEPROM_Float(EEPROM_Total_KWh_ADD));
    else
        Write_EEPROM_Float(EEPROM_Total_KWh_ADD, 0.0);
    Write_EEPROM_String(Table_pointer, USER_ID);
    Write_EEPROM_Float(Table_pointer + OFFSET, KWh);
    Write_EEPROM_Float(Table_pointer + OFFSET + OFFSET, Session_Count);
    Write_EEPROM_String(Table_pointer + OFFSET + OFFSET + OFFSET, Transaction_ID);
    Write_EEPROM_Float(EEPROM_USER_TABLE_POINTER_ADD, Table_pointer + EEPROM_USER_FIELD_ADD_OFFSET);
    Serial.println();
    Serial.printf("Total KWh Usage: %0.6f\n", Read_EEPROM_Float(EEPROM_Total_KWh_ADD));
    Serial.printf("Table Pointer: %0.1f\n", Read_EEPROM_Float(EEPROM_USER_TABLE_POINTER_ADD));
    for (uint16_t i = EEPROM_USER_ID_TABLE_ADD_VALUE; i < MAX_TABLE_SIZE; i += EEPROM_USER_FIELD_ADD_OFFSET)
        Serial.printf("User %d: %s\t KWh: %0.6f\t Session Sec: %0.2f\t Transaction ID: %s\n",
                      (i / EEPROM_USER_FIELD_ADD_OFFSET) - 1,
                      Read_EEPROM_String(i).c_str(),
                      Read_EEPROM_Float(i + OFFSET),
                      Read_EEPROM_Float(i + OFFSET + OFFSET),
                      Read_EEPROM_String(i + OFFSET + OFFSET + OFFSET).c_str());
    Serial.println();
}
