#include <map>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "VInformationManager.h"
#include "VUtils.h"
#include "VDefines.h"
#include "VDeviceData.h"
#include "VImageUpgrade.h"
#include "VCalibrationAPIExecution.h"
#include "VCalibrationSensorCalculation.h"

/* GLOBAL SENSOR VARIABLES */
extern float NH3ODVal;
extern float VOCVal; 
extern float TEMPVal;
extern float HUMVal;

String strPostBodyDevDtata = "";

bool isDevcinCalibrationMode = false;
String strAckDevRes = "";
String strBackLogData = "";

String strDevcinfo = "";
bool NewAlertSet = false;

extern std::map<String, uint32_t> mapCalibrationInfo;


String sendDeviceDataToServer(std::map<String, String>& inMapConnectionInfo, 
                              std::map<String, String>& inMapDeviceInfo,
                              std::map<String, double>& inMapDeviceValues,
                              String TimeStamp,
                              String RTCTimeStat,
                              bool isdevcbklogdata ){

  String strRetVal = DEVICE_API_RESPONSE_SUCCESS_CODE; 
  String strNewOTAinfo ="";
  String objNewFwID ="";
  String objNewUpdateKey ="";

  String strRTCinfo = "";
  String strdevcDT = "";

  
  String strSPIFFSretVal = "";
  String strNewDevcName = "";
  String strNewDevcPinCode = "";
  String strNewDevcVicinityType = "";
  uint8_t  NewDevcIntervalTime = 0;
  String  CountryCode = "";


  if( inMapDeviceInfo.size() <= 0 || inMapConnectionInfo.size() <= 0 ||
      inMapConnectionInfo.find(DEVC_API_URL) == inMapConnectionInfo.end() ||
      inMapDeviceInfo.find(V_DEVICE_ID) == inMapDeviceInfo.end() ||
      inMapDeviceInfo.find(V_DEVICE_AUTH) == inMapDeviceInfo.end()
  ) {

    Serial.println("Required Keys are missing from the DeviceInfo/Connection Map.");
    
    strRetVal = REQUIRED_KEYS_MISSING_IN_MAP; // Required Keys are missing from the DeviceInfo/Connection Map.
    return strRetVal;
  }

  std::map<String, double> ::iterator itDeviceValues;
  String strMeasuredParam = "";
  int mapDeviceValuesSize = inMapDeviceValues.size() - 1;
  int indexOfSingleParam = 0;

  for(itDeviceValues = inMapDeviceValues.begin(); itDeviceValues != inMapDeviceValues.end(); itDeviceValues++) {

    if( mapDeviceValuesSize  == indexOfSingleParam ) { // Avoid comma for last parameter
      strMeasuredParam +=  "\""+ itDeviceValues->first + "\":" + itDeviceValues->second + " ";
    } else {
      strMeasuredParam +=  "\""+ itDeviceValues->first + "\":" + itDeviceValues->second + "," ;
    }
    
    indexOfSingleParam ++;
  }

  // If it's fresh data then send IsBkLog as false, else its some old backlog data then keep it as true.
  strBackLogData = (isdevcbklogdata == true) ? "true" : "false";

  if(RTCTimeStat == RTC_DATE_TIME_INVALID) {

    strPostBodyDevDtata =  String("{") +
                          "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\", " +
                          "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\", " +
                          " \"Status\": \"1\", " +
                          " \"MeasuredParams\": { " + strMeasuredParam + ",\"IsBkLog\": " + strBackLogData + "}, " +
                          " \"LogTime\": null "
                          "}";

    

  } else { 

    strPostBodyDevDtata =  String("{") +
                          "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\", " +
                          "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\", " +
                          " \"Status\": \"1\", " +
                          " \"MeasuredParams\": { " + strMeasuredParam + ",\"IsBkLog\": " + strBackLogData + "}, " +
                          " \"LogTime\": \"" + TimeStamp + "\" "
                          "}";

  }
 
 
  HTTPClient http;
  DeserializationError err;
  String lstrMessage;

  Serial.print("URL: ");
  Serial.print(inMapConnectionInfo[DEVC_API_URL]);
  Serial.println("vdevice/saveDeviceCurrentReadings/");
  
  http.begin( inMapConnectionInfo[DEVC_API_URL] + "vdevice/saveDeviceCurrentReadings/");  //Specify destination for HTTP request
  
  http.addHeader("Content-Type", "application/json");             //Specify content-type header
  // To Hide Device Auth.
  // Serial.println(strPostBodyDevDtata);
  
  int httpResponseCode = http.POST(strPostBodyDevDtata);   //Send the actual POST request
 
  if(httpResponseCode > 0) { 

    if(httpResponseCode == HTTP_CODE_OK) {

      String response = http.getString(); 
      JsonObject objJSONResponse;
      DynamicJsonDocument doc(1024);
      err = deserializeJson(doc, response);
      if (err) {
        Serial.print("Error while converting Http Response JSON String to JSON Document");
        Serial.println(err.c_str());
        strRetVal = ERR_DESERIALIZE_JSON_FAILED;

      } else {
        objJSONResponse = doc.as<JsonObject>();
        Serial.println(response);           //Print request answer

        if( objJSONResponse.containsKey(DEVICE_API_RESPONSE_CODE) &&
            objJSONResponse[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE)) {

          // Device Data Readings Sent Successfully.
          Serial.println("Success");

          // Check RTC Info simultaneously.
          if(objJSONResponse.containsKey(RES_KEY_RTC_INFO)) {

            Serial.println("RTC Information present.Checking Status");
            //Check if RTC time has any issue
            strRTCinfo = objJSONResponse[RES_KEY_RTC_INFO].as<String>();
            JsonObject objRTCInfo = {};
            DynamicJsonDocument docRTCInfo(1024);
            err = deserializeJson(docRTCInfo, strRTCinfo);

            if (err) {
              Serial.print("Error while converting RTCInfo Arduino String to JSON Document");
              Serial.println(err.c_str());
            } else {
              //Convert into a Json object.
              objRTCInfo = docRTCInfo.as<JsonObject>();

              if(objRTCInfo.containsKey(RES_KEY_RTC_ERROR)) {

                Serial.println("Issue Found in RTC.Displaying wrong time.Need to Reset it");
                strdevcDT = objRTCInfo[RES_KEY_DEVC_DT_TM].as<String>();
                Serial.println("The time displayed by the device is: ");
                Serial.println(strdevcDT);
                // Call now getUTCDateTime API.
                FiregetCurrentUTCDateTime();

              } else {

                Serial.println("RTCInfo found as null.No issue in RTC Found");
              }

            }

          }

        } else if( objJSONResponse.containsKey(DEVICE_API_RESPONSE_CODE) &&
            objJSONResponse[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_NEW_DEVC_UPDATES_AVAILABLE)) {
              
          //Check if new FW ver is available
          if( objJSONResponse.containsKey(RES_KEY_OTA_INFO) == true ) {

            Serial.println("New FW updates available");
            //Check if new setting is for that particular item
            strNewOTAinfo = objJSONResponse[RES_KEY_OTA_INFO].as<String>();
            JsonObject objNewOTAInfo = {};
            DynamicJsonDocument docNewFwInfo(1024);
            err = deserializeJson(docNewFwInfo, strNewOTAinfo);

            if (err) {
              Serial.print("Error while converting OTAInfo Arduino String to JSON Document");
              Serial.println(err.c_str());
            } else {          
              //Convert into a Json object.
              objNewOTAInfo = docNewFwInfo.as<JsonObject>();
          
              if( objNewOTAInfo.containsKey(RES_KEY_OTA_FW_ID) && 
                  objNewOTAInfo.containsKey(RES_KEY_OTA_UPD_KEY) ) {

                objNewFwID = objNewOTAInfo[RES_KEY_OTA_FW_ID].as<String>();
                objNewUpdateKey = objNewOTAInfo[RES_KEY_OTA_UPD_KEY].as<String>();

                image_setNewFWAvailability();
                inMapDeviceInfo[RES_KEY_OTA_FW_ID] = objNewFwID;
                inMapDeviceInfo[RES_KEY_OTA_UPD_KEY] = objNewUpdateKey;
                // Store the key and ID in the SPIFFS so that it can be used for registration after OTA is successful.
                writeDeviceInfoToFile(inMapDeviceInfo);
                
                Serial.print("FW-ID: "); 
                Serial.println(objNewFwID); 
                Serial.print("Update-key: "); 
                Serial.println(objNewUpdateKey); 

              } else {
                Serial.println("But missing FW-ID or Update-key."); 
              }

            }
          
          }

          // Check for new Device Updates simultaneously.
          if( objJSONResponse.containsKey(RES_KEY_DEVICE_INFO) == true ) {

            Serial.println("New Device Updates are available.Saving them accordingly.");
            strDevcinfo = objJSONResponse[RES_KEY_DEVICE_INFO].as<String>();
            Serial.println(strDevcinfo);
            
            JsonObject objDevcInfo = {};
            DynamicJsonDocument docDevcInfo(1024);
            err = deserializeJson(docDevcInfo, strDevcinfo);

            if (err) {
              Serial.print("Error while converting Device Info Arduino String to JSON Document");
              Serial.println(err.c_str());
            } else {
              // Convert into a Json object.
              objDevcInfo = docDevcInfo.as<JsonObject>();
              // Suppose a new Device Name is entered from the webpage.
              if(objDevcInfo.containsKey(RES_KEY_DEVICE_NAME)) {

                Serial.println("New Device Name entered from the web page.Saving it");

                strNewDevcName = objDevcInfo[RES_KEY_DEVICE_NAME].as<String>();

                inMapDeviceInfo[V_DEVICE_NAME] = strNewDevcName;
                strSPIFFSretVal = writeDeviceInfoToFile(inMapDeviceInfo);
              }

              // Suppose a new Pin Code is entered from the webpage.
              if(objDevcInfo.containsKey(RES_KEY_DEVICE_PINCODE)) {

                Serial.println("New Pin Code entered from the web page.Saving it");

                strNewDevcPinCode = objDevcInfo[RES_KEY_DEVICE_PINCODE].as<String>();

                inMapDeviceInfo[V_DEVICE_POSTALCODE] = strNewDevcPinCode;
                strSPIFFSretVal = writeDeviceInfoToFile(inMapDeviceInfo);
              }

              // Suppose the Vicinity Type is entered from the webpage.
              if(objDevcInfo.containsKey(RES_KEY_DEVICE_VICINITY_TYPE)) {

                Serial.println("New Vicinity Type entered from the web page.Saving it");

                strNewDevcVicinityType = objDevcInfo[RES_KEY_DEVICE_VICINITY_TYPE].as<String>();

                inMapDeviceInfo[V_DEVICE_ROOMTYPE] = strNewDevcVicinityType;
                strSPIFFSretVal = writeDeviceInfoToFile(inMapDeviceInfo);
              }

              // Suppose the Data Interval is changed from the webpage.
              if(objDevcInfo.containsKey(RES_KEY_DEVICE_DATA_SENT_INTERVAL)) {

                Serial.println("New Data Interval Time entered from the web page.Saving it");

                NewDevcIntervalTime = objDevcInfo[RES_KEY_DEVICE_DATA_SENT_INTERVAL].as<int>();

                mapCalibrationInfo[DATA_FILTRATION_DELAY] = NewDevcIntervalTime;
                strSPIFFSretVal = writeCalibrationInfoToFile(mapCalibrationInfo);
              }

              // Suppose the Country Code is entered from the webpage.
              if(objDevcInfo.containsKey(RES_KEY_DEVICE_COUNTRY_CODE)) {

                Serial.println("Country Code entered from the web page.Saving it");

                CountryCode = objDevcInfo[RES_KEY_DEVICE_COUNTRY_CODE].as<String>();

                inMapDeviceInfo[COUNTRY_CODE] = CountryCode;
                strSPIFFSretVal = writeDeviceInfoToFile(inMapDeviceInfo);
              }

              // Send ack to server if saving is successful so that the server doesnt send that again. 
              if(strSPIFFSretVal == STATUS_SUCCESS) {
                
                Serial.println("New Device Name/Pin Code/Vicinity Type/Data Interval/CountryCode stored successfully in SPIFFS.Firing DeviceUpdate acknowledgement.");
                SendAcknowledgementforDeviceUpdates();
              } else {

                Serial.println("Sorry some error occured while saving New Device Name/Pin Code/Vicinity Type/Data Interval/CountryCode into SPIFFS");
                Serial.println(strSPIFFSretVal);
              }

            }

          }

          // Check for new Alert Settings too simultaneously.
          if( objJSONResponse.containsKey(RES_KEY_SETTINGS_INFO) == true ) {

            Serial.println("New Alert Settings set by the user.Saving them accordingly.");             
            strSPIFFSretVal = fillAlertSettingsMap();
            // Send ack to server if saving is successful so that the server doesnt send that again. 
            if(strSPIFFSretVal == STATUS_SUCCESS) {
              
              Serial.println("New Alert Settings stored successfully in SPIFFS.Firing DeviceUpdate acknowledgement.");
              NewAlertSet = true;
              SendAcknowledgementforDeviceUpdates();
              NewAlertSet = false;
              Serial.println("Flag for new Alert Settings is made false.");
            } else {

              Serial.println("Sorry some error occured while saving New Alert Settings into SPIFFS");
              Serial.println(strSPIFFSretVal);
            }            

          }

          // Check for RTC issue simultaneously.
          if( objJSONResponse.containsKey(RES_KEY_RTC_INFO) == true ) {

            Serial.println("RTC Information present.Checking Status");
            //Check if RTC time has any issue
            strRTCinfo = objJSONResponse[RES_KEY_RTC_INFO].as<String>();
            JsonObject objRTCInfo = {};
            DynamicJsonDocument docRTCInfo(1024);
            err = deserializeJson(docRTCInfo, strRTCinfo);

            if (err) {
              Serial.print("Error while converting RTCInfo Arduino String to JSON Document");
              Serial.println(err.c_str());
            } else {
              //Convert into a Json object.
              objRTCInfo = docRTCInfo.as<JsonObject>();

              if(objRTCInfo.containsKey(RES_KEY_RTC_ERROR)) {

                Serial.println("Issue Found in RTC.Displaying wrong time.Need to Reset it");
                strdevcDT = objRTCInfo[RES_KEY_DEVC_DT_TM].as<String>();
                Serial.println("The time displayed by the device is: ");
                Serial.println(strdevcDT);
                // Call now getUTCDateTime API.
                FiregetCurrentUTCDateTime();

              } else {

                Serial.println("RTCInfo found as null.No issue in RTC Found");
              }

            }   

          }

        } else if( objJSONResponse.containsKey(DEVICE_API_RESPONSE_CODE) &&
            objJSONResponse[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVC_MARKED_FOR_CALIBRATION)) {

          // The device is set to calibration mode.
          Serial.println("The Device is set for Calibration.");
          // Also a flag is made High,if the device is set in Calibration Mode.SendDeviceData and Alerts won't be sent hereon.
          isDevcinCalibrationMode = true;
          Serial.println("The Calibration Flag is made true too");

        } else if( objJSONResponse.containsKey(DEVICE_API_RESPONSE_CODE) &&
            objJSONResponse[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVC_GET_OUT_OF_CALIB_MODE)) {

          // An Indication that the user wants to come out of the Calibration Mode.
          Serial.println("User has invoked to come out of calibration mode.Firing Ack API and executing MoveDeviceOutOfCalibrationMode function");
          strAckDevRes = SendAckForDeviceOutOfCalibMode();
          if(strAckDevRes != STATUS_SUCCESS) {
            // If 'SUCCESS' is not received, print the error message.
            Serial.println("Could not receive 'SUCCESS' for Acknowledgement for bringing device out of Calibration Mode.");
            Serial.println(strAckDevRes);
              
          } else {
            
            Serial.println("Received 'SUCCESS' for Acknowledgement for bringing device out of Calibration Mode.");
          }
          // Bring device out of the calibration mode irrespective of the status.
          MoveDeviceOutOfCalibrationMode();

        } else {
          if( objJSONResponse.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
            lstrMessage = objJSONResponse[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
          } else {
            lstrMessage = "Should not happen. Http Send Device Data failed.";
          }
          Serial.println(lstrMessage);

          strRetVal = objJSONResponse.containsKey(DEVICE_API_RESPONSE_CODE) ?
                      objJSONResponse[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
        }

      }

    } else {

      Serial.printf("\nHTTP Post failed while calling saveDeviceCurrentReadings. Result Not HTTP_CODE_OK");
      strRetVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
    } 

  } else {
    
    Serial.printf("\nHTTP Post failed while calling saveDeviceCurrentReadings. Error: %s\r\n", 
                                                      http.errorToString(httpResponseCode).c_str());
    strRetVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
  }
  
  http.end();  // Free resources

  return strRetVal;
  
}
