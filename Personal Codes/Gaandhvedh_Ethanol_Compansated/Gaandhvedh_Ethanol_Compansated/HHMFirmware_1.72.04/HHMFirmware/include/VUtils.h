#ifndef __V_UTILS_H__
#define __V_UTILS_H__

#include <xxtea-iot-crypt.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <map>

void initSPIFFS();
String encrypt(String PlainText);
String decrypt(String instrEncryptedContent);
void setSpecifiedIndicatorColorForLED(int inLEDColor);
void setLedRgbColor(int red_light_value, int green_light_value, int blue_light_value);
String connectToWifi(String instrSSID, String instrPass);
String ReadRTCTime(String &strRtcTimeDate, String &strRtcDate, String &strRtcTime);
void PrintRtcTime(String RtcDateTime, uint32_t &RTCUnixT);
void setRTCTimeFromUNIX(char *TimeVal);
void ViewClockError(char *ErrorVal);
String getDeviceID();
void ReadDeviceIDAuthKey();
String GetStringPartAtSpecificIndex(String StringToSplit, char SplitChar, int StringPartIndex);

#endif //__V_UTILS_H__