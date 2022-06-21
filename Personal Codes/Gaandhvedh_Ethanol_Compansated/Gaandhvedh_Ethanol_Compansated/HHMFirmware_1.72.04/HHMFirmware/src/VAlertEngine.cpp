#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <map>
#include "VDefines.h"
#include "VUtils.h"
#include "VAlertEngine.h"
#include "VInformationManager.h"
#include "VCommunication.h"

#include "VResponseParser.h"
#include "VDelay.h"
#include "VCalculation.h"
#include "VSensors.h"


DELAY   objAlertDelay_NO2;
DELAY   objAlertDelay_O3;
DELAY   objAlertDelay_SO2;
DELAY   objAlertDelay_VOC;
DELAY   objAlertDelay_CO;
DELAY   objAlertDelay_NH3;
DELAY   objAlertDelay_CO2;
DELAY   objAlertDelay_HUM;
DELAY   objAlertDelay_PM10;
DELAY   objAlertDelay_PM25;
DELAY   objAlertDelay_TEMP;
DELAY   objAlertDelay_NH3OD;

ALERT_DELAY_COUNTER objAlertCounter;


String retAlertTime;
String strAlertPostBody = "";

// Flag to avaoid mix-up of alerts data with the backlog.
extern bool isAverageSensorValuesCalcualted;


void alert_initAlertSystem () {
  
  sensor_initAlertBuffers();
  
  objAlertDelay_NO2.setDelay(ALERT_DELAY_NO2);
  objAlertDelay_O3.setDelay(ALERT_DELAY_O3);
  objAlertDelay_SO2.setDelay(ALERT_DELAY_SO2);
  objAlertDelay_VOC.setDelay(ALERT_DELAY_VOC);
  objAlertDelay_CO.setDelay(ALERT_DELAY_CO);
  objAlertDelay_NH3.setDelay(ALERT_DELAY_NH3);  
  objAlertDelay_CO2.setDelay(ALERT_DELAY_CO2);  
  objAlertDelay_HUM.setDelay(ALERT_DELAY_HUM);
  objAlertDelay_PM10.setDelay(ALERT_DELAY_PM10);
  objAlertDelay_PM25.setDelay(ALERT_DELAY_PM25);
  objAlertDelay_TEMP.setDelay(ALERT_DELAY_TEMP);
  objAlertDelay_NH3OD.setDelay(ALERT_DELAY_NH3OD);
}

String getParNameBasedOnType (String inParamNameType) {

  if(inParamNameType == VMP_NO2) {
    return "Nitrogen Oxide";
  } else if(inParamNameType == VMP_O3) {
    return "Ozone";
  } else if(inParamNameType == VMP_SO2) {
    return "Sulphur Dioxide";
  } else if(inParamNameType == VMP_VOC) {
    return "VOC";
  } else if(inParamNameType == VMP_CO) {
    return "Carbon Monoxide";
  } else if(inParamNameType == VMP_NH3) {
    return "Ammonium";  
  } else if(inParamNameType == VMP_CO2) {
    return "Carbon Dioxide";  
  } else if(inParamNameType == VMP_HUM) {
    return "Humidity";
  } else if(inParamNameType == VMP_PM10) {
    return "Dust(PM10)";
  } else if(inParamNameType == VMP_PM25) {
    return "Dust(PM2.5)";
  } else if(inParamNameType == VMP_TEMP) {
    return "Temperature";
  } else if(inParamNameType == VMP_NH3OD) {
    return "Odour";
  } else {
    Serial.println("Unable to get ParamName. Unknown Param Type: " + inParamNameType);
    return ""; // Return empty tag
  }
}

String getUnitsBasedOnParamtype( String inParamNameType) {
  if(inParamNameType == VMP_NO2 || inParamNameType == VMP_O3 || inParamNameType == VMP_H2S || inParamNameType == VMP_SO2 || inParamNameType == VMP_VOC) {
    return "PPB";
  } else if(inParamNameType == VMP_CO || inParamNameType == VMP_NH3 ||inParamNameType == VMP_NH3OD || inParamNameType == VMP_CO2 || inParamNameType == VMP_CH4) {
    return "PPM";
  } else if(inParamNameType == VMP_HUM) {
    return "Percent";
  } else if(inParamNameType == VMP_PM10 || inParamNameType == VMP_PM25) {
    return "micro-gm/cubic-mtr";
  } else if(inParamNameType == VMP_TEMP) {
    return "Degree Celcius";
  } else {
    Serial.println("Unable to get Units. Unknown Param Type: " + inParamNameType);
    return ""; // Return empty tag
  }
}


void setDelayBasedOnType (String inParamNameType) {

  if(inParamNameType.equals(VMP_NO2)) {
    objAlertDelay_NO2.setDelay(ALERT_DELAY_NO2);

  } else if(inParamNameType.equals(VMP_O3)) {
    objAlertDelay_O3.setDelay(ALERT_DELAY_O3);

  } else if(inParamNameType.equals(VMP_SO2)) {
    objAlertDelay_SO2.setDelay(ALERT_DELAY_SO2);

  } else if(inParamNameType.equals(VMP_VOC)) {
    objAlertDelay_VOC.setDelay(ALERT_DELAY_VOC);

  } else if(inParamNameType.equals(VMP_CO)) {
    objAlertDelay_CO.setDelay(ALERT_DELAY_CO);

  } else if(inParamNameType.equals(VMP_NH3)) {
    objAlertDelay_NH3.setDelay(ALERT_DELAY_NH3);  

  } else if(inParamNameType.equals(VMP_CO2)) {
    objAlertDelay_CO2.setDelay(ALERT_DELAY_CO2);  

  } else if(inParamNameType.equals(VMP_HUM)) {
    objAlertDelay_HUM.setDelay(ALERT_DELAY_HUM);

  } else if(inParamNameType.equals(VMP_PM10)) {
    objAlertDelay_PM10.setDelay(ALERT_DELAY_PM10);

  } else if(inParamNameType.equals(VMP_PM25)) {
    objAlertDelay_PM25.setDelay(ALERT_DELAY_PM25);

  } else if(inParamNameType.equals(VMP_TEMP)) {
    objAlertDelay_TEMP.setDelay(ALERT_DELAY_TEMP);

  } else if(inParamNameType.equals(VMP_NH3OD)) {
    objAlertDelay_NH3OD.setDelay(ALERT_DELAY_NH3OD);
  } else {
    Serial.print("Unable to set. Unknown Param Type: ");
    Serial.println(inParamNameType);   
  } 
}

bool ifDelayExpired(String inParamNameType) {

  if(inParamNameType.equals(VMP_NO2)) {
    return objAlertDelay_NO2.isTimeOut();

  } else if(inParamNameType.equals(VMP_O3)) {
    return objAlertDelay_O3.isTimeOut();

  } else if(inParamNameType.equals(VMP_SO2)) {
    return objAlertDelay_SO2.isTimeOut();

  } else if(inParamNameType.equals(VMP_VOC)) {
 //   Serial.print("Remaining Timout(S) of VOC: "); 
 //   Serial.println(objAlertDelay_VOC.getRemDelayinMS()/1000); 
    return objAlertDelay_VOC.isTimeOut();

  } else if(inParamNameType.equals(VMP_CO)) {
    return objAlertDelay_CO.isTimeOut();

  } else if(inParamNameType.equals(VMP_NH3)) {
    return objAlertDelay_NH3.isTimeOut();

  } else if(inParamNameType.equals(VMP_CO2)) {
    return objAlertDelay_CO2.isTimeOut();

  } else if(inParamNameType.equals(VMP_HUM)) {
 //   Serial.print("Remaining Timout(S) of HUM: "); 
 //   Serial.println(objAlertDelay_HUM.getRemDelayinMS()/1000); 
    return objAlertDelay_HUM.isTimeOut();

  } else if(inParamNameType.equals(VMP_PM10)) {
    return objAlertDelay_PM10.isTimeOut();

  } else if(inParamNameType.equals(VMP_PM25)) {
    return objAlertDelay_PM25.isTimeOut();

  } else if(inParamNameType.equals(VMP_TEMP)) {
 //   Serial.print("Remaining Timout(S) of TEMP: "); 
 //   Serial.println(objAlertDelay_TEMP.getRemDelayinMS()/1000); 
    return objAlertDelay_TEMP.isTimeOut();

  } else if(inParamNameType.equals(VMP_NH3OD)) {
 //   Serial.print("Remaining Timout(S) of NH3: "); 
 //   Serial.println(objAlertDelay_NH3OD.getRemDelayinMS()/1000);
    return objAlertDelay_NH3OD.isTimeOut();
  } else {
    Serial.println("Unknown Param Type: " + inParamNameType); 
    return 0;
  }  
}


String getAlertSettingsFromServer(std::map<String, String>& inMapConnectionInfo, 
                                  std::map<String, String>& inMapDeviceInfo,
                                  std::map<String, CVSingleAlertSetting>& inMapAlertSettings){

  String lstrRetVal = STATUS_SUCCESS;

  if( inMapDeviceInfo.size() <= 0 || inMapConnectionInfo.size() <= 0 ||
      inMapConnectionInfo.find(DEVC_API_URL) == inMapConnectionInfo.end() ||
      inMapDeviceInfo.find(V_DEVICE_ID) == inMapDeviceInfo.end() ||
      inMapDeviceInfo.find(V_DEVICE_AUTH) == inMapDeviceInfo.end() ) {

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


  http.begin( inMapConnectionInfo[DEVC_API_URL] + "vdevice/getAlertSettings/");  // Specify destination for HTTP request
  http.addHeader("Content-Type", "application/json"); // Specify content-type header

  Serial.println("Sending Reqest for Alert Config");
  // To Hide Device Auth.
  // Serial.println(strPostBody);
  
  int httpResponseCode = http.POST(strPostBody); // Send the actual POST request

  if( httpResponseCode > 0 ) {
    if(httpResponseCode == HTTP_CODE_OK) {

      String response = http.getString(); // Get the response to the request

      Serial.println("Response from Server: ");
      Serial.println(response);

      JsonObject objResponseJson;
      String strAlertSettingsJson;      
      DynamicJsonDocument docResponseJson(2048);
      err = deserializeJson(docResponseJson, response);
      if (err) {
        Serial.print("Error while converting Http Response JSON String to JSON Document");
        Serial.println(err.c_str());
      } else {
        objResponseJson = docResponseJson.as<JsonObject>();  

        if( objResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
            objResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE) &&
            objResponseJson.containsKey("AlertSettings") ) {

          // Get required information from JSON Object
          strAlertSettingsJson = objResponseJson["AlertSettings"].as<String>();

          JsonObject objAlertSettingsJson;
          DynamicJsonDocument doc2(2048);
          err = deserializeJson(doc2, strAlertSettingsJson);
          if (err) {
            Serial.print("Error while converting AlertSettingJSON String to JSON Document. Error: ");
            Serial.println(err.c_str());
          } else {
            objAlertSettingsJson = doc2.as<JsonObject>();
          }
        
          DynamicJsonDocument docSingleAlertSetting(2048);
          JsonObject objJsonSingleAlertSetting;
          
          for (JsonPair p : objAlertSettingsJson) {
            String strMeasuredParam = String( p.key().c_str() );
            String strSingleParamAlertSetting = p.value().as<String>();

            CVSingleAlertSetting lobjSingleAlertSetting;

            if( lobjSingleAlertSetting.initializeObjectFromJsonString(strSingleParamAlertSetting) ) {
              // Fill the alert setting in global map to be used later
              inMapAlertSettings[strMeasuredParam] = lobjSingleAlertSetting;
            } else {
              Serial.println( "Should not happen. Error while converting Json String for Measured Param [" + 
                strMeasuredParam + "] to Alert Settings object." );
              continue; // Skip this alert setting              
            }

          }

        } else {
          if( objResponseJson.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
            lstrMessage = objResponseJson[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
          } else {
            lstrMessage = "Should not happen. Http Get Alert Settings failed.";
          }
          Serial.println(lstrMessage);

          lstrRetVal = objResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) ?
                       objResponseJson[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
        }

      }
  
    } else {
      lstrRetVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
      Serial.printf("\nHTTP Post failed while getting Alert Settings. Result Not HTTP_CODE_OK.");
    }

  } else {
    lstrRetVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;   
    Serial.printf("\nHTTP Post failed while getting Alert Settings. Error: %s", http.errorToString(httpResponseCode).c_str());
  }

  http.end();  // Free resources

  return lstrRetVal;

}


String alert_handleAllActiveAlerts(std::map<String, String>& inMapConnectionInfo,
                         std::map<String, String>& inMapDeviceInfo,
                         std::map<String, CVSingleAlertSetting>& inMapAlertSettings,
                         std::map<String, double>& inMapSensorReadings,
                         String AlertTime,
                         String &retAlertTime,
                         String RTCStat) {

  String lstrResult = NO_ALERT;

  retAlertTime = AlertTime;
  
  String lstrParam;
  bool isAlarmSet;

  float triggerValue = 0.0;
  struct alarm_settings a_config;
  
  int32_t retStatus = -1;
  int32_t retFromParser = -1;

  uint32_t writeIndex = 0;

  // Don't keep it inside for loop as every time during each iteration map will be cleared and we will loose all trigger values of 'previous iteration' too.
  if(isAverageSensorValuesCalcualted) {  
              
    // This function is called because the mapDeviceValues map is used for sending both backlog as well as current data.
    // The data sent after Alerts will have mix of both 'Trigger' and old backlog data in case any backlog is sent just prior to alerts.
    // Hence we are clearing the mapDeviceValues map using the below function and reinserting the recently calculated averaged out values along
    // with the 'Trigger' value too. At start, no average will be present so on reboot just insert the trigger values, otherwise if there is only 'key' and no 'value', then the UI will show as value '0' which is wrong.
    sensor_reinsertAveragedOutValues(inMapSensorReadings);

  } else if(!isAverageSensorValuesCalcualted) {

    // Since its first Alert after reboot, we need to clear the mapDeviceValues map and insert only the 'Trigger values' as the 
    // mapDeviceValues map may be containing old backlog data and also Average has not been calculated yet.
    inMapSensorReadings.clear();
  }

  //Create an iterator for Map-AlertSettings
  std::map<String, CVSingleAlertSetting>::iterator row;

  //Check all Alerts one by one
  for(row=inMapAlertSettings.begin(); row!=inMapAlertSettings.end(); row++){

    triggerValue = 0.0;
    lstrParam = row->first;
    isAlarmSet = row->second.canNotify();

    if(!isAlarmSet){
      //If Alert-Notify is not set, move to next Alert
      continue;
    }

    a_config.lowCutoff = row->second.getLow();
    a_config.highCutoff = row->second.getHigh();

    a_config.isLowNull = row->second.isLowNull();
    a_config.isHighNull = row->second.isHighNull();

    String lstrLowerSetting = (a_config.isLowNull) ? "null" : String(a_config.lowCutoff, 2); 
    String lstrHigherSetting = (a_config.isHighNull) ? "null" : String(a_config.highCutoff, 2); 
    
    String lstrAlertMsg;

    //If Time-out period of that item is expired
    if(ifDelayExpired(lstrParam)){
      //Print the Parameter, including settings
      Serial.println();
      Serial.println(lstrParam+":"+isAlarmSet+", "+a_config.lowCutoff +":"+a_config.isLowNull +", " +
                                                  a_config.highCutoff+":" + a_config.isHighNull);

      //Set the Delay again for that particular item
      setDelayBasedOnType(lstrParam);
    
      //Get both Write Index and Read Index
      writeIndex = sensor_getWriteIndexBasedOnType(lstrParam);

      try{
        //Read the current data of that particular item
        retStatus = calculation_detectValidTriggerPoint(sensor_getBufferBasedOnType(lstrParam),
                                                        ALERT_BUFFER_SIZE,
                                                        writeIndex, sensor_getReadIndexBasedOnType(lstrParam), 
                                                        &a_config, triggerValue);
      }
      catch(...){
       Serial.println("calculation_detectValidTriggerPoint: Error occured"); 
      }
      
      Serial.print("Ret Stat: "); 
      Serial.println(retStatus);
      Serial.print("Trigger Val: "); 
      Serial.println(triggerValue); 


      //Convert the trigger point value into string                                                
      String lstrParamValue = String(triggerValue);
      
      if(retStatus == VALID_POSITIVE_TRIGGER){
        //If it hits the higher cut-off
        lstrAlertMsg = getParNameBasedOnType(lstrParam) + " value " + lstrParamValue + " has gone above " + lstrHigherSetting + 
                            " " +getUnitsBasedOnParamtype(lstrParam) + ".";;

      } else if(retStatus == VALID_NEGATIVE_TRIGGER){
        //If it hits lower cut-off
        lstrAlertMsg = getParNameBasedOnType(lstrParam) + " value " + lstrParamValue + " has gone below " + lstrLowerSetting + 
                            " " + getUnitsBasedOnParamtype(lstrParam) + ".";

      }

      String relURI = "vdevice/sendAlertMessage/";
      String httpResposne;

      if(RTCStat == RTC_DATE_TIME_INVALID) {

        strAlertPostBody =  String("{") +
                                  "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\", " +
                                  "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\", " +
                                  "\"MeasuredParam\": \"" + lstrParam + "\", " +
                                  "\"Value\": \"" + lstrParamValue + "\", " +
                                  "\"MinLimitVal\": \""+ lstrLowerSetting + "\"," +
                                  "\"MaxLimitVal\": \""+ lstrHigherSetting + "\"," +
                                  "\"AlertMessage\": \"" + lstrAlertMsg + "\", " +
                                  "\"AlertTime\": null " +
                                  "}";
      } else {

        strAlertPostBody =  String("{") +
                                  "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\", " +
                                  "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\", " +
                                  "\"MeasuredParam\": \"" + lstrParam + "\", " +
                                  "\"Value\": \"" + lstrParamValue + "\", " +
                                  "\"MinLimitVal\": \""+ lstrLowerSetting + "\"," +
                                  "\"MaxLimitVal\": \""+ lstrHigherSetting + "\"," +
                                  "\"AlertMessage\": \"" + lstrAlertMsg + "\", " +
                                  "\"AlertTime\": \"" + AlertTime + "\" " +
                                  "}"; 

      }

      if((retStatus == VALID_POSITIVE_TRIGGER) ||
          (retStatus == VALID_NEGATIVE_TRIGGER)) {
        
        // The data sent after alert will have the same value as that of alert triggered for that lstrparam.
        inMapSensorReadings[lstrParam] = triggerValue;    

        // Once the alert is triggered immediately send the next alert after 15 min or 30 sec
        // according to the parameter. 
        objAlertCounter.setCounterforTrigger(lstrParam);

        Serial.println("Sending Alert to Server");
       
        if(comm_communicateToServer(inMapConnectionInfo, inMapDeviceInfo, relURI, strAlertPostBody, httpResposne)){
          
          Serial.println("Server Response");       
          retFromParser = parseAlertResponse(lstrParam, httpResposne, inMapAlertSettings);

          if(retFromParser != UNKNOWN){
            lstrResult = STATUS_SUCCESS;
          }
          
        } else {
          lstrResult = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
        }
        
      } else {
        Serial.println("Alert Engine: No Valid Trigger Point");

        // If there is no trigger then the counter will be zero, and the alert check will continue every 5 sec 
        // or 10 sec according to the parameter.This is not so useful here, but suppose we want the second alert timing to be 
        // diffrent eg.SMOKE or LPG(2nd alert after 30 sec) then this is very useful.
        objAlertCounter.ResetCounterforNoTrigger(lstrParam);
      }
    } else {
//      Serial.print(lstrParam);
//      Serial.println(" Alarm Delay not expired.");
    }
  }
  return lstrResult;
}