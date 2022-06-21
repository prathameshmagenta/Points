#include "VInformationManager.h"
#include "VDefines.h"
#include <SPIFFS.h>
#include <FS.h>
#include <xxtea-iot-crypt.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "VSingleAlertSetting.h"
#include "VUtils.h"

String ExtractedStringFromIndices = "";

uint32_t ExtractRtcDate;
String strExtractedRtcDate;
uint32_t ExtractRtcTime;
String strExtractedRtcTime;
String strExtractedRtcDateTime;
char charDateTime[20];
String ResultingTimeStamp;
String ExtractedTimeStamp;

String writeKeyValuesToFile(String instrSpiffFilePath, std::map<String, uint32_t>& inMapKeyValuesToWrite) {

  String lstrRetVal = STATUS_SUCCESS; // TODO: This variable to be filled based on error conditions

  std::map<String, uint32_t>::iterator row; 

  uint32_t CalibIntvalue = 0;
  String CalibParamName = "";
  String lstrCalibParam = "";
  String lstrIntCalibvalue = "";
  String strCalibvalue = "";


  if(inMapKeyValuesToWrite.size() <= 0) {
    Serial.println("Key-Value pair empty. Not able to write in file: " + instrSpiffFilePath);
    lstrRetVal = INFORMATION_MAP_EMPTY;
    return lstrRetVal;  
  }

  Serial.println("Writing Key-Value pair in file: " + instrSpiffFilePath);

  File f = SPIFFS.open(instrSpiffFilePath, FILE_WRITE);

  if( !f ) {
    Serial.println("Failed to open SPIFFS File: " + instrSpiffFilePath);
    lstrRetVal = FILE_OPEN_IN_WRITE_MODE_FAILED;
    return lstrRetVal;    
  } else {
    
    for (row = inMapKeyValuesToWrite.begin(); row != inMapKeyValuesToWrite.end(); row++) { 
       
      CalibParamName = row->first;  // Key is Measured Param
      CalibIntvalue  = row->second; // Value is the value of the corresponding Key.

      // Serial.printf("Param Names: %s\r\n", CalibParamName.c_str());
      // Serial.printf("Integer Values are: %d\r\n", CalibIntvalue);

      // The Integer Values are stored in a Char Buffer and then converted to String.
      char CaliIntbuff[100];
      sprintf(CaliIntbuff, "%d", CalibIntvalue);
      String lstrIntCalibvalue(CaliIntbuff);

      // Serial.println(lstrIntCalibvalue);
  
      lstrCalibParam = CalibParamName;

      if(f.print(lstrCalibParam) <= 0) {

        Serial.println("Should not happen. Key [" + lstrCalibParam + "] was not written into SPIFFS file: " + instrSpiffFilePath);
        lstrRetVal = FAILED_TO_WRITE;
      }        
      if(f.print(NEWLINE_CHAR) <= 0 ) {
        Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
        lstrRetVal = FAILED_TO_WRITE;
      };

      strCalibvalue = lstrIntCalibvalue;

      if(f.print(strCalibvalue) <= 0) {
        Serial.println("Should not happen. Value for Key [" + lstrCalibParam + "], was not written into SPIFFS file: [" + instrSpiffFilePath + "]");
        lstrRetVal = FAILED_TO_WRITE;
      }          
      if(f.print(NEWLINE_CHAR) <= 0 ) {
        Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
        lstrRetVal = FAILED_TO_WRITE;
      };
    }

    f.close(); // Close the file after all Key/Values written 
  }

  return lstrRetVal; 
}

String readKeyValuesFromFile(String instrSpiffFilePath, std::map<String, uint32_t>& inMapKeyValueResult) {

  String lstrRetVal = STATUS_SUCCESS; // TODO: This variable to be filled based on error conditions

  String strContentLine = "";
  String strSingleKeyFromFile = "";
  String lstrsingleValueFromFile = "";
  uint32_t singleValueFromFile = 0;

  inMapKeyValueResult.clear();

  if(SPIFFS.exists(instrSpiffFilePath)) {
    Serial.print( "File: " + instrSpiffFilePath + " exists, Size: " );

    File f = SPIFFS.open(instrSpiffFilePath, FILE_READ);
    
    Serial.println( f.size() );  

    if( !f || f.size() == 0 ) {
      Serial.println("Failed to open SPIFFS File or File Size is Zero: " + instrSpiffFilePath);

      lstrRetVal = FILE_OPEN_IN_READ_MODE_FAILED;
      return lstrRetVal;

    } else {
 
      while( f.available() > 0 ) {
        // Read and decrypt Single Calib Key
        strContentLine = f.readStringUntil(NEWLINE_CHAR);
        strSingleKeyFromFile = strContentLine;

        if( f.available() > 0 ) {
          // Read and decrypt Single Calib Int Value          
          // singleValueFromFile = f.parseInt() & 0xFFFFFFFF; 
          lstrsingleValueFromFile = f.readStringUntil(NEWLINE_CHAR);
          
          if( strSingleKeyFromFile.length() <= 0 ) {
            Serial.println( "Should not happen. Single Key from file is empty.");
            continue; // Skip this key/value info.   
          }

          singleValueFromFile = lstrsingleValueFromFile.toInt();
          inMapKeyValueResult[strSingleKeyFromFile] = singleValueFromFile;  // Fill Key/value in map.
          // Serial.println("Key of reading File: " + strSingleKeyFromFile);
          // Serial.println("Value: " + inMapKeyValueResult[strSingleKeyFromFile]);
          // Serial.println("Value of Reading File: " + strSingleValueFromFile);

        } else {
          Serial.println( "Should not happen. Single Key does not have a corresponding Value in file.");
        }

      } // WHILE

    }
    f.close(); // Read complete
  } else {
    Serial.println( "File: " + instrSpiffFilePath + " does not exist." );
  }
  return lstrRetVal; 
}

String writeKeyValuesToFile(String instrSpiffFilePath, std::map<String, String>& inMapKeyValuesToWrite) {

  String lstrRetVal = STATUS_SUCCESS; // TODO: This variable to be filled based on error conditions

  if(inMapKeyValuesToWrite.size() <= 0) {
    Serial.println("Input Key/Value Information map is empty. Cannot write to file: " + instrSpiffFilePath);
    lstrRetVal = INFORMATION_MAP_EMPTY;
    return lstrRetVal;  
  }

  Serial.println("Writing Key/Value Information to file: " + instrSpiffFilePath);

  File f = SPIFFS.open(instrSpiffFilePath, FILE_WRITE);

  if( !f ) {
    Serial.println("Failed to open SPIFFS File: " + instrSpiffFilePath);
    lstrRetVal = FILE_OPEN_IN_WRITE_MODE_FAILED;
    return lstrRetVal;    
  } else {
      String encryptedContentLine;
      std::map<String, String>::iterator it; 
      for (it = inMapKeyValuesToWrite.begin(); it != inMapKeyValuesToWrite.end(); it++) { 
        
        encryptedContentLine = encrypt(it->first); // Write Key on the next line

        if(f.print(encryptedContentLine) <= 0) {
          if(encryptedContentLine.length() <= 0) {
            Serial.println("Should not happen. Encrypted Key is empty for Key [" + it->first + "]");
          } else {
            Serial.println("Should not happen. Encrypted Key [" + it->first + "] was not written into SPIFFS file: " + instrSpiffFilePath);
          }
          lstrRetVal = FAILED_TO_WRITE;
        }

        if(f.print(NEWLINE_CHAR) <= 0 ) {
          Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
          lstrRetVal = FAILED_TO_WRITE;
        };
        
        encryptedContentLine = encrypt(it->second); // Write Value on the next line

        if(f.print(encryptedContentLine) <= 0) {
          if(encryptedContentLine.length() <= 0) {
            Serial.println("Should not happen. Encrypted Value is empty for Key [" + it->first + "]");  // Dont Serial print value as it might contain sensitive data.
          } else {
            Serial.println("Should not happen. Encrypted Value [" + encryptedContentLine + "] for Key [" + it->first + "] was not written into SPIFFS file: " + instrSpiffFilePath);
          }
          lstrRetVal = FAILED_TO_WRITE;
        } 

        if(f.print(NEWLINE_CHAR) <= 0 ) {
          Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
          lstrRetVal = FAILED_TO_WRITE;
        };

      }

      f.close(); // Close the file after all Key/Values written 
  }
  
  return lstrRetVal; 
}

String readKeyValuesFromFile(String instrSpiffFilePath, std::map<String, String>& inMapKeyValueResult) {

  String lstrRetVal = STATUS_SUCCESS; // TODO: This variable to be filled based on error conditions

  String strEncryptedContentLine;
  String strSingleKeyFromFile;
  String strSingleValueFromFile;

  inMapKeyValueResult.clear();

  if (SPIFFS.exists(instrSpiffFilePath)) {
    Serial.print( "File: " + instrSpiffFilePath + " exists, Size: " );

    File f = SPIFFS.open(instrSpiffFilePath, FILE_READ);
    
    Serial.println( f.size() );  

    if( !f || f.size() == 0 ) {
      Serial.println("Failed to open SPIFFS File or File Size is Zero: " + instrSpiffFilePath);

      lstrRetVal = FILE_OPEN_IN_READ_MODE_FAILED;
      return lstrRetVal;

    } else {
 
      while( f.available() > 0 ) {
        // Read and decrypt Single Key
        strEncryptedContentLine = f.readStringUntil(NEWLINE_CHAR);
        strSingleKeyFromFile = decrypt(strEncryptedContentLine);

        if( f.available() > 0 ) {
          // Read and decrypt Single Value
          strEncryptedContentLine = f.readStringUntil(NEWLINE_CHAR);
          strSingleValueFromFile = decrypt(strEncryptedContentLine);
          // Serial.println("Value after Decryption of reading file: " + strSingleValueFromFile);

          if( strSingleKeyFromFile.length() <= 0 || strSingleValueFromFile.length() <= 0 ) {
            Serial.println( "Should not happen. Single Key or corresponding Value from file is empty.");
            continue; // Skip this key/value info.   
          }

          inMapKeyValueResult[strSingleKeyFromFile] = strSingleValueFromFile;  // Fill Key/value in map.
          // Serial.println("Key of reading File: " + strSingleKeyFromFile);
          // Serial.println("Value: " + inMapKeyValueResult[strSingleKeyFromFile]);
          // Serial.println("Value of Reading File: " + strSingleValueFromFile);

        } else {
          Serial.println( "Should not happen. Single Key does not have a corresponding Value in file.");
        }

      } // WHILE

    }
    f.close(); // Read complete
  } else {
    Serial.println( "File: " + instrSpiffFilePath + " does not exist." );
  }
  return lstrRetVal; 
}

String writeConnectionInfoToFile(std::map<String, String>& inMapConnectionInfo) {
  String lstrRetVal = writeKeyValuesToFile(SPIFF_DEVICE_CONNECTION_INFO_FILE, inMapConnectionInfo);
  return lstrRetVal;
}

String readConnectionInfoFromFile(std::map<String, String>& inMapConnectionInfo) {
  String lstrRetVal = readKeyValuesFromFile(SPIFF_DEVICE_CONNECTION_INFO_FILE, inMapConnectionInfo);
  return lstrRetVal;
}

String writeDeviceInfoToFile(std::map<String, String>& inMapDeviceInfo) {
  String lstrRetVal = writeKeyValuesToFile(SPIFF_DEVICE_INFO_FILE, inMapDeviceInfo);
  return lstrRetVal;
}

String readDeviceInfoFromFile(std::map<String, String>& mapDeviceInfo) {
  String lstrRetVal = readKeyValuesFromFile(SPIFF_DEVICE_INFO_FILE, mapDeviceInfo);
  return lstrRetVal;
}

String writeCalibrationInfoToFile(std::map<String, uint32_t>& mapCaliInfo) {
  String lstrRetVal = writeKeyValuesToFile(SPIFF_DEVICE_CALIBRATION_FILE, mapCaliInfo);
  return lstrRetVal;
}

String readCalibrationInfoFromFile(std::map<String, uint32_t>& mapCaliInfo) {
  String lstrRetVal = readKeyValuesFromFile(SPIFF_DEVICE_CALIBRATION_FILE, mapCaliInfo);
  return lstrRetVal;
}


String writeAlertSettingsToFile (std::map<String, CVSingleAlertSetting>& inMapAlertSettings) 
{
  String lstrRetVal = STATUS_SUCCESS; // TODO: This variable to be filled based on error conditions

  if(inMapAlertSettings.size() <= 0) {
    Serial.println("Alert Settings map is empty. Cannot write to settings file.");
    lstrRetVal = ALERT_SETTINGS_MAP_EMPTY;
    return lstrRetVal;
  }

  std::map<String, CVSingleAlertSetting> ::iterator itAlertSettings;

  String lstrMeasuredParam = "";
  String lstrSingleAlertSettingJsonString = "";
  String encryptedContent = "";
  String strAlertSettingsFilePath = SPIFF_ALERT_SETTINGS_FILE;

  // Write the Alert Settings to SPIFF File
  Serial.println("Writing Alert Settings file");
  // Open the file for writing (this will create the file if it does not exist)
  File f = SPIFFS.open(strAlertSettingsFilePath, FILE_WRITE); // TODO: Handle error if the file cannot be opened

  if( !f ) {
    Serial.println("Failed to open File: " + strAlertSettingsFilePath);
    lstrRetVal = FILE_OPEN_IN_WRITE_MODE_FAILED;
    return lstrRetVal;
  } else {
      for ( itAlertSettings = inMapAlertSettings.begin(); itAlertSettings != inMapAlertSettings.end(); itAlertSettings++ ) {
        
        lstrMeasuredParam = itAlertSettings->first; // Key is Measured Param
        lstrSingleAlertSettingJsonString = itAlertSettings->second.toJsonString();

        // Write the measured param (encrypted) on separate line
        encryptedContent = encrypt(lstrMeasuredParam);

        if(f.print(encryptedContent) <= 0) {
          if(encryptedContent.length() <= 0) {
            Serial.println("Should not happen. Encrypted Key is empty for Key [" + lstrMeasuredParam + "]");
          } else {
            Serial.println("Should not happen. Encrypted Key [" + lstrMeasuredParam + "] was not written into SPIFFS file: " + strAlertSettingsFilePath);
          }
          lstrRetVal = FAILED_TO_WRITE;
        }        
        if(f.print(NEWLINE_CHAR) <= 0 ) {
          Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
          lstrRetVal = FAILED_TO_WRITE;
        };

        // Write the Single param alert setting json string (encrypted) on the line 
        // that follows the measured param
        encryptedContent = encrypt(lstrSingleAlertSettingJsonString);

        if(f.print(encryptedContent) <= 0) {
          if(encryptedContent.length() <= 0) {
            Serial.println("Should not happen. Encrypted Value is empty for Key [" + lstrMeasuredParam + "], Value [" + lstrSingleAlertSettingJsonString + "]");
          } else {
            Serial.println("Should not happen. Encrypted Value [" + encryptedContent + "] for Key [" + lstrMeasuredParam + "] was not written into SPIFFS file: " + strAlertSettingsFilePath);
          }
          lstrRetVal = FAILED_TO_WRITE;

        }          
        if(f.print(NEWLINE_CHAR) <= 0 ) {
          Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
          lstrRetVal = FAILED_TO_WRITE;
        };

      }

      f.close(); // Close the file after all settings written
  }

  return lstrRetVal; 
}

String readAlertSettingsFromFile (std::map<String, CVSingleAlertSetting>& inMapAlertSettingsToModify) {

  String lstrRetVal = STATUS_SUCCESS; // TODO: This variable to be filled based on error conditions

  inMapAlertSettingsToModify.clear(); // Initially clear the Map.

  String strAlertSettingsFilePath = SPIFF_ALERT_SETTINGS_FILE;

  if (SPIFFS.exists(strAlertSettingsFilePath)) {
    Serial.print( "File: " + strAlertSettingsFilePath + " exists, Size: " );

    File f = SPIFFS.open(strAlertSettingsFilePath, FILE_READ);

    Serial.println( f.size() );

    if( !f || f.size() == 0 ) {
      Serial.println("Failed to open SPIFFS File or File Size is Zero: " + strAlertSettingsFilePath);

      lstrRetVal = FILE_OPEN_IN_READ_MODE_FAILED;
      return lstrRetVal;
    } else {
        String strEncryptedContentLine;
        String strIgnoredNewLineString;
        String lstrMeasuredParam;
        String lstrSingleAlertSettingJsonString;
        while( f.available() > 0 ) {
          // Read and decrypt measured param
          strEncryptedContentLine = f.readStringUntil(NEWLINE_CHAR);
          lstrMeasuredParam = decrypt(strEncryptedContentLine);

          // Read and decrypt the corresponding measure param setting
          if( f.available() > 0 ) {
            strEncryptedContentLine = f.readStringUntil(NEWLINE_CHAR);
            lstrSingleAlertSettingJsonString = decrypt(strEncryptedContentLine);        

            if( lstrMeasuredParam.length() <= 0 || lstrSingleAlertSettingJsonString.length() <= 0 ) {
              Serial.println( "Should not happen. Measured Param or corresponding Alert setting is empty.");
              continue; // Skip this alert setting   
            }

            // lstrSingleAlertSettingJsonString = decrypt(strEncryptedContentLine);

            CVSingleAlertSetting lobjSingleAlertSetting;

            if( lobjSingleAlertSetting.initializeObjectFromJsonString(lstrSingleAlertSettingJsonString) ) {
              // Fill the alert setting in global map to be used later
              inMapAlertSettingsToModify[lstrMeasuredParam] = lobjSingleAlertSetting;
            } else {
              Serial.println( "Should not happen. Error while converting Json String for Measured Param [" + 
                lstrMeasuredParam + "] to Single Alert Setting object."
              );
              continue; // Skip this alert setting              
            }
          } else {
              Serial.println( "Should not happen. Key does not have a corresponding Value in File.");
          }
        
        } // WHILE

      }

    f.close(); // Read complete
  } else {
    Serial.println( "File: " + strAlertSettingsFilePath + " does not exist." );
    lstrRetVal = FILE_DOES_NOT_EXIT;
  }

  return lstrRetVal; 
}


String writeDeviceModelSensorInfoToFile (std::map<String, CVDeviceModelSingleSensorInfo>& inMapDeviceModelSensorInfo) 
{
  String lstrRetVal = STATUS_SUCCESS; // TODO: This variable to be filled based on error conditions

  if(inMapDeviceModelSensorInfo.size() <= 0) {
    Serial.println("Device Model Sensor Info map is empty. Cannot write to device model sensor info file.");
    lstrRetVal = DEVICE_MODEL_SENSOR_INFO_MAP_EMPTY;
    return lstrRetVal;
  }

  std::map<String, CVDeviceModelSingleSensorInfo> ::iterator itDeviceModelSensorInfo;

  String lstrMeasuredParam = "";
  String lstrDeviceModelSensorInfoJsonString = "";
  String paramName = "";
  String paramValue = "";
  String strDeviceModelSensorInfoFilePath = SPIFF_DEVICE_MODEL_SENSOR_INFO_FILE;

  // Write the Device Model Sensor Info to SPIFF File
  Serial.println("Writing Device Model Sensor Info file");
  // Open the file for writing (this will create the file if it does not exist)
  File f = SPIFFS.open(strDeviceModelSensorInfoFilePath, FILE_WRITE); // TODO: Handle error if the file cannot be opened

  if( !f ) {
    Serial.println("Failed to open File: " + strDeviceModelSensorInfoFilePath);
    lstrRetVal = FILE_OPEN_IN_WRITE_MODE_FAILED;
    return lstrRetVal;
  } else {
      for ( itDeviceModelSensorInfo = inMapDeviceModelSensorInfo.begin(); itDeviceModelSensorInfo != inMapDeviceModelSensorInfo.end(); itDeviceModelSensorInfo++ ) {
        
        lstrMeasuredParam = itDeviceModelSensorInfo->first; // Key is Measured Param
        lstrDeviceModelSensorInfoJsonString = itDeviceModelSensorInfo->second.toJsonString();

        paramName = lstrMeasuredParam;

        if(f.print(paramName) <= 0) {

          Serial.println("Should not happen. Key [" + lstrMeasuredParam + "] was not written into SPIFFS file: " + strDeviceModelSensorInfoFilePath);
          lstrRetVal = FAILED_TO_WRITE;
        }        
        if(f.print(NEWLINE_CHAR) <= 0 ) {
          Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
          lstrRetVal = FAILED_TO_WRITE;
        };

        paramValue = lstrDeviceModelSensorInfoJsonString;

        if(f.print(paramValue) <= 0) {
          Serial.println("Should not happen. Value for Key [" + lstrMeasuredParam + "], was not written into SPIFFS file: [" + strDeviceModelSensorInfoFilePath + "]");
          lstrRetVal = FAILED_TO_WRITE;
        }          
        if(f.print(NEWLINE_CHAR) <= 0 ) {
          Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
          lstrRetVal = FAILED_TO_WRITE;
        };
      }
      f.close(); // Close the file after all settings written
  }

  return lstrRetVal; 
}


String readDeviceModelSensorInfoFromFile (std::map<String, CVDeviceModelSingleSensorInfo>& inMapDeviceModelSensorInfoToModify) {

  String lstrRetVal = STATUS_SUCCESS; // TODO: This variable to be filled based on error conditions

  inMapDeviceModelSensorInfoToModify.clear(); // Initially clear the Map.

  String strDeviceModelSensorInfoFilePath = SPIFF_DEVICE_MODEL_SENSOR_INFO_FILE;

  if (SPIFFS.exists(strDeviceModelSensorInfoFilePath)) {
    Serial.print( "File: " + strDeviceModelSensorInfoFilePath + " exists, Size: " );

    File f = SPIFFS.open(strDeviceModelSensorInfoFilePath, FILE_READ);

    Serial.println( f.size() ); 

    if( !f || f.size() == 0 ) {
      Serial.println("Failed to open SPIFFS File or File Size is Zero: " + strDeviceModelSensorInfoFilePath);

      lstrRetVal = FILE_OPEN_IN_READ_MODE_FAILED;
      return lstrRetVal;
    } else {
        String strIgnoredNewLineString;
        String lstrMeasuredParam;
        String lstrDeviceModelSensorInfoJsonString;
        while( f.available() > 0 ) {
          // Read and decrypt measured param
          lstrMeasuredParam = f.readStringUntil(NEWLINE_CHAR);

          // Read and decrypt the corresponding device model sensor info
          if( f.available() > 0 ) {
            lstrDeviceModelSensorInfoJsonString = f.readStringUntil(NEWLINE_CHAR);

            if( lstrMeasuredParam.length() <= 0 || lstrDeviceModelSensorInfoJsonString.length() <= 0 ) {
              Serial.println( "Should not happen. Measured Param or corresponding Device Model Sensor Info is empty.");
              continue; // Skip this device model sensor info   
            }

            // lstrDeviceModelSensorInfoJsonString = decrypt(strEncryptedContentLine);

            CVDeviceModelSingleSensorInfo lobjDeviceModelInfo;

            if( lobjDeviceModelInfo.initializeObjectFromJsonString(lstrDeviceModelSensorInfoJsonString) ) {
              // Fill the device model sensor info in global map to be used later
              inMapDeviceModelSensorInfoToModify[lstrMeasuredParam] = lobjDeviceModelInfo;
            } else {
              Serial.println( "Should not happen. Error while converting Json String for Measured Param [" + 
                lstrMeasuredParam + "] to Device Model Sensor Info object."
              );
              continue; // Skip this device model sensor info              
            }
          } else {
              Serial.println( "Should not happen. Key does not have a corresponding Value in File.");
          }
        
        } // WHILE

      }

    f.close(); // Read complete
  } else {
    Serial.println( "File: " + strDeviceModelSensorInfoFilePath + " does not exist." );
    lstrRetVal = FILE_DOES_NOT_EXIT;
  }

  return lstrRetVal; 
}


/************************** Related to Memory Management *******************************/

String writeDeviceDataToFileForBackup (std::map<String, double>& mapDeviceDataInfo, uint16_t NewIndexCountToBeAppended) {

  String lstrRetVal = STATUS_SUCCESS; 
  
  String strBackupDeviceDataInfoFilePath = "";
  String strNewIndexCountToBeAppended = "";

  if(mapDeviceDataInfo.size() <= 0) {
    Serial.println("Device Data map is empty. Cannot write to device data info file.");
    lstrRetVal = DEVICE_DATA_INFO_MAP_EMPTY;
    return lstrRetVal;
  }

  std::map<String, double> ::iterator itDeviceDataInfo;

  String lstrMeasuredParam = "";
  String lstrDeviceDataVal = "";
  String paramName = "";
  String paramValue = "";

  // Write the Device Data Info to SPIFF File
  Serial.println("Writing Device Data Info file");

  File f;

  // Now the first dataset needs to be written, whereas the later ones needs to be appended to the first one.
  // If it's starting from 'ONE', then it should definitely start from First File. So pickup that First File.
  if(NewIndexCountToBeAppended == 1) {

    strBackupDeviceDataInfoFilePath = SPIFF_DEVICE_FIRST_BACKUP_SENSORDATA_FILE;

    // Open the file for writing (this will create the file if it does not exist)
    f = SPIFFS.open(strBackupDeviceDataInfoFilePath, FILE_WRITE);

  } else if( (NewIndexCountToBeAppended > 1) && (NewIndexCountToBeAppended <= FILE_LIMIT_ONE) ) {

    // If it's greater than 'ONE' and less than equal '240', then it should definitely append to First File. So pickup that First File.
    strBackupDeviceDataInfoFilePath = SPIFF_DEVICE_FIRST_BACKUP_SENSORDATA_FILE;

    // Open the file for writing and appending(this will create the file if it does not exist)
    f = SPIFFS.open(strBackupDeviceDataInfoFilePath, FILE_APPEND);

  } else if(NewIndexCountToBeAppended == (FILE_LIMIT_ONE + 1)) {

    // If it's starting from '241', then it should definitely start from Second File. So pickup that second File.
    strBackupDeviceDataInfoFilePath = SPIFF_DEVICE_SECOND_BACKUP_SENSORDATA_FILE;

    // Open the file for writing (this will create the file if it does not exist)
    f = SPIFFS.open(strBackupDeviceDataInfoFilePath, FILE_WRITE);

  } else if( (NewIndexCountToBeAppended > (FILE_LIMIT_ONE + 1)) && (NewIndexCountToBeAppended <= FILE_LIMIT_TWO) ) {

    // If it's greater than '241' and less than equal '400', then it should definitely append to Second File. So pickup that Second File.
    strBackupDeviceDataInfoFilePath = SPIFF_DEVICE_SECOND_BACKUP_SENSORDATA_FILE;
    
    // Open the file for writing and appending(this will create the file if it does not exist)
    f = SPIFFS.open(strBackupDeviceDataInfoFilePath, FILE_APPEND);
  } else {

    Serial.println("Please check the Index Count");
  }

  if( !f ) {
    Serial.println("Failed to open File: " + strBackupDeviceDataInfoFilePath);
    lstrRetVal = FILE_OPEN_IN_WRITE_MODE_FAILED;
    return lstrRetVal;
  } else {

    // Firstly, the dataset to be saved in the backup file needs to be appended with an Index eg.'1:' at the start of the dataset.
    strNewIndexCountToBeAppended = String(NewIndexCountToBeAppended) + ':';
    Serial.print("Index Appended at start ");
    Serial.println(strNewIndexCountToBeAppended);

    f.print(strNewIndexCountToBeAppended);

      for ( itDeviceDataInfo = mapDeviceDataInfo.begin(); itDeviceDataInfo != mapDeviceDataInfo.end(); itDeviceDataInfo++ ) {
        
        lstrMeasuredParam = itDeviceDataInfo->first; // Key is Measured Param
        lstrDeviceDataVal = itDeviceDataInfo->second;

        paramName = lstrMeasuredParam;

        if(f.print(paramName) <= 0) {

          Serial.println("Should not happen. Key [" + lstrMeasuredParam + "] was not written into SPIFFS file: " + strBackupDeviceDataInfoFilePath);
          lstrRetVal = FAILED_TO_WRITE;
        }        
        if(f.print(NEWLINE_CHAR) <= 0 ) {
          Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
          lstrRetVal = FAILED_TO_WRITE;
        };

        paramValue = lstrDeviceDataVal;

        if(f.print(paramValue) <= 0) {
          Serial.println("Should not happen. Value for Key [" + lstrMeasuredParam + "], was not written into SPIFFS file: [" + strBackupDeviceDataInfoFilePath + "]");
          lstrRetVal = FAILED_TO_WRITE;
        }          
        if(f.print(NEWLINE_CHAR) <= 0 ) {
          Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
          lstrRetVal = FAILED_TO_WRITE;
        };
      }
    
    // Secondly, the dataset to be saved in the backup file needs to be appended with an Index eg.'1:' even at the end of the dataset.
    strNewIndexCountToBeAppended = String(NewIndexCountToBeAppended) + ':';
    // Serial.print("Index Appended at end ");
    // Serial.println(strNewIndexCountToBeAppended);

    f.print(strNewIndexCountToBeAppended);

    f.close(); // Close the file after all data written and indices are appended at the start and end of the dataset.
  }

  return lstrRetVal; 
}


String readBackUpStringFromFile (uint16_t AppendedIndexVal, std::map<String, double>& inBackedUpMapDeviceValues, String &outFinalExtractedString, String &outstrfileStat) {

  String lstrRetVal = STATUS_SUCCESS; 

  String strBackupFileData = "";

  String strAppendedIndexVal = "";

  String lstrExtractionStat = "";

  String strBackedUpDataInfoFilePath = "";

  uint8_t AppendedIndexLength = 0;

  // An Array of String. Initialise as empty, Old data stored will be flushed.
  String ArrExtractedString[2] = {};

  inBackedUpMapDeviceValues.clear();  // Initially clear the Map.Very Important Step otherwise old data will also be sent.

  // Choose the File to be read according to the Index Appended.
  if(AppendedIndexVal <= FILE_LIMIT_ONE) {

    Serial.println("Picking data from First File");
    strBackedUpDataInfoFilePath = SPIFF_DEVICE_FIRST_BACKUP_SENSORDATA_FILE;

  } else if(AppendedIndexVal <= FILE_LIMIT_TWO) {

    Serial.println("Picking data from Second File");
    strBackedUpDataInfoFilePath = SPIFF_DEVICE_SECOND_BACKUP_SENSORDATA_FILE;
  } else {

    Serial.println("Check the Index Value for reading");
  }


  if (SPIFFS.exists(strBackedUpDataInfoFilePath)) {

    Serial.println( "File: " + strBackedUpDataInfoFilePath + " exists." );

    File f = SPIFFS.open(strBackedUpDataInfoFilePath, FILE_READ);

    Serial.println( f.size() ); 

    if( !f || f.size() == 0 ) {
      Serial.println("Failed to open SPIFFS File or File Size is Zero: " + strBackedUpDataInfoFilePath);

      lstrRetVal = FILE_OPEN_IN_READ_MODE_FAILED;

      return lstrRetVal;

    } else {

      // Firstly, the whole file of /FirstBackupSensorDataInfo.inf or /SecondBackupSensorDataInfo.inf needs to be read first and the whole data will be stored as a String.
      strBackupFileData = f.readString();

      if(strBackupFileData.length() <= 0) {

        Serial.println("Empty String obtained. Please check.");
      }

      // Now the Index Appended will be increasing in units, tens and thousands. So we need to calculate the length and split the character accordingly.
      // Also convert the integer(Index Value) to String.
      strAppendedIndexVal = String(AppendedIndexVal) + ':';

      AppendedIndexLength = strAppendedIndexVal.length();
      // Serial.printf("Appended Index Length is %d\r\n", AppendedIndexLength);

      // Next after the whole file contents is received as a String, next we need to extract the part of data we actually want using
      // appending and terminating Indices.
      for (int loop = 0; loop < 2; loop++)	{

        // Give this function your string to split, your Index to split and the actual index of the StringPart you want to extract.
        lstrExtractionStat = ExtractStringFromSpecificIndex(strBackupFileData, strAppendedIndexVal, loop, ExtractedStringFromIndices, AppendedIndexLength);        
        ArrExtractedString[loop] = ExtractedStringFromIndices;

      }

      if(lstrExtractionStat == STATUS_SUCCESS) {
         
        outFinalExtractedString = ArrExtractedString[1];
        // Serial.println("The String successfully extracted is: ");
        // Serial.println(outFinalExtractedString);

      } else if(lstrExtractionStat == STATUS_FAILED) {

        Serial.printf("Some Error occured while extracting the String which is %s\n", ArrExtractedString[1].c_str());
      }

    }

    f.close(); // Read complete

  } else {

    Serial.println( "File: " + strBackedUpDataInfoFilePath + " does not exist." );
    lstrRetVal = FILE_DOES_NOT_EXIT;
  }

  // Check both file status and extraction status too.
  outstrfileStat = lstrRetVal;

  return lstrExtractionStat;

}

String writeBackupDataIntoTempFile(String StringinTempFile) {

  String strTempBackupDataInfoFilePath = "";

  String lstrTempRetVal = STATUS_SUCCESS;

  // Now the Extracted String is written to another temporary file so that the Key and Value can be extracted and written to map.
  strTempBackupDataInfoFilePath = SPIFF_DEVICE_TEMPORARY_BACKUPDATA_FILE;

  // Write the Device Data Info to SPIFF File
  // Serial.println("Writing Extracted Device Data String into temporary file.");

  File Tempfile;

  // Open the file for writing (this will create the file if it does not exist)
  Tempfile = SPIFFS.open(strTempBackupDataInfoFilePath, FILE_WRITE);

  if( !Tempfile ) {

    Serial.println("Failed to open File: " + strTempBackupDataInfoFilePath);

    lstrTempRetVal = FILE_OPEN_IN_WRITE_MODE_FAILED;

    return lstrTempRetVal;

  } else {

    Tempfile.print(StringinTempFile);
  
    Tempfile.close(); // Close the file after all settings written in the Temporary file.
  }

  return lstrTempRetVal;
      
}

String readBackedUpDataInfo(std::map<String, double>& inBackedUpMapDeviceValues, String &ExtractedTimeStamp) {

  String lstrBackUpRetVal = STATUS_SUCCESS; 

  inBackedUpMapDeviceValues.clear();  // Initially clear the Map.Very Important Step otherwise old data will also be sent.

  String strBackUpStringInfoFilePath = "";

  strBackUpStringInfoFilePath = SPIFF_DEVICE_TEMPORARY_BACKUPDATA_FILE;

  if (SPIFFS.exists(strBackUpStringInfoFilePath)) {

    Serial.println( "File: " + strBackUpStringInfoFilePath + " exists." );

    File f = SPIFFS.open(strBackUpStringInfoFilePath, FILE_READ);

    // Serial.println( f.size() ); 

    if( !f || f.size() == 0 ) {
      Serial.println("Failed to open SPIFFS File or File Size is Zero: " + strBackUpStringInfoFilePath);

      lstrBackUpRetVal = FILE_OPEN_IN_READ_MODE_FAILED;

      return lstrBackUpRetVal;

    } else {

      String strIgnoredNewLineString;
      String lstrMeasuredParam;
      String lstrDeviceDataVal;

      while( f.available() > 0 ) {
        // Read and measured param.
        lstrMeasuredParam = f.readStringUntil(NEWLINE_CHAR);

        // Read the corresponding device data info.
        if( f.available() > 0 ) {
          lstrDeviceDataVal = f.readStringUntil(NEWLINE_CHAR);

          if( lstrMeasuredParam.length() <= 0 || lstrDeviceDataVal.length() <= 0 ) {
            Serial.println( "Should not happen. Measured Param or corresponding Device Data Info is empty.");
            continue; // Skip this device data info   
          }

          Serial.println(lstrMeasuredParam + ":" + lstrDeviceDataVal);
          // Also copy back the contents of the Text File to the mapDeviceValues Map.
          inBackedUpMapDeviceValues[lstrMeasuredParam] = lstrDeviceDataVal.toDouble();
          
          if(lstrMeasuredParam == VMP_DATE) {
            // Extract the Date from the Text File into a variable and Erase this Date from the Map.
            ExtractRtcDate = inBackedUpMapDeviceValues[VMP_DATE];
            inBackedUpMapDeviceValues.erase(VMP_DATE);
            
          } else if(lstrMeasuredParam == VMP_TIME) {
            // Extract the Time from the Text File into a variable and Erase this Time from the Map.
            ExtractRtcTime = inBackedUpMapDeviceValues[VMP_TIME];
            inBackedUpMapDeviceValues.erase(VMP_TIME);

          }

        } else {

          Serial.println( "Should not happen. Key does not have a corresponding Value in File.");
        }
      
      } // WHILE

    }

    Serial.print("File size: ");
    Serial.println(f.size());

    f.close(); // Read complete.
    // Create the actual TimeStamp from the Extracted Date and Time and pass this TimeStamp to the SendDeviceData through this Function.
    ExtractedTimeStamp = ExtractActualTimeStamp(ExtractRtcDate, ExtractRtcTime);

  } else {

    Serial.println( "File: " + strBackUpStringInfoFilePath + " does not exist." );
    lstrBackUpRetVal = FILE_DOES_NOT_EXIT;
  }

  return lstrBackUpRetVal;
   
}


String ExtractActualTimeStamp(uint32_t RtcDate, uint32_t RtcTime) {

  strExtractedRtcDate = String(RtcDate);
  // Serial.println("The Date in String Format is:");
  // Serial.println(strExtractedRtcDate);
  
  strExtractedRtcTime = String(RtcTime);
  // Serial.println("The Time in String Format is:");
  // Serial.println(strExtractedRtcTime);

  strExtractedRtcDateTime = strExtractedRtcDate + strExtractedRtcTime;
  // Serial.println("The TimeStamp is:");
  // Serial.println(strExtractedRtcDateTime);

  int n = strExtractedRtcDateTime.length(); 

  // If the String Length is '13', meaning there is no Prefix 'ZERO' before Hour Value.
  // That prefix has got lost while converting Time to Double and storing it in File.
  // So we are adding Prefix 'ZERO' before single digit HOUR value.
  if(n == 13) {

    // We are adding Prefix 'ZERO' before Single Digit Hour Value. eg. 07:22:34.
    strExtractedRtcDateTime = strExtractedRtcDate + "0" + strExtractedRtcTime;
    Serial.println("The TimeStamp is:");
    Serial.println(strExtractedRtcDateTime);

  } else if(n == 12) {

    // We are adding Prefix 'ZERO ZERO' before Double Digit '00' ie. 12 Midnight HOUR value. eg. 00:22:34.
    strExtractedRtcDateTime = strExtractedRtcDate + "00" + strExtractedRtcTime;
    Serial.println("The TimeStamp is:");
    Serial.println(strExtractedRtcDateTime);

  } else if(n == 11) {

    // We are adding Prefix 'ZERO ZERO ZERO' before Double Digit '00' ie. 12 Midnight HOUR value and Single Digit Minute Value. eg. 00:02:34.
    strExtractedRtcDateTime = strExtractedRtcDate + "000" + strExtractedRtcTime;
    Serial.println("The TimeStamp is:");
    Serial.println(strExtractedRtcDateTime);

  } else if(n == 10) {

    // We are adding Prefix 'ZERO ZERO ZERO ZERO' before Double Digit '00' ie. 12 Midnight HOUR value and only Seconds Value. eg.00:00:34.
    strExtractedRtcDateTime = strExtractedRtcDate + "0000" + strExtractedRtcTime;
    Serial.println("The TimeStamp is:");
    Serial.println(strExtractedRtcDateTime);

  } else if(n == 9) {

    // We are adding Prefix 'ZERO ZERO ZERO ZERO ZERO' before Double Digit '00' ie. 12 Midnight HOUR value and Single Digit Seconds Value. eg. 00:00:04
    strExtractedRtcDateTime = strExtractedRtcDate + "00000" + strExtractedRtcTime;
    Serial.println("The TimeStamp is:");
    Serial.println(strExtractedRtcDateTime);

  } else if(n == 8) {

    // We are adding Prefix 'ZERO ZERO ZERO ZERO ZERO ZERO' before Double Digit '00' ie. 12 Midnight HOUR value and no Minute and Seconds Value. eg. 00:00:00
    strExtractedRtcDateTime = strExtractedRtcDate + "000000" + strExtractedRtcTime;
    Serial.println("The TimeStamp is:");
    Serial.println(strExtractedRtcDateTime);

  } 

  // Declaring character array 
  char char_array[n + 1]; 

  // Copying the contents of the string to char array 
  strcpy(char_array, strExtractedRtcDateTime.c_str()); 

  // for (int i = 0; i < n; i++) {
  //   Serial.println("Contents of Char Buffer");
  //   Serial.println(char_array[i]);
  // }

  snprintf(charDateTime, sizeof(charDateTime), "%1c%1c%1c%1c-%1c%1c-%1c%1c %1c%1c:%1c%1c:%1c%1c",
          char_array[0],
          char_array[1],
          char_array[2],
          char_array[3],
          char_array[4],
          char_array[5],
          char_array[6],
          char_array[7],
          char_array[8],
          char_array[9],
          char_array[10],
          char_array[11],
          char_array[12],
          char_array[13]);
  
  ResultingTimeStamp = charDateTime;
  Serial.println("The current TimeStamp is: ");
  Serial.println(ResultingTimeStamp);

  return ResultingTimeStamp;

}

String ExtractStringFromSpecificIndex(String BackupDataStringToSplit, String instrAppendedIndexVal, int loopVal, String &outstrFinalString, uint8_t IndexLength) {

	String originallyString = BackupDataStringToSplit;

	String ExtractedString = "";

  String ExtractionStat = "";

	for (int i11 = 0; i11 <= loopVal; i11++)
	{
    // If the for loop starts again reset the ExtractedString (in this case another part of the String is needed to be extracted).
		ExtractedString = "";										

    // Set the IndexOfAppendedIndex with the position of the instrAppendedIndexVal(eg. 1: or 2: etc.) in BackupDataStringToSplit.
		int IndexOfAppendedIndex = BackupDataStringToSplit.indexOf(instrAppendedIndexVal);	
    // Serial.printf("The Index Value is %d\n", IndexOfAppendedIndex);

		if (IndexOfAppendedIndex == -1)								// Is true, if no Char is found at the given Index.
		{
			// ExtractedString += "Error in GetStringPartAtSpecificIndex: No instrAppendedIndexVal found at String '" + originallyString + "' since StringPart '" + (i1-1) + "'";		//just to find Errors
      Serial.printf("error\n");

      outstrFinalString = ExtractedString;

      ExtractionStat = STATUS_FAILED;

			return ExtractionStat;
		}

		for (int i12 = 0; i12 < IndexOfAppendedIndex; i12++)
		{
      // Write the char at Position 0 of BackupDataStringToSplit to ExtractedString and go on Appending.
			ExtractedString += BackupDataStringToSplit.charAt(i12);			
      // Serial.printf("The ExtractedString value is %s\n", ExtractedString.c_str());
		}

    // Change the String to the Substring starting at the (position+IndexLength) where last Appended Index was found. We have found the Index Length as it can vary as ':4' or ':40' or '400' 
    // ie. IndexLength characters and we need to jump those many characters and split the String following this.
		BackupDataStringToSplit = BackupDataStringToSplit.substring(BackupDataStringToSplit.indexOf(instrAppendedIndexVal) + IndexLength);	
    // Serial.printf("The BackupDataStringToSplit value is %s\n", BackupDataStringToSplit.c_str());

	}

  // Serial.printf("The final ExtractedString value is %s\n", ExtractedString.c_str());

  outstrFinalString = ExtractedString;

	ExtractionStat = STATUS_SUCCESS;

	return ExtractionStat;

}

String writeCalibDoubleValuesToFile (std::map<String, double>& inMapCalibSensorValInfo) {

  String lstrSensorRetVal = STATUS_SUCCESS; // TODO: This variable to be filled based on error conditions

  if(inMapCalibSensorValInfo.size() <= 0) {
    Serial.println("Calib Values map is empty. Cannot write to Calib file.");
    lstrSensorRetVal = CALIB_VALUE_MAP_EMPTY;
    return lstrSensorRetVal;
  }

  std::map<String, double> ::iterator itCalibValInfo;

  String lstrCalibMeasuredParam = "";
  String lstrCalibValue = "";
  double CalibValueDouble = 0.0;
  String CalibparamName = "";
  String CalibparamValue = "";
  String strCalibInfoFilePath = SPIFF_DEVICE_CALIB_VALUE_FILE;

  // Write the Calib values Info to SPIFF File
  Serial.println("Writing Calib Sensor Info file");
  // Open the file for writing (this will create the file if it does not exist)
  File f = SPIFFS.open(strCalibInfoFilePath, FILE_WRITE); // TODO: Handle error if the file cannot be opened

  if( !f ) {
    Serial.println("Failed to open File: " + strCalibInfoFilePath);
    lstrSensorRetVal = FILE_OPEN_IN_WRITE_MODE_FAILED;
    return lstrSensorRetVal;
  } else {
      for ( itCalibValInfo = inMapCalibSensorValInfo.begin(); itCalibValInfo != inMapCalibSensorValInfo.end(); itCalibValInfo++ ) {
        
        lstrCalibMeasuredParam = itCalibValInfo->first; // Key is Measured Param
        CalibValueDouble = itCalibValInfo->second * 10000000;
        CalibValueDouble = (CalibValueDouble/10000000);

        // Serial.printf("Double Values are:%0.7f\r\n", CalibValueDouble);

        // We are multiplying and dividing with 10 to the power 7 so save the 7 digit precision after Demical which is very much required for SPEC and CO (Electrochemical)Sensors and also for Ammonia.
        // If we convert direct Double value to String the precision is only till 2 digits which is totally useless.
        char Calibuff[100];
        sprintf(Calibuff, "%1.07f", CalibValueDouble);
        String lstrCalibValue(Calibuff);

        // Serial.println(lstrCalibValue);
        CalibparamName = lstrCalibMeasuredParam;

        if(f.print(CalibparamName) <= 0) {

          Serial.println("Should not happen. Key [" + CalibparamName + "] was not written into SPIFFS file: " + strCalibInfoFilePath);
          lstrSensorRetVal = FAILED_TO_WRITE;
        }        
        if(f.print(NEWLINE_CHAR) <= 0 ) {
          Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
          lstrSensorRetVal = FAILED_TO_WRITE;
        };

        CalibparamValue = lstrCalibValue;

        if(f.print(CalibparamValue) <= 0) {
          Serial.println("Should not happen. Value for Key [" + CalibparamName + "], was not written into SPIFFS file: [" + strCalibInfoFilePath + "]");
          lstrSensorRetVal = FAILED_TO_WRITE;
        }          
        if(f.print(NEWLINE_CHAR) <= 0 ) {
          Serial.println("Should not happen. Failed to Add NEWLINE_CHAR");
          lstrSensorRetVal = FAILED_TO_WRITE;
        };
      }
      f.close(); // Close the file after all settings written
  }

  return lstrSensorRetVal;

}

String ReadCalibDoubleValuesFromFile (std::map<String, double>& inMapCalibSensorInfoVal) {

  double CalibSensorDoubleVal = 0.0;

  String lstrCalibStatVal = STATUS_SUCCESS; // TODO: This variable to be filled based on error conditions

  inMapCalibSensorInfoVal.clear(); // Initially clear the Map.

  String strCalibInfoValFilePath = SPIFF_DEVICE_CALIB_VALUE_FILE;

  if (SPIFFS.exists(strCalibInfoValFilePath)) {
    Serial.print( "File: " + strCalibInfoValFilePath + " exists, Size: " );   

    File f = SPIFFS.open(strCalibInfoValFilePath, FILE_READ);

    Serial.println( f.size() ); 

    if( !f || f.size() == 0 ) {
      Serial.println("Failed to open SPIFFS File or File Size is Zero: " + strCalibInfoValFilePath);

      lstrCalibStatVal = FILE_OPEN_IN_READ_MODE_FAILED;
      return lstrCalibStatVal;
    } else {
        String strIgnoredNewLineString;
        String lstrCalibValMeasuredParam;
        String lstrCalibValueString;
        while( f.available() > 0 ) {
          // Read and decrypt measured param
          lstrCalibValMeasuredParam = f.readStringUntil(NEWLINE_CHAR);

          // Read and decrypt the corresponding Calib Value
          if( f.available() > 0 ) {
            lstrCalibValueString = f.readStringUntil(NEWLINE_CHAR);

            if( lstrCalibValMeasuredParam.length() <= 0 || lstrCalibValueString.length() <= 0 ) {
              Serial.println( "Should not happen. Measured Param or corresponding Calib Value Info is empty.");
              continue; // Skip this device model sensor info   
            }
            // Serial.println(lstrCalibValMeasuredParam + ":" + lstrCalibValueString);

            // Also copy back the contents of the Text File to the mapDeviceValues Map.
            CalibSensorDoubleVal = lstrCalibValueString.toDouble();
            inMapCalibSensorInfoVal[lstrCalibValMeasuredParam] = CalibSensorDoubleVal;

          } else {
              Serial.println( "Should not happen. Key does not have a corresponding Value in File.");
          }
        
        } // WHILE

      }

    f.close(); // Read complete
  } else {
    Serial.println( "File: " + strCalibInfoValFilePath + " does not exist." );
  }

  return lstrCalibStatVal; 

}


