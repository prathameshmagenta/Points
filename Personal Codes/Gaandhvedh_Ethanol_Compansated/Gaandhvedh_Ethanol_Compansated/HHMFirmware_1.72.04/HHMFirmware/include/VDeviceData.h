#ifndef __V_DEVICE_DATA_H__
#define __V_DEVICE_DATA_H__

#include <map>
#include <Arduino.h>

String fillAlertSettingsMap();

String sendDeviceDataToServer(std::map<String, String>& inMapConnectionInfo, 
                              std::map<String, String>& inMapDeviceInfo,
                              std::map<String, double>& inMapDeviceValues,
                              String TimeStamp,
                              String RTCTimeStat,
                              bool isdevcbklogdata );

// Generates random test data
String generateRandomSensorReadings(std::map<String, double>& inMapSensorReadings);

#endif // __V_DEVICE_DATA_H__
