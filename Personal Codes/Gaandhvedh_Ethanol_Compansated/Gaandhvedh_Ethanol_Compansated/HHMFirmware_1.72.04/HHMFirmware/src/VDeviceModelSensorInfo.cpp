#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <map>
#include "VDefines.h"
#include "VUtils.h"
#include "VDeviceModelSensorInfo.h"
#include "VInformationManager.h"

String getDeviceModelSensorInfoFromServer(std::map<String, String>& inMapConnectionInfo, 
                                          std::map<String, String>& inMapDeviceInfo,
                                          std::map<String, CVDeviceModelSingleSensorInfo>& inMapDeviceModelSensorInfo ){

  String lstrRetVal = STATUS_SUCCESS;

  if( inMapDeviceInfo.size() <= 0 || inMapConnectionInfo.size() <= 0 ||
      inMapConnectionInfo.find(DEVC_API_URL) == inMapConnectionInfo.end() ||
      inMapDeviceInfo.find(V_DEVICE_ID) == inMapDeviceInfo.end() ||
      inMapDeviceInfo.find(V_DEVICE_AUTH) == inMapDeviceInfo.end()
  ) {

    Serial.println("Required Keys are missing from the DeviceInfo/Connection Map.");

    lstrRetVal = REQUIRED_KEYS_MISSING_IN_MAP; // Required Keys are missing from the DeviceInfo/Connection Map.
    return lstrRetVal;
  }

  // Request for getting Alert Settings
  HTTPClient http;

  String lstrMessage = "";
  DeserializationError err;

  String strPostBody =  String("{") +
                          "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                          "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\""
                        "}";


  http.begin( inMapConnectionInfo[DEVC_API_URL] + "vdevice/getModelInfo/");  // Specify destination for HTTP request
  http.addHeader("Content-Type", "application/json"); // Specify content-type header

  Serial.println("Sending Request for Device Model Sensor Info");
  // To Hide Device Auth.
  // Serial.println(strPostBody);
  
  int httpResponseCode = http.POST(strPostBody); // Send the actual POST request

  if( httpResponseCode > 0 ) {
    if(httpResponseCode == HTTP_CODE_OK) {

      String response = http.getString(); // Get the response to the request
      Serial.println("Response from Server: ");
      Serial.print(response);
      JsonObject objResponseJson;
      String strDeviceModelSensorInfo;      
      DynamicJsonDocument docResponseJson(4096);
      err = deserializeJson(docResponseJson, response);
      if (err) {
        Serial.print("Error while converting Http Response JSON String to JSON Document");
        Serial.println(err.c_str());
      } else {
        objResponseJson = docResponseJson.as<JsonObject>();  

        if( objResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
            objResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE) &&
            objResponseJson.containsKey("DeviceModelSensorInfo") ) {

          // Get required information from JSON Object
          strDeviceModelSensorInfo = objResponseJson["DeviceModelSensorInfo"].as<String>();

          JsonObject objModelSensorInfoJson;
          DynamicJsonDocument doc2(4096);
          err = deserializeJson(doc2, strDeviceModelSensorInfo);
          if (err) {
            Serial.print("Error while converting DeviceModelSensorInfoJSON String to JSON Document. Error: ");
            Serial.println(err.c_str());
          } else {
            objModelSensorInfoJson = doc2.as<JsonObject>();
          }
        
          DynamicJsonDocument docSingleSensorInfo(4096);
          JsonObject objJsonSingleDeviceModelSensorInfo;
          for (JsonPair p : objModelSensorInfoJson) {
            String strMeasuredParam = String( p.key().c_str() );
            String strSingleSensorInfo = p.value().as<String>();

            CVDeviceModelSingleSensorInfo lobjSingleSensorInfo;

            if( lobjSingleSensorInfo.initializeObjectFromJsonString(strSingleSensorInfo) ) {
              // Fill the alert setting in global map to be used later
              inMapDeviceModelSensorInfo[strMeasuredParam] = lobjSingleSensorInfo;
            } else {
              Serial.println( "Should not happen. Error while converting Json String for Measured Param [" + 
                strMeasuredParam + "] to Model Sensor Information object."
              );
              continue; // Skip this Model Sensor Information             
            }

          }

        } else {
          if( objResponseJson.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
            lstrMessage = objResponseJson[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
          } else {
            lstrMessage = "Should not happen. Http Get Device model Sensor Info failed.";
          }
          Serial.println(lstrMessage);

          lstrRetVal = objResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) ?
                       objResponseJson[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
        }

      }
  
    } else {

      lstrRetVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
      Serial.printf("\nHTTP Post failed while getting Model Sensor Information. Result Not HTTP_CODE_OK.");
    }

  } else {

    lstrRetVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;   
    Serial.printf("\nHTTP Post failed while getting Model Sensor Information. Error: %s", http.errorToString(httpResponseCode).c_str());
  }

  http.end();  // Free resources

  return lstrRetVal;
  
}

