#include<Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "VDefines.h"
#include "VUtils.h"
#include "VRegistration.h"
#include "VInformationManager.h"

// This flag is for OTA registration after successful OTA Completion.
bool isOTADone = false;

String registerDevice(std::map<String, String>& inMapConnectionInfo,
                      std::map<String, String>& inMapDeviceInfo,
                      std::map<String, uint32_t>& inMapCalibrationInfo) {

  String lstrRetVal = STATUS_SUCCESS;
  String strPostBody = "";

  if( inMapDeviceInfo.size() <= 0 || inMapConnectionInfo.size() <= 0 ||
      inMapConnectionInfo.find(DEVC_API_URL) == inMapConnectionInfo.end() ||
      inMapDeviceInfo.find(V_DEVICE_ID) == inMapDeviceInfo.end() ||
      inMapDeviceInfo.find(V_DEVICE_AUTH) == inMapDeviceInfo.end() ||
      inMapDeviceInfo.find(V_DEVICE_NAME) == inMapDeviceInfo.end() ||
      inMapDeviceInfo.find(V_USER_EMAIL_ID) == inMapDeviceInfo.end() ||
      inMapDeviceInfo.find(V_DEVICE_POSTALCODE) == inMapDeviceInfo.end() ||
      inMapDeviceInfo.find(V_DEVICE_ROOMTYPE) == inMapDeviceInfo.end() ) {        

    Serial.println("Required Keys are missing from the DeviceInfo/Connection Map.");

    lstrRetVal = REQUIRED_KEYS_MISSING_IN_MAP; // Required Keys are missing from the DeviceInfo/Connection Map.    
    return lstrRetVal;
  }

  Serial.print("In Registration, Share-Info:");
  // Serial.println(inMapDeviceInfo[DEVC_SHARE_FW_INFO]);
  Serial.println(inMapCalibrationInfo[DEVC_SHARE_FW_INFO]);
  
  if(inMapCalibrationInfo[DEVC_SHARE_FW_INFO]) {

    strPostBody =  String("{") +
          "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\", " +
          "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\", " +
          "\"DeviceName\": \"" + inMapDeviceInfo[V_DEVICE_NAME] + "\", " +
          "\"FrmwrReleaseNum\": \"" + inMapDeviceInfo[FW_VER] + "\", " +
          "\"ownerID\": \"" + inMapDeviceInfo[V_USER_EMAIL_ID] + "\", " +
          "\"installPostalID\": null, " +
          "\"Pincode\": \"" + inMapDeviceInfo[V_DEVICE_POSTALCODE] + "\", " +
          "\"installVicinityID\": \"" + inMapDeviceInfo[V_DEVICE_ROOMTYPE] + "\", " +
          "\"OTAInfo\": {" +
                "\"FrmwrID\": " + inMapDeviceInfo[RES_KEY_OTA_FW_ID] + ", " +
                "\"UpdateKey\": \"" + inMapDeviceInfo[RES_KEY_OTA_UPD_KEY] + "\" }" +
          "}";
  } else {
    strPostBody =  String("{") +
          "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\", " +
          "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\", " +
          "\"DeviceName\": \"" + inMapDeviceInfo[V_DEVICE_NAME] + "\", " +
          "\"FrmwrReleaseNum\": \"" + inMapDeviceInfo[FW_VER] + "\", " +
          "\"ownerID\": \"" + inMapDeviceInfo[V_USER_EMAIL_ID] + "\", " +
          "\"installPostalID\": null, " +
          "\"Pincode\": \"" + inMapDeviceInfo[V_DEVICE_POSTALCODE] + "\", " +
          "\"installVicinityID\": \"" + inMapDeviceInfo[V_DEVICE_ROOMTYPE] + "\" " +
          "}";
  }

    
  HTTPClient http;

  Serial.print("URL: ");
  Serial.print(inMapConnectionInfo[DEVC_API_URL]);
  Serial.println("vdevice/registerDevice/");

  http.begin( inMapConnectionInfo[DEVC_API_URL] + "vdevice/registerDevice/");  //Specify destination for HTTP request
  http.addHeader("Content-Type", "application/json");             //Specify content-type header
  
  Serial.println("Sending Request for Device Registration");
  // To Hide Device Auth.
  // Serial.println(strPostBody);
  
  int httpResponseCode = http.POST(strPostBody);   //Send the actual POST request
  DeserializationError err;
  String lstrMessage = "";

  if(httpResponseCode > 0) {

    if(httpResponseCode == HTTP_CODE_OK) {
      String response = http.getString();  //Get the response to the request
      Serial.println("Response from Server: ");
      Serial.println(response);            //Print request answer

      JsonObject objRegistrationResponse;
      DynamicJsonDocument docResponse(1024);
      err = deserializeJson(docResponse, response);
      if (err) {
          Serial.print("Error while converting Http Response JSON String to JSON Document");
          Serial.println(err.c_str());
      } else {
        objRegistrationResponse = docResponse.as<JsonObject>();
        if(objRegistrationResponse.containsKey(DEVICE_API_RESPONSE_CODE) &&
            objRegistrationResponse[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE)) {

          // Device Registered Successfully. 
          Serial.println("Success");

        } else {

          if( objRegistrationResponse.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
            lstrMessage = objRegistrationResponse[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
          } else {
            lstrMessage = "Should not happen. Http Registration failed.";
          }
          Serial.println(lstrMessage);

          lstrRetVal = objRegistrationResponse.containsKey(DEVICE_API_RESPONSE_CODE) ?
                        objRegistrationResponse[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
        }

        // Even if OTA is never done, we will always obtain IsOtaUpdStatusAccepted with some status.Ideally it should be 'null' if no OTA is done.
        // So, check the OTA Status every time whether its true/false/null as and when OTA is fired.
        if(objRegistrationResponse.containsKey(DEVICE_API_RESPONSE_FLAG_AFTER_FW_UPGRADE)) {

          isOTADone = objRegistrationResponse[DEVICE_API_RESPONSE_FLAG_AFTER_FW_UPGRADE].as<boolean>();
          Serial.println(isOTADone);
          if(isOTADone == true) {

            Serial.println("OTA Completed successfully.Shared Info of new Firmware successfully.Setting ShareFwInfo Flag as false");
            // Set the flag to false
            inMapCalibrationInfo[DEVC_SHARE_FW_INFO] = 0;
            writeCalibrationInfoToFile(inMapCalibrationInfo);
            isOTADone = false;

          } else if(isOTADone == false) {

            Serial.println("OTA Info obtained is null/OTA Completed successfully.But ERROR sending Shared Info of new Firmware to Server.");
          } 

        } else {
          Serial.println("Missing key IsOtaUpdStatusAccepted");
        }

      }
      
    } else {

      lstrRetVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
      Serial.printf("\nHTTP Post failed while Registering Device. Result Not HTTP_CODE_OK");
    }
  } else {
 
    lstrRetVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
    Serial.printf("\nHTTP Post failed while Registering Device. Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  http.end();  //Free resources

  return lstrRetVal;
  
}
