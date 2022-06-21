#ifndef __V_ACCESSPOINTMANAGER_H__
#define __V_ACCESSPOINTMANAGER_H__

#include <Arduino.h>
#include <map>

extern std::map<String, String> mapDeviceInfo;
extern std::map<String, String> mapConnectionInfo;


String switchDeviceToAPServerMode(std::map<String, String>& inMapDeviceInfo);
String InitializeDeviceAPModeServer();
void getSSIDList();
void saveSSIDAndDeviceDetails();
void handleAPModeClient();
void handleFileUpload();
String getContentType(String filename);
bool handleFileRead(String path);

#endif // __V_ACCESSPOINTMANAGER_H__