/* Medium example for ESP8266 (not for Arduino, uses additional Base64 layer) */
#include "Arduino.h"
#include "AES.h"
#include "AESLib.h"
#include "AES_config.h"
#include "base64.h"

AESLib aesLib;

String encrypt(char *msg, uint16_t msgLen, byte iv[]);
String decrypt(char *msg, uint16_t msgLen, byte iv[]);

char cleartext[256];
char ciphertext[512];

// AES Encryption Key
// byte aes_key[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
// byte aes_key[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte aes_key[] = {'A', 'A', 'A', 'A', 'B', 'B', 'B', 'B', 'C', 'C', 'C', 'C', 'D', 'D', 'D', 'D'};
// String aes_key = "AAAABBBBCCCCDDDD";

// General initialization veDtor (you must use your own IV's in production for full security!!!)
// byte aes_iv[N_BLOCK] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// byte aes_iv[N_BLOCK] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
byte aes_iv[N_BLOCK] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned long loopcount = 0;
byte enc_iv[N_BLOCK] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte dec_iv[N_BLOCK] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

String encrypt(char *msg, uint16_t msgLen, byte iv[])
{
  String En;
  int cipherlength = aesLib.get_cipher64_length(msgLen);
  char encrypted[cipherlength]; // AHA! needs to be large, 2x is not enough
  aesLib.encrypt64(msg, msgLen, encrypted, aes_key, sizeof(aes_key), iv);
  Serial.print("encrypted = ");
  Serial.println(encrypted);
  return String(encrypted);
}

String decrypt(char *msg, uint16_t msgLen, byte iv[])
{
  char decrypted[msgLen];
  aesLib.decrypt64(msg, msgLen, decrypted, aes_key, sizeof(aes_key), iv);
  return String(decrypted);
}

// Generate IV (once)
void aes_init()
{
  Serial.println("gen_iv()");
  aesLib.gen_iv(aes_iv);
}

void setup()
{
  Serial.begin(115200);
  aes_init();
  aesLib.set_paddingmode(paddingMode::CMS);

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

  Serial.println("Enter text to be encrypted into console (no feedback) and press ENTER (newline):");
}

/* non-blocking wait function */
void wait(unsigned long milliseconds)
{
  unsigned long timeout = millis() + milliseconds;
  while (millis() < timeout)
  {
    yield();
  }
}

void loop()
{
  if (Serial.available())
  {
    loopcount++;
    Serial.println(loopcount); // entry counter
    String readBuffer = Serial.readStringUntil('\n');
    Serial.println("INPUT:" + readBuffer);
    sprintf(cleartext, "%s", readBuffer.c_str()); // must not exceed 255 bytes; may contain a newline

    // Encrypt
    uint16_t clen = String(cleartext).length();
    String encrypted = encrypt(cleartext, clen, enc_iv);
    sprintf(ciphertext, "%s", encrypted.c_str());
    Serial.print("Ciphertext: ");
    Serial.println(encrypted.c_str());
    delay(1000);
    // Decrypt
    delay(1000);
    uint16_t dlen = encrypted.length();
    String decrypted = decrypt(ciphertext, dlen, dec_iv);
    Serial.print("Cleartext: ");
    Serial.println(decrypted.c_str());

    if (decrypted.equals(cleartext))
      Serial.println("SUCCES");
    else
      Serial.println("FAILURE");
    for (int i = 0; i < 16; i++)
    {
      enc_iv[i] = 0;
      dec_iv[i] = 0;
    }
  }
}
