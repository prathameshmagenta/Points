#ifndef __V_VInformationManager_H__
#define __V_VInformationManager_H__

#include <Arduino.h>
#include <map>
#include "VSingleAlertSetting.h"
#include "VDeviceModelSingleSensorInfo.h"

String writeKeyValuesToFile(String instrSpiffFilePath, std::map<String, String>& inMapKeyValuesToWrite);
String readKeyValuesFromFile(String instrSpiffFilePath, std::map<String, String>& inMapKeyValueResult);

String writeDeviceInfoToFile(std::map<String, String>& mapDeviceInfo);
String readDeviceInfoFromFile(std::map<String, String>& mapDeviceInfo);

String writeConnectionInfoToFile(std::map<String, String>& mapConnectionInfo);
String readConnectionInfoFromFile(std::map<String, String>& mapConnectionInfo);

String writeAlertSettingsToFile(std::map<String, CVSingleAlertSetting>& inMapAlertSettings);
String readAlertSettingsFromFile(std::map<String, CVSingleAlertSetting>& inMapAlertSettingsToModify);

String writeDeviceModelSensorInfoToFile(std::map<String, CVDeviceModelSingleSensorInfo>& inMapDeviceModelSensorInfo);
String readDeviceModelSensorInfoFromFile(std::map<String, CVDeviceModelSingleSensorInfo>& inMapDeviceModelSensorInfoToModify);

String writeCalibrationInfoToFile(std::map<String, uint32_t>& mapCaliInfo);
String readCalibrationInfoFromFile(std::map<String, uint32_t>& mapCaliInfo); 

String writeDeviceDataToFileForBackup(std::map<String, double>& mapDeviceDataInfo, uint16_t NewIndexCountToBeAppended);
String readBackUpStringFromFile (uint16_t AppendedIndexVal, std::map<String, double>& inBackedUpMapDeviceValues, String &outFinalExtractedString, String &outstrfileStat);
String writeBackupDataIntoTempFile(String StringinTempFile);
String readBackedUpDataInfo(std::map<String, double>& inBackedUpMapDeviceValues, String &ExtractedTimeStamp);

String ExtractActualTimeStamp(uint32_t RtcDate, uint32_t RtcTime);
String ExtractStringFromSpecificIndex(String BackupDataStringToSplit, String instrAppendedIndexVal, int loopVal, String &outstrFinalString, uint8_t IndexLength);

String writeCalibDoubleValuesToFile (std::map<String, double>& inMapCalibSensorValInfo);
String ReadCalibDoubleValuesFromFile (std::map<String, double>& inMapCalibSensorInfoVal);

#endif // __V_VInformationManager_H__