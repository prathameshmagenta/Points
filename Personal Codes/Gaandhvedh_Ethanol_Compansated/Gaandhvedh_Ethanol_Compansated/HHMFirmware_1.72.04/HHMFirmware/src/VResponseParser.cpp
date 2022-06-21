#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <map>
#include "VDefines.h"
#include "VUtils.h"
#include "VSingleAlertSetting.h"
#include "VDeviceModelSingleSensorInfo.h"
#include "VResponseParser.h"
#include "VInformationManager.h"

extern std::map<String, long> mapSensorSanityInterval; // To avoid glitch in a Sensor.

int32_t parseAlertResponse(String &item,
                        String &response,
                        std::map<String, CVSingleAlertSetting>& inMapAlertSettings){

    int32_t retValue = UNKNOWN;

    String param = String(item);
    String httpResponse = String(response);

    String receivedResponseCode = "";
    String objChangedAlertSettingParam = "";
    JsonObject objNewIndividualValue = {};

    DeserializationError err;
    JsonObject objLogAlertResponse;
    DynamicJsonDocument docResponse(1024);      

    err = deserializeJson(docResponse, httpResponse);
    if (err) {
        Serial.print("Error while converting Http Response JSON String to JSON Document");
        Serial.println(err.c_str());
    } else {
        objLogAlertResponse = docResponse.as<JsonObject>(); 

        if (objLogAlertResponse.containsKey(RES_KEY_CODE)) {    

            String code = objLogAlertResponse[DEVICE_API_RESPONSE_CODE].as<String>(); 
            
            if(code.equals(RES_VAL_CODE_SUCCESS)) {
                Serial.println("Send Alert Response: Success");
                retValue = SUCCESS;
            } else if (code.equals(RES_VAL_CODE_FAILED)) {
                Serial.println("Send Alert Response: Device Auth Failed");
                retValue = INVALID_AUTH;
            } else if (code.equals(RES_VAL_CODE_INVALID)) {
                Serial.println("Send Alert Response: Invalid Device ID");
                retValue = INVALID_DEVICE;

            } else if (code.equals(RES_VAL_CODE_SETTING)) {

                Serial.println("Send Alert Response: New Alert Setting");
                retValue = NEW_SETTINGS;
                if( objLogAlertResponse.containsKey(param) == true ) {
                    //Check if new setting is for that particular item
                    objChangedAlertSettingParam = objLogAlertResponse[param].as<String>();
                    DynamicJsonDocument docChangedAlertSettingParam(1024);
                    err = deserializeJson(docChangedAlertSettingParam, objChangedAlertSettingParam);

                    if (err) {
                        Serial.print("Error while converting Changed Param AlertSettingJSON String to JSON Document");
                        Serial.println(err.c_str());
                    } else {          
                        //Convert into a Json object.
                        objNewIndividualValue = docChangedAlertSettingParam.as<JsonObject>();
                    
                        if( objNewIndividualValue.containsKey(ALERT_SETTING_LOW) && 
                            objNewIndividualValue.containsKey(ALERT_SETTING_HIGH) &&
                            objNewIndividualValue.containsKey(ALERT_SETTING_NOTIFY) ) {

                            // Update Alert settings, if all components (high, low, notify) are present.
                            inMapAlertSettings[param] = CVSingleAlertSetting(
                                                                    objNewIndividualValue[ALERT_SETTING_LOW].as<String>(),
                                                                    objNewIndividualValue[ALERT_SETTING_HIGH].as<String>(),
                                                                    objNewIndividualValue[ALERT_SETTING_NOTIFY].as<bool>() );

                            Serial.print("New Settings: ");
                            Serial.print(objNewIndividualValue[ALERT_SETTING_LOW].as<String>()); Serial.print(" ");
                            Serial.print(objNewIndividualValue[ALERT_SETTING_HIGH].as<String>());Serial.print(" ");
                            Serial.println(objNewIndividualValue[ALERT_SETTING_NOTIFY].as<bool>());

                            Serial.println("Now writing into SPIFF");
                            
                            //Write the new alert setting for that particular item into SPIFF
                            writeAlertSettingsToFile(inMapAlertSettings);
                        }
                    }
                }
            } else {
                Serial.print("Send Alert Response: Not Supported, msg:");
                Serial.println(code);
            }
        } else {
            //If Code key is not present, check for Failure Msg key
            if (objLogAlertResponse.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE))  {
                Serial.print("No Code Key Found, Failure Msg: ");
                Serial.println(objLogAlertResponse[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>());
            } else {
                Serial.println("Alert Response: Unknown");
            }
        }
    }
  return retValue;
}


int32_t parseOtaResponse(String &response){

    int32_t retValue = UNKNOWN;

    String httpResponse = String(response);

    DeserializationError err;
    JsonObject objLogAlertResponse;
    DynamicJsonDocument docResponse(1024);      

    err = deserializeJson(docResponse, httpResponse);
    if (err) {
        Serial.print("Error while converting Http Response JSON String to JSON Document");
        Serial.println(err.c_str());
    } else {
        objLogAlertResponse = docResponse.as<JsonObject>(); 

        if (objLogAlertResponse.containsKey(RES_KEY_CODE)) {    

            String code = objLogAlertResponse[DEVICE_API_RESPONSE_CODE].as<String>(); 
            
            if(code.equals(RES_VAL_CODE_SUCCESS)) {
                Serial.println("OTA Clearance: Success");
                retValue = SUCCESS;
            } else if (code.equals(RES_VAL_CODE_FAILED)) {
                Serial.println("OTA Clearance: Device Auth Failed");
                retValue = INVALID_AUTH;
            } else if (code.equals(RES_VAL_CODE_INVALID)) {
                Serial.println("OTA Clearance: Invalid Device ID");
                retValue = INVALID_DEVICE;

            } else if (code.equals(RES_VAL_CODE_FWID_OR_UPKEY)) {
                Serial.println("OTA Clearance: Invalid Fw ID or Update Key");
                retValue = INVALID_FWID;
               
            } else if (code.equals(RES_VAL_CODE_SERVER_ISSUE)) {
                Serial.println("OTA Clearance: Server Issue");
                retValue = SERVER_PROBLEM;

            } else {
                Serial.print("OTA Clearance: Not Supported, msg: ");
                Serial.println(code);
            }
        } else {
            //If Code key is not present, check for Failure Msg key
            if (objLogAlertResponse.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE))  {
                Serial.print("No Code Key Found, Failure Msg: ");
                Serial.println(objLogAlertResponse[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>());
            } else {
                Serial.println("Alert Response: Unknown");
            }
        }
    }
  return retValue;
}
