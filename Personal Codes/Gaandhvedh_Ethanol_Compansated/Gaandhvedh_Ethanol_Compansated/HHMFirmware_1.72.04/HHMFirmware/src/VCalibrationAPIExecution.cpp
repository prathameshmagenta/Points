#include <map>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "VCalibrationAPIExecution.h"
#include "VCalibrationSensorCalculation.h"
#include "VInformationManager.h"
#include "VDefines.h"
#include "VUtils.h"

uint8_t Statinvokefun = 0;

String strCalibMeasuredParam = "";
float Step1InputVal = 0.0f;

String RTCUnixTimeStamp = "";
String UTCDateTime = "";
String CurrentUTCTimeStamp = "";

float Step2InputVal = 0.0f;
float Step3InputVal = 0.0f;
String StepNumberInfo = "";
String Step1Status = "";
String Step2Status = "";
String Step3Status = "";
float PostCalParamOutput = 0.0f;
String strAckRes = "";

String strDevcAckPostBody = "";
String strOTADevcAckPostBody = "";
extern String strDevcinfo;
extern bool NewAlertSet;

// Flag to check whether Step ONE is invoked again after fully calibrating a device.
bool invokeStepOneAgain = false;

extern std::map<String, String> mapDeviceInfo;                                  
extern std::map<String, String> mapConnectionInfo;
extern std::map<String, uint32_t> mapCalibrationInfo;


String sendAcknowledgementForCalibration(std::map<String, String>& inMapConnectionInfo,
                                        std::map<String, String>& inMapDeviceInfo) {

    String lstrAckRetVal = STATUS_SUCCESS;

    if( inMapDeviceInfo.size() <= 0 || inMapConnectionInfo.size() <= 0 ||
        inMapConnectionInfo.find(DEVC_API_URL) == inMapConnectionInfo.end() ||
        inMapDeviceInfo.find(V_DEVICE_ID) == inMapDeviceInfo.end() ||
        inMapDeviceInfo.find(V_DEVICE_AUTH) == inMapDeviceInfo.end() ) {

        Serial.println("Required Keys are missing from the DeviceInfo/Connection Map.");

        lstrAckRetVal = REQUIRED_KEYS_MISSING_IN_MAP; // Required Keys are missing from the DeviceInfo/Connection Map.
        return lstrAckRetVal;
    }

    // Request for sending Acknowledgement.
    HTTPClient http;

    String lstrAckMessage = "";
    DeserializationError err;

    String strAckPostBody =  String("{") +
                            "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                            "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                            "\"DevcCalibStatus\": \"DevcReadyForCalib\""
                            "}";

    http.begin( inMapConnectionInfo[DEVC_API_URL] + "vdevice/acknowledgeReadyForCalibration/");  // Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json"); // Specify content-type header

    // To Hide Device Auth.
    Serial.println("Sending Request for Acknowledgement");
    // Serial.println(strAckPostBody);
    
    int httpResponseCode = http.POST(strAckPostBody); // Send the actual POST request

    if( httpResponseCode > 0 ) {
        if(httpResponseCode == HTTP_CODE_OK) {

            String Ackresponse = http.getString(); // Get the response to the request

            Serial.println("Response from Server: ");
            Serial.println(Ackresponse);

            JsonObject objAckResponseJson;     
            DynamicJsonDocument docResponseJson(1024);
            err = deserializeJson(docResponseJson, Ackresponse);
            if (err) {
                Serial.print("Error while converting Http Response JSON String to JSON Document");
                Serial.println(err.c_str());
            } else {
                objAckResponseJson = docResponseJson.as<JsonObject>();  

                if( objAckResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objAckResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE)) {

                    // Acknowledgement received successfully. 
                    Serial.println("Success");

                } else if( objAckResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objAckResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVC_GET_OUT_OF_CALIB_MODE) ) {

                    // An Indication that the user wants to come out of the Calibration Mode.
                    Serial.println("User has invoked to come out of calibration mode.Firing Ack API and executing MoveDeviceOutOfCalibrationMode function");
                    strAckRes = SendAckForDeviceOutOfCalibMode();
                    if(strAckRes != STATUS_SUCCESS) {
                        // If 'SUCCESS' is not received, print the error message.
                        Serial.println("Could not receive 'SUCCESS' for Acknowledgement for bringing device out of Calibration Mode.");
                        Serial.println(strAckRes);
                        
                    } else {
                        
                        Serial.println("Received 'SUCCESS' for Acknowledgement for bringing device out of Calibration Mode.");
                    }
                    // Bring device out of the calibration mode irrespective of the status.
                    MoveDeviceOutOfCalibrationMode();

                } else {

                    if( objAckResponseJson.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
                        lstrAckMessage = objAckResponseJson[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
                    } else {
                        lstrAckMessage = "Should not happen. Http SendAcknowledgement failed.";
                    }
                    Serial.println(lstrAckMessage);

                    lstrAckRetVal = objAckResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) ?
                                    objAckResponseJson[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
                }

            }
        
        } else {

            lstrAckRetVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
            Serial.printf("\nHTTP Post failed while Sending the Acknowledgement. Result Not HTTP_CODE_OK");
        }
    } else {
 
        lstrAckRetVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
        Serial.printf("\nHTTP Post failed while Sending the Acknowledgement. Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();  //Free resources

    return lstrAckRetVal;

}

String getCalibrationStepInfo(std::map<String, String>& inMapConnectionInfo,
                            std::map<String, String>& inMapDeviceInfo, 
                            uint8_t &Statinvokefun,
                            String &strCalibMeasuredParam,
                            float &Step1InputVal,
                            float &Step2InputVal,
                            float &Step3InputVal,
                            String &StepNumberInfo,
                            String &Step1Status,
                            String &Step2Status,
                            String &Step3Status,
                            float &PostCalParamOutput) {

    String lstrCalibStepRetVal = STATUS_SUCCESS;
    

    if( inMapDeviceInfo.size() <= 0 || inMapConnectionInfo.size() <= 0 ||
        inMapConnectionInfo.find(DEVC_API_URL) == inMapConnectionInfo.end() ||
        inMapDeviceInfo.find(V_DEVICE_ID) == inMapDeviceInfo.end() ||
        inMapDeviceInfo.find(V_DEVICE_AUTH) == inMapDeviceInfo.end() ) {

        Serial.println("Required Keys are missing from the DeviceInfo/Connection Map.");

        lstrCalibStepRetVal = REQUIRED_KEYS_MISSING_IN_MAP; // Required Keys are missing from the DeviceInfo/Connection Map.
        return lstrCalibStepRetVal;
    }

    // Request for sending Calibration Step Info.
    HTTPClient http;

    String lstrCalibStepMessage = "";
    String strCalibValuesJson = "";
    String strCalibStatusJson = "";
    DeserializationError err;

    String strCalibStepPostBody =  String("{") +
                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\""
                                "}";

    http.begin( inMapConnectionInfo[DEVC_API_URL] + "vdevice/getCurrentCalibStepInfo/");  // Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json"); // Specify content-type header

    // To Hide Device Auth.
    Serial.println("Sending Request for Current Calib Step");
    // Serial.println(strCalibStepPostBody);
    
    int httpResponseCode = http.POST(strCalibStepPostBody); // Send the actual POST request

    if( httpResponseCode > 0 ) {
        if(httpResponseCode == HTTP_CODE_OK) {

            String StepInforesponse = http.getString(); // Get the response to the request

            Serial.println("Response from Server: ");
            Serial.println(StepInforesponse);

            JsonObject objCalibStepResponseJson;     
            DynamicJsonDocument docResponseJson(1024);
            err = deserializeJson(docResponseJson, StepInforesponse);
            if (err) {
                Serial.print("Error while converting Http Response JSON String to JSON Document");
                Serial.println(err.c_str());
            } else {
                objCalibStepResponseJson = docResponseJson.as<JsonObject>();  

                if( objCalibStepResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objCalibStepResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE)) {
                    // getCurrentCalibStepInfo received successfully. 
                    Serial.println("Success");

                } else if( objCalibStepResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objCalibStepResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVC_GET_OUT_OF_CALIB_MODE) ) {

                    // An Indication that the user wants to come out of the Calibration Mode.
                    Serial.println("User has invoked to come out of calibration mode.Firing Ack API and executing MoveDeviceOutOfCalibrationMode function");
                    strAckRes = SendAckForDeviceOutOfCalibMode();
                    if(strAckRes != STATUS_SUCCESS) {
                        // If 'SUCCESS' is not received, print the error message.
                        Serial.println("Could not receive 'SUCCESS' for Acknowledgement for bringing device out of Calibration Mode.");
                        Serial.println(strAckRes);
                        
                    } else {
                        
                        Serial.println("Received 'SUCCESS' for Acknowledgement for bringing device out of Calibration Mode.");
                    }
                    // Bring device out of the calibration mode irrespective of the status.
                    MoveDeviceOutOfCalibrationMode();

                } else if( objCalibStepResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objCalibStepResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DVC_PARAM_NOT_FOUND_FOR_CALIB)) {

                    Serial.println("User has put the device in Calibration Mode but hasn't invoked anything yet");
                    // Ping 8 sec indication that invokegetCalibStepInfoAgain flag should be made true.
                    Statinvokefun = 1;
                    lstrCalibStepRetVal = DVC_PARAM_NOT_FOUND_FOR_CALIB;

                } else {

                    if( objCalibStepResponseJson.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
                        lstrCalibStepMessage = objCalibStepResponseJson[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
                    } else {
                        lstrCalibStepMessage = "Should not happen. Http getCurrentCalibStepInfo failed.";
                    }
                    Serial.println(lstrCalibStepMessage);
                    // Ping 8 sec indication that invokegetCalibStepInfoAgain flag should be made true, so that for any reason sensor calculations shouldn't begin.
                    Statinvokefun = 1;

                    lstrCalibStepRetVal = objCalibStepResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) ?
                                            objCalibStepResponseJson[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
                }

                if(objCalibStepResponseJson.containsKey(DEVICE_API_MEASURED_PARAM)) {
                    // Extract the measured Param from the API.
                    strCalibMeasuredParam = objCalibStepResponseJson[DEVICE_API_MEASURED_PARAM].as<String>();
                    Serial.printf("The Measured Param extracted is: %s\r\n", strCalibMeasuredParam.c_str());

                    // Extract the Key and Values present inside the Key:CalibValues
                    strCalibValuesJson = objCalibStepResponseJson[DEVICE_API_CALIB_VALUES].as<String>();
                    JsonObject objCalibValuesJson;
                    DynamicJsonDocument docCalibVal(1024);
                    err = deserializeJson(docCalibVal, strCalibValuesJson);

                    if (err) {
                        Serial.print("Error while converting Calib Values Arduino String to JSON Document");
                        Serial.println(err.c_str());
                    } else {          
                        //Convert into a Json object.
                        objCalibValuesJson = docCalibVal.as<JsonObject>();
                    }

                    // Extract the Key and Values present inside the Key:DevcCalibStatus
                    strCalibStatusJson = objCalibStepResponseJson[DEVICE_API_CALIB_STATUS].as<String>();
                    JsonObject objCalibStatusJson;
                    DynamicJsonDocument docCalibStatus(1024);
                    err = deserializeJson(docCalibStatus, strCalibStatusJson);

                    if (err) {
                        Serial.print("Error while converting Calib Status Arduino String to JSON Document");
                        Serial.println(err.c_str());
                    } else {          
                        //Convert into a Json object.
                        objCalibStatusJson = docCalibStatus.as<JsonObject>();
                    }

                    // All the keys values have been converted to JSON Object.Now, check if User has really clicked any button.
                    if((strCalibMeasuredParam.length() <= 0) && 
                        !(objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP1INP)) &&
                        !(objCalibStatusJson.containsKey(DEVICE_API_CALIB_STATUS_STEP1))) {

                        // Ping 8 sec indication that invokegetCalibStepInfoAgain flag should be made true.
                        Statinvokefun = 1;
                        Serial.println("Measured Param is null.Step1Inp and Step1 Stat are empty.Meaning User has not entered anything yet.");
                    
                    } else if((strCalibMeasuredParam.length() != 0) &&
                            (objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP1INP)) &&
                            (objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP2INP)) &&
                            (objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP3INP)) &&
                            (objCalibStatusJson.containsKey(DEVICE_API_CALIB_STATUS_STEP1)) &&
                            (objCalibStatusJson.containsKey(DEVICE_API_CALIB_STATUS_STEP2)) &&
                            (objCalibStatusJson.containsKey(DEVICE_API_CALIB_STATUS_STEP3))) {
                        
                        // This indicates that All steps were completed initially, but the User wants to recalibrate again.So, he won't get
                        // The Steps and Inputs as EMPTY.That is only for a fresh PCB Board.Hence on recalibration, he will get All steps info, but
                        // The function needs to be invoked every 8 sec to get the User Input ONE.
                        // Ping 8 sec indication that invokegetCalibStepInfoAgain flag should be made true.
                        Statinvokefun = 1;
                        // These information will be useful for timeouts after a fresh calibration is invoked 2nd time but no user input is provided.So the step value will stay as step THREE.
                        Step1InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP1INP].as<float>();
                        Step2InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP2INP].as<float>();
                        Step3InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP3INP].as<float>();
                        StepNumberInfo = DEVICE_API_CALIB_STATUS_STEP3;
                        Step1Status = objCalibStatusJson[DEVICE_API_CALIB_STATUS_STEP1].as<String>();
                        Step2Status = objCalibStatusJson[DEVICE_API_CALIB_STATUS_STEP2].as<String>();
                        Step3Status = objCalibStatusJson[DEVICE_API_CALIB_STATUS_STEP3].as<String>();
                        PostCalParamOutput = objCalibValuesJson[DEVICE_API_CALIB_POSTCALOUTPUT].as<float>();
                        Serial.println("Measured Param is Present.Old Step1, Step2, Step3 Input and Stat is present.Meaning User has not entered anything for recalibrating from Step ONE");

                    } else if((strCalibMeasuredParam.length() != 0) &&
                            (objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP1INP)) &&
                            (objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP2INP)) &&
                            (objCalibStatusJson.containsKey(DEVICE_API_CALIB_STATUS_STEP1)) &&
                            (objCalibStatusJson.containsKey(DEVICE_API_CALIB_STATUS_STEP2))) {

                        if((objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP3INP))) {

                            // Extract the Step3Inp Value and store it in a variable.Also extract the StepStatus of previous step which can be used for the this step.
                            // Pass it in sensor function, also give and indication that invokegetCalibStepInfoAgain flag need not be true.
                            Statinvokefun = 0;
                            Step1InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP1INP].as<float>();
                            Step2InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP2INP].as<float>();
                            Step3InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP3INP].as<float>();
                            StepNumberInfo = DEVICE_API_CALIB_STATUS_STEP3;
                            Step1Status = objCalibStatusJson[DEVICE_API_CALIB_STATUS_STEP1].as<String>();
                            Step2Status = objCalibStatusJson[DEVICE_API_CALIB_STATUS_STEP2].as<String>();
                            Serial.printf("Step THREE is invoked by the User.The Value sent from APP is: %0.2f\r\n", Step3InputVal);

                        } else if(!(objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP3INP))) {

                            // Ping 8 sec indication that invokegetCalibStepInfoAgain flag should be made true.
                            Statinvokefun = 1;
                            // These information will be useful for timeouts after a fresh calibration is invoked 2nd time but no user input is provided.So the step value will stay as step TWO.
                            Step1InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP1INP].as<float>();
                            Step2InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP2INP].as<float>();
                            StepNumberInfo = DEVICE_API_CALIB_STATUS_STEP2;
                            Step1Status = objCalibStatusJson[DEVICE_API_CALIB_STATUS_STEP1].as<String>();
                            Step2Status = objCalibStatusJson[DEVICE_API_CALIB_STATUS_STEP2].as<String>();
                            PostCalParamOutput = objCalibValuesJson[DEVICE_API_CALIB_POSTCALOUTPUT].as<float>();
                            Serial.println("Measured Param is Present.Step3Inp absent and Old Step1 and Step2 Stat and Inputs are present.Meaning User has not entered anything for Third Step.");

                        } else {
                            Serial.println("Please check Calibration for Step TWO.");
                        }

                    } else if((strCalibMeasuredParam.length() != 0) && 
                            (objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP2INP)) &&
                            (objCalibStatusJson.containsKey(DEVICE_API_CALIB_STATUS_STEP2)) &&
                            !(objCalibStatusJson.containsKey(DEVICE_API_CALIB_STATUS_STEP1))) {

                        if((objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP1INP))) {

                            // This condition is used when there is recalibration of Step ONE with the Step2 intact as it is.User just wants to recalibrate Step ONE again for some reason.
                            // Extract the Step1Inp Value and store it in a variable.
                            // Pass it in sensor function, also give and indication that invokegetCalibStepInfoAgain flag need not be true.
                            Statinvokefun = 0;
                            Step1InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP1INP].as<float>();
                            StepNumberInfo = DEVICE_API_CALIB_STATUS_STEP1;
                            Step2InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP2INP].as<float>();
                            Step2Status = objCalibStatusJson[DEVICE_API_CALIB_STATUS_STEP2].as<String>();
                            // Making a flag ie. invokeStepOneAgain true and after completing step remember to make it false.
                            invokeStepOneAgain = true;
                            Serial.printf("Step ONE(REDO) is invoked by the User again keeping Step TWO intact.Making invokeStepOneAgain flag true.The Value sent from APP is: %0.2f\r\n", Step1InputVal);

                        } else {

                            Statinvokefun = 1;
                            Serial.println("Step TWO Stat and Inputs are present with no information of Step ONE.This is a future step, please check.");
                        }

                    } else if((strCalibMeasuredParam.length() != 0) &&
                            (objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP1INP)) &&
                            (objCalibStatusJson.containsKey(DEVICE_API_CALIB_STATUS_STEP1))) {
                        

                        if((objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP2INP))) {

                            // Extract the Step2Inp and Step1Inp Value and store it in a variable.Also extract the StepStatus of previous step which can be used for the this step.
                            // Pass it in sensor function, also give and indication that invokegetCalibStepInfoAgain flag need not be true.
                            Statinvokefun = 0;
                            Step1InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP1INP].as<float>();
                            Step2InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP2INP].as<float>();
                            StepNumberInfo = DEVICE_API_CALIB_STATUS_STEP2;
                            Step1Status = objCalibStatusJson[DEVICE_API_CALIB_STATUS_STEP1].as<String>();
                            Serial.printf("Step TWO is invoked by the User.The Value sent from APP is: %0.2f\r\n", Step2InputVal);

                        } else if(!(objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP2INP))) {

                            // Ping 8 sec indication that invokegetCalibStepInfoAgain flag should be made true.
                            Statinvokefun = 1;
                            // These information will be useful for timeouts after a fresh calibration is invoked 2nd time but no user input is provided.So the step value will stay as step ONE.
                            Step1InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP1INP].as<float>();
                            StepNumberInfo = DEVICE_API_CALIB_STATUS_STEP1;
                            Step1Status = objCalibStatusJson[DEVICE_API_CALIB_STATUS_STEP1].as<String>();
                            PostCalParamOutput = objCalibValuesJson[DEVICE_API_CALIB_POSTCALOUTPUT].as<float>();
                            Serial.println("Measured Param is Present.Step2Inp absent and Old Step1 Stat and Input is present.Meaning User has not entered anything for Second Step.");

                        } else {
                            Serial.println("Please check Calibration for Step ONE.");
                        }

                    } else if(strCalibMeasuredParam.equals(VMP_RTC)) {

                        if(!(objCalibStatusJson.containsKey(DEVICE_API_CALIB_STATUS_STEP1))) {
                            
                            // Pass it in sensor function, also give and indication that invokegetCalibStepInfoAgain flag need not be true.                        
                            Statinvokefun = 0;
                            RTCUnixTimeStamp = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP1INP].as<String>();
                            UTCDateTime = objCalibValuesJson[DEVICE_API_SERVER_UTC_DT].as<String>();
                            Serial.printf("Step ONE for RTC is invoked by the User.");

                        } else {

                            Statinvokefun = 1;
                            // To do extract if timeout happens next day or after reboot
                            Step1Status = objCalibStatusJson[DEVICE_API_CALIB_STATUS_STEP1].as<String>();
                            RTCUnixTimeStamp = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP1INP].as<String>();
                            UTCDateTime = objCalibValuesJson[DEVICE_API_SERVER_UTC_DT].as<String>();
                            Serial.println("RTC has been set before.User has not invoked again for fresh setting.");
                        }

                    } else if((strCalibMeasuredParam.length() != 0) &&
                            (objCalibValuesJson.containsKey(DEVICE_API_CALIB_VAL_STEP1INP)) &&
                            !(objCalibStatusJson.containsKey(DEVICE_API_CALIB_STATUS_STEP1))) {

                        // Extract the Step1Inp Value and store it in a variable.
                        // Pass it in sensor function, also give and indication that invokegetCalibStepInfoAgain flag need not be true.
                        Statinvokefun = 0;
                        Step1InputVal = objCalibValuesJson[DEVICE_API_CALIB_VAL_STEP1INP].as<float>();
                        StepNumberInfo = DEVICE_API_CALIB_STATUS_STEP1;
                        Serial.printf("Step ONE is invoked by the User.The Value sent from APP is: %0.2f\r\n", Step1InputVal);

                    } else {

                        Serial.println("Unknown Keys acquired from the getCurrentCalibStepInfo API.");
                        // Ping 8 sec indication that invokegetCalibStepInfoAgain flag should be made true, so that for any reason sensor calculations shouldn't begin.
                        Statinvokefun = 1;
                    } 

                }

            }
        
        } else {

            lstrCalibStepRetVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
            Serial.printf("\nHTTP Post failed while calling getCurrentCalibStepInfo. Result Not HTTP_CODE_OK");
            // Ping 8 sec indication that invokegetCalibStepInfoAgain flag should be made true, so that for any reason sensor calculations shouldn't begin.
            Statinvokefun = 1;
        }

    } else {
 
        lstrCalibStepRetVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
        Serial.printf("\nHTTP Post failed while calling getCurrentCalibStepInfo. Error: %s\n", http.errorToString(httpResponseCode).c_str());
        // Ping 8 sec indication that invokegetCalibStepInfoAgain flag should be made true, so that for any reason sensor calculations shouldn't begin.
        Statinvokefun = 1;
    }

    http.end();  //Free resources

    return lstrCalibStepRetVal;

}

String SaveCalibrationStepResult(std::map<String, String>& inMapConnectionInfo,
                                std::map<String, String>& inMapDeviceInfo,
                                String strResPostBody) {

    String lstrStepResVal = STATUS_SUCCESS;

    if( inMapDeviceInfo.size() <= 0 || inMapConnectionInfo.size() <= 0 ||
        inMapConnectionInfo.find(DEVC_API_URL) == inMapConnectionInfo.end() ||
        inMapDeviceInfo.find(V_DEVICE_ID) == inMapDeviceInfo.end() ||
        inMapDeviceInfo.find(V_DEVICE_AUTH) == inMapDeviceInfo.end() ) {

        Serial.println("Required Keys are missing from the DeviceInfo/Connection Map.");

        lstrStepResVal = REQUIRED_KEYS_MISSING_IN_MAP; // Required Keys are missing from the DeviceInfo/Connection Map.
        return lstrStepResVal;
    }

    HTTPClient http;

    String lstrStepResMessage = "";
    DeserializationError err;

    http.begin( inMapConnectionInfo[DEVC_API_URL] + "vdevice/saveCurrentCalibStepResult/");  // Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json"); // Specify content-type header

    // To Hide Device Auth.
    Serial.println("Sending Request for saveCurrentCalibStepResult");
    // Serial.println(strResPostBody);
    
    int httpResponseCode = http.POST(strResPostBody); // Send the actual POST request

    if( httpResponseCode > 0 ) {
        if(httpResponseCode == HTTP_CODE_OK) {

            String Stepresponse = http.getString(); // Get the response to the request

            Serial.println("Response from Server: ");
            Serial.println(Stepresponse);

            JsonObject objCalibResponseJson;     
            DynamicJsonDocument docResponseJson(1024);
            err = deserializeJson(docResponseJson, Stepresponse);
            if (err) {
                Serial.print("Error while converting Http Response JSON String to JSON Document");
                Serial.println(err.c_str());
            } else {
                objCalibResponseJson = docResponseJson.as<JsonObject>();  

                if( objCalibResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objCalibResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE)) {

                    // Save Calib Step received successfully. 
                    Serial.println("Success");

                } else if( objCalibResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objCalibResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVC_GET_OUT_OF_CALIB_MODE) ) {

                    // An Indication that the user wants to come out of the Calibration Mode.
                    Serial.println("User has invoked to come out of calibration mode.Firing Ack API and executing MoveDeviceOutOfCalibrationMode function");
                    strAckRes = SendAckForDeviceOutOfCalibMode();
                    if(strAckRes != STATUS_SUCCESS) {
                        // If 'SUCCESS' is not received, print the error message.
                        Serial.println("Could not receive 'SUCCESS' for Acknowledgement for bringing device out of Calibration Mode.");
                        Serial.println(strAckRes);
                        
                    } else {
                        
                        Serial.println("Received 'SUCCESS' for Acknowledgement for bringing device out of Calibration Mode.");
                    }
                    // Bring device out of the calibration mode irrespective of the status.
                    MoveDeviceOutOfCalibrationMode();

                } else {

                    if( objCalibResponseJson.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
                        lstrStepResMessage = objCalibResponseJson[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
                    } else {
                        lstrStepResMessage = "Should not happen. Http saveCurrentCalibStepResult failed.";
                    }
                    Serial.println(lstrStepResMessage);

                    lstrStepResVal = objCalibResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) ?
                                    objCalibResponseJson[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
                }

            }
        
        } else {

            lstrStepResVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
            Serial.printf("\nHTTP Post failed while Sending the saveCurrentCalibStepResult. Result Not HTTP_CODE_OK");
        }
    } else {
 
        lstrStepResVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
        Serial.printf("\nHTTP Post failed while Sending the saveCurrentCalibStepResult. Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();  //Free resources

    return lstrStepResVal;
}

// Remember in sensor calculations, if the Step is more than ONE, then before starting calculations make invokegetCalibStepInfoAgain false so that there won't be unecessary pings
// And if you want the second/third step then make invokegetCalibStepInfoAgain Flag true if you want the User input Again.
// This unnecessary pings will for gas entities where time will be consumed.Otherwise no need.
// Gas I am blocking APIs anyways so need to make flag false again.


String SendSaveCalibErrorToServer(std::map<String, String>& inMapConnectionInfo,
                                std::map<String, String>& inMapDeviceInfo,
                                String strResPostErrorBody) {

    String lstrDevcErrorVal = STATUS_SUCCESS;

    if( inMapDeviceInfo.size() <= 0 || inMapConnectionInfo.size() <= 0 ||
        inMapConnectionInfo.find(DEVC_API_URL) == inMapConnectionInfo.end() ||
        inMapDeviceInfo.find(V_DEVICE_ID) == inMapDeviceInfo.end() ||
        inMapDeviceInfo.find(V_DEVICE_AUTH) == inMapDeviceInfo.end() ) {

        Serial.println("Required Keys are missing from the DeviceInfo/Connection Map.");

        lstrDevcErrorVal = REQUIRED_KEYS_MISSING_IN_MAP; // Required Keys are missing from the DeviceInfo/Connection Map.
        return lstrDevcErrorVal;
    }

    HTTPClient http;

    String lstrDevcErrorMessage = "";
    DeserializationError err;

    http.begin( inMapConnectionInfo[DEVC_API_URL] + "vdevice/saveDeviceCalibError/");  // Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json"); // Specify content-type header

    // To Hide Device Auth.
    Serial.println("Sending Request for saveDeviceCalibError");
    // Serial.println(strResPostErrorBody);
    
    int httpResponseCode = http.POST(strResPostErrorBody); // Send the actual POST request

    if( httpResponseCode > 0 ) {
        if(httpResponseCode == HTTP_CODE_OK) {

            String CalibErrorresponse = http.getString(); // Get the response to the request

            Serial.println("Response from Server: ");
            Serial.println(CalibErrorresponse);

            JsonObject objCalibErrorResponseJson;     
            DynamicJsonDocument docResponseJson(1024);
            err = deserializeJson(docResponseJson, CalibErrorresponse);
            if (err) {
                Serial.print("Error while converting Http Response JSON String to JSON Document");
                Serial.println(err.c_str());
            } else {
                objCalibErrorResponseJson = docResponseJson.as<JsonObject>();  

                if( objCalibErrorResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objCalibErrorResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE)) {

                    // Calib Error received successfully. 
                    Serial.println("Success");

                } else if( objCalibErrorResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objCalibErrorResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVC_GET_OUT_OF_CALIB_MODE) ) {

                    // An Indication that the user wants to come out of the Calibration Mode.
                    Serial.println("User has invoked to come out of calibration mode.Firing Ack API and executing MoveDeviceOutOfCalibrationMode function");
                    strAckRes = SendAckForDeviceOutOfCalibMode();
                    if(strAckRes != STATUS_SUCCESS) {
                        // If 'SUCCESS' is not received, print the error message.
                        Serial.println("Could not receive 'SUCCESS' for Acknowledgement for bringing device out of Calibration Mode.");
                        Serial.println(strAckRes);
                        
                    } else {
                        
                        Serial.println("Received 'SUCCESS' for Acknowledgement for bringing device out of Calibration Mode.");
                    }
                    // Bring device out of the calibration mode irrespective of the status.
                    MoveDeviceOutOfCalibrationMode();

                } else {

                    if( objCalibErrorResponseJson.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
                        lstrDevcErrorMessage = objCalibErrorResponseJson[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
                    } else {
                        lstrDevcErrorMessage = "Should not happen. Http saveDeviceCalibError failed.";
                    }
                    Serial.println(lstrDevcErrorMessage);

                    lstrDevcErrorVal = objCalibErrorResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) ?
                                    objCalibErrorResponseJson[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
                }

            }
        
        } else {

            lstrDevcErrorVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
            Serial.printf("\nHTTP Post failed while Sending the saveCurrentCalibStepResult. Result Not HTTP_CODE_OK");
        }
    } else {
 
        lstrDevcErrorVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
        Serial.printf("\nHTTP Post failed while Sending the saveCurrentCalibStepResult. Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();  //Free resources

    return lstrDevcErrorVal;
    
}

String SendAckForDeviceOutOfCalibMode() {

    String lstrDevcAckVal = STATUS_SUCCESS;

    if( mapDeviceInfo.size() <= 0 || mapConnectionInfo.size() <= 0 ||
        mapConnectionInfo.find(DEVC_API_URL) == mapConnectionInfo.end() ||
        mapDeviceInfo.find(V_DEVICE_ID) == mapDeviceInfo.end() ||
        mapDeviceInfo.find(V_DEVICE_AUTH) == mapDeviceInfo.end() ) {

        Serial.println("Required Keys are missing from the DeviceInfo/Connection Map.");

        lstrDevcAckVal = REQUIRED_KEYS_MISSING_IN_MAP; // Required Keys are missing from the DeviceInfo/Connection Map.
        return lstrDevcAckVal;
    }

    // Request for sending Acknowledgement.
    HTTPClient http;

    String lstrAckCalibMessage = "";
    DeserializationError err;

    String strAckCalibPostBody =  String("{") +
                            "\"DeviceID\": \"" + mapDeviceInfo[V_DEVICE_ID] + "\"," +
                            "\"DeviceAuth\": \"" + mapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                            "\"ActionStatus\": \"ExitedCalibrationMode\""
                            "}";

    http.begin( mapConnectionInfo[DEVC_API_URL] + "vdevice/acknowledgeDeviceAction/");  // Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json"); // Specify content-type header

    // To Hide Device Auth.
    Serial.println("Sending Request for Acknowledgement to come out of Calib Mode");
    // Serial.println(strAckCalibPostBody);
    
    int httpResponseCode = http.POST(strAckCalibPostBody); // Send the actual POST request

    if( httpResponseCode > 0 ) {
        if(httpResponseCode == HTTP_CODE_OK) {

            String AckCalibresponse = http.getString(); // Get the response to the request

            Serial.println("Response from Server: ");
            Serial.println(AckCalibresponse);

            JsonObject objAckCalibResponseJson;     
            DynamicJsonDocument docCalibResponseJson(1024);
            err = deserializeJson(docCalibResponseJson, AckCalibresponse);
            if (err) {
                Serial.print("Error while converting Http Response JSON String to JSON Document");
                Serial.println(err.c_str());
            } else {
                objAckCalibResponseJson = docCalibResponseJson.as<JsonObject>();  

                if( objAckCalibResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objAckCalibResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE)) {

                    // Acknowledgement received successfully. 
                    Serial.println("Success");

                } else {

                    if( objAckCalibResponseJson.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
                        lstrAckCalibMessage = objAckCalibResponseJson[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
                    } else {
                        lstrAckCalibMessage = "Should not happen. Http Acknowledgement to come out of Calib Mode failed.";
                    }
                    Serial.println(lstrAckCalibMessage);

                    lstrDevcAckVal = objAckCalibResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) ?
                                    objAckCalibResponseJson[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
                }

            }
        
        } else {

            lstrDevcAckVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
            Serial.printf("\nHTTP Post failed while Sending the Acknowledgement to come out of Calib Mode. Result Not HTTP_CODE_OK");
        }
    } else {
 
        lstrDevcAckVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
        Serial.printf("\nHTTP Post failed while Sending the Acknowledgement to come out of Calib Mode. Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();  //Free resources

    return lstrDevcAckVal;

}

String FiregetCurrentUTCDateTime() {

    String lstrUTCretVal = STATUS_SUCCESS;

    if( mapDeviceInfo.size() <= 0 || mapConnectionInfo.size() <= 0 ||
        mapConnectionInfo.find(DEVC_API_URL) == mapConnectionInfo.end() ||
        mapDeviceInfo.find(V_DEVICE_ID) == mapDeviceInfo.end() ||
        mapDeviceInfo.find(V_DEVICE_AUTH) == mapDeviceInfo.end() ) {

        Serial.println("Required Keys are missing from the DeviceInfo/Connection Map.");

        lstrUTCretVal = REQUIRED_KEYS_MISSING_IN_MAP; // Required Keys are missing from the DeviceInfo/Connection Map.
        return lstrUTCretVal;
    }

    // Request for sending getCurrentUTCDateTime.
    HTTPClient http;

    String lstrUTCretMessage = "";
    DeserializationError err;

    String strUTCPostBody =  String("{") +
                            "\"DeviceID\": \"" + mapDeviceInfo[V_DEVICE_ID] + "\"," +
                            "\"DeviceAuth\": \"" + mapDeviceInfo[V_DEVICE_AUTH] + "\""
                            "}";

    http.begin( mapConnectionInfo[DEVC_API_URL] + "vdevice/getCurrentUTCDatetime/");  // Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json"); // Specify content-type header

    // To Hide Device Auth.
    Serial.println("Sending Request for getting current UTC Date Time");
    // Serial.println(strUTCPostBody);
    
    int httpResponseCode = http.POST(strUTCPostBody); // Send the actual POST request

    if( httpResponseCode > 0 ) {
        if(httpResponseCode == HTTP_CODE_OK) {

            String UTCresponse = http.getString(); // Get the response to the request

            Serial.println("Response from Server: ");
            Serial.println(UTCresponse);

            JsonObject objUTCResponseJson;     
            DynamicJsonDocument docUTCResponseJson(1024);
            err = deserializeJson(docUTCResponseJson, UTCresponse);
            if (err) {
                Serial.print("Error while converting Http Response JSON String to JSON Document");
                Serial.println(err.c_str());
            } else {
                objUTCResponseJson = docUTCResponseJson.as<JsonObject>();  

                if( objUTCResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objUTCResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE)) {

                    // Acknowledgement received successfully. 
                    Serial.println("Success");

                    if(objUTCResponseJson.containsKey(DEVICE_API_DEVICE_CURRUTC_TM)) {

                        CurrentUTCTimeStamp = objUTCResponseJson[DEVICE_API_DEVICE_CURRUTC_TM].as<String>();
                        Serial.println("The UNIX Time Stamp Extracted is: ");
                        Serial.println(CurrentUTCTimeStamp);
                        // Set the RTC again using the extracted UNIX TimeStamp.
                        ResetRTC();
                    } else {

                        Serial.println("UNIXTimeStamp key not found");
                    }

                } else {

                    if( objUTCResponseJson.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
                        lstrUTCretMessage = objUTCResponseJson[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
                    } else {
                        lstrUTCretMessage = "Should not happen. Http getCurrentUTCDatetime failed.";
                    }
                    Serial.println(lstrUTCretMessage);

                    lstrUTCretVal = objUTCResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) ?
                                    objUTCResponseJson[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
                }

            }
        
        } else {

            lstrUTCretVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
            Serial.printf("\nHTTP Post failed while Sending the getCurrentUTCDatetime. Result Not HTTP_CODE_OK");
        }
    } else {
 
        lstrUTCretVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
        Serial.printf("\nHTTP Post failed while Sending the getCurrentUTCDatetime. Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();  //Free resources

    return lstrUTCretVal;

}

void ResetRTC() {

    int UNIXnum = CurrentUTCTimeStamp.length();
    // Declaring character array 
    char UnixcharArr[UNIXnum + 1]; 
    // Copying the contents of the string to char array 
    strcpy(UnixcharArr, CurrentUTCTimeStamp.c_str()); 
    setRTCTimeFromUNIX(UnixcharArr);

}

String SendAcknowledgementforDeviceUpdates() {

    String lstrDevcAckretVal = STATUS_SUCCESS;

    if( mapDeviceInfo.size() <= 0 || mapConnectionInfo.size() <= 0 ||
        mapConnectionInfo.find(DEVC_API_URL) == mapConnectionInfo.end() ||
        mapDeviceInfo.find(V_DEVICE_ID) == mapDeviceInfo.end() ||
        mapDeviceInfo.find(V_DEVICE_AUTH) == mapDeviceInfo.end() ) {

        Serial.println("Required Keys are missing from the DeviceInfo/Connection Map.");

        lstrDevcAckretVal = REQUIRED_KEYS_MISSING_IN_MAP; // Required Keys are missing from the DeviceInfo/Connection Map.
        return lstrDevcAckretVal;
    }

    // Request for sending Device Update Status.
    HTTPClient http;

    String lstrDevcAckMessage = "";
    DeserializationError err;

    if(!NewAlertSet) {

        strDevcAckPostBody =  String("{") +
                            "\"DeviceID\": \"" + mapDeviceInfo[V_DEVICE_ID] + "\"," +
                            "\"DeviceAuth\": \"" + mapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                            "\"DeviceInfo\": " + strDevcinfo + " "
                            "}";

    } else if(NewAlertSet) {

        strDevcAckPostBody =  String("{") +
                            "\"DeviceID\": \"" + mapDeviceInfo[V_DEVICE_ID] + "\"," +
                            "\"DeviceAuth\": \"" + mapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                            "\"SettingsInfo\": true "
                            "}";

    }
    

    http.begin( mapConnectionInfo[DEVC_API_URL] + "vdevice/reportDeviceUpdateStatus/");  // Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json"); // Specify content-type header

    // To Hide Device Auth.
    Serial.println("Sending Request for sending reportDeviceUpdateStatus");
    // Serial.println(strDevcAckPostBody);
    
    int httpResponseCode = http.POST(strDevcAckPostBody); // Send the actual POST request

    if( httpResponseCode > 0 ) {
        if(httpResponseCode == HTTP_CODE_OK) {

            String DevcAckresponse = http.getString(); // Get the response to the request

            Serial.println("Response from Server: ");
            Serial.println(DevcAckresponse);

            JsonObject objDevcAckResponseJson;     
            DynamicJsonDocument docDevcAckResponseJson(1024);
            err = deserializeJson(docDevcAckResponseJson, DevcAckresponse);
            if (err) {
                Serial.print("Error while converting Http Response JSON String to JSON Document");
                Serial.println(err.c_str());
            } else {
                objDevcAckResponseJson = docDevcAckResponseJson.as<JsonObject>();  

                if( objDevcAckResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objDevcAckResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE)) {

                    // Acknowledgement received successfully. 
                    Serial.println("Success");

                } else {

                    if( objDevcAckResponseJson.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
                        lstrDevcAckMessage = objDevcAckResponseJson[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
                    } else {
                        lstrDevcAckMessage = "Should not happen. Http reportDeviceUpdateStatus failed.";
                    }
                    Serial.println(lstrDevcAckMessage);

                    lstrDevcAckretVal = objDevcAckResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) ?
                                    objDevcAckResponseJson[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
                }

            }
        
        } else {

            lstrDevcAckretVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
            Serial.printf("\nHTTP Post failed while Sending the reportDeviceUpdateStatus. Result Not HTTP_CODE_OK");
        }
    } else {
 
        lstrDevcAckretVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
        Serial.printf("\nHTTP Post failed while Sending the reportDeviceUpdateStatus. Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();  //Free resources

    return lstrDevcAckretVal;

}


// This API will be fired first on reboot of the device only if DEVC_SHARE_FW_INFO Flag is true, meaning OTA is completed successfully.
String SendStatusForOTAUpdate() {

    String lstrOTAAckretVal = STATUS_SUCCESS;

    if( mapDeviceInfo.size() <= 0 || mapConnectionInfo.size() <= 0 ||
        mapConnectionInfo.find(DEVC_API_URL) == mapConnectionInfo.end() ||
        mapDeviceInfo.find(V_DEVICE_ID) == mapDeviceInfo.end() ||
        mapDeviceInfo.find(V_DEVICE_AUTH) == mapDeviceInfo.end() ) {

        Serial.println("Required Keys are missing from the DeviceInfo/Connection Map.");

        lstrOTAAckretVal = REQUIRED_KEYS_MISSING_IN_MAP; // Required Keys are missing from the DeviceInfo/Connection Map.
        return lstrOTAAckretVal;
    }

    // Request for sending OTA Device Update Status.
    HTTPClient http;

    String lstrDevcOTAAckMessage = "";
    DeserializationError err;

    // Send the OTA Successful indication only when the DEVC_SHARE_FW_INFO Flag is High as this API will be Fired First after Reboot before Registration meaning the OTA is successful.
    if(mapCalibrationInfo[DEVC_SHARE_FW_INFO]) {

        strOTADevcAckPostBody =  String("{") +
                            "\"DeviceID\": \"" + mapDeviceInfo[V_DEVICE_ID] + "\"," +
                            "\"DeviceAuth\": \"" + mapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                            "\"OTAInfo\": {" +
                                        "\"UpdateStatus\": \"SUCCESS\"," +
                                        "\"FrmwrID\": " + mapDeviceInfo[RES_KEY_OTA_FW_ID] + ", " +
                                        "\"UpdateKey\": \"" + mapDeviceInfo[RES_KEY_OTA_UPD_KEY] + "\" }" +
                            "}";

    } else {

        Serial.println("The New Firmware Indication Flag is low. Please check.");
    }
    

    http.begin( mapConnectionInfo[DEVC_API_URL] + "vdevice/reportDeviceUpdateStatus/");  // Specify destination for HTTP request
    http.addHeader("Content-Type", "application/json"); // Specify content-type header

    // To Hide Device Auth.
    Serial.println("Sending Request for sending OTADeviceUpdateStatus");
    // Serial.println(strOTADevcAckPostBody);
    
    int httpResponseCode = http.POST(strOTADevcAckPostBody); // Send the actual POST request

    if( httpResponseCode > 0 ) {
        if(httpResponseCode == HTTP_CODE_OK) {

            String OTADevcAckresponse = http.getString(); // Get the response to the request

            Serial.println("Response from Server: ");
            Serial.println(OTADevcAckresponse);

            JsonObject objOTADevcAckResponseJson;     
            DynamicJsonDocument docOTADevcAckResponseJson(1024);
            err = deserializeJson(docOTADevcAckResponseJson, OTADevcAckresponse);
            if (err) {
                Serial.print("Error while converting Http Response JSON String to JSON Document");
                Serial.println(err.c_str());
            } else {
                objOTADevcAckResponseJson = docOTADevcAckResponseJson.as<JsonObject>();  

                if( objOTADevcAckResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) &&
                    objOTADevcAckResponseJson[DEVICE_API_RESPONSE_CODE].as<String>().equals(DEVICE_API_RESPONSE_SUCCESS_CODE)) {

                    // Acknowledgement received successfully. 
                    Serial.println("Success");

                    Serial.println("OTA Completed successfully.Shared Info of new Firmware successfully.Setting ShareFwInfo Flag as false");
                    // Set the DEVC_SHARE_FW_INFO flag to false so that this API won't be executed on next Reboot or when Wifi gets connected.
                    mapCalibrationInfo[DEVC_SHARE_FW_INFO] = 0;
                    writeCalibrationInfoToFile(mapCalibrationInfo);

                } else {

                    if( objOTADevcAckResponseJson.containsKey(DEVICE_API_RESPONSE_FAILURE_MESSAGE) ) {
                        lstrDevcOTAAckMessage = objOTADevcAckResponseJson[DEVICE_API_RESPONSE_FAILURE_MESSAGE].as<String>();
                    } else {
                        lstrDevcOTAAckMessage = "Should not happen. Http OTADeviceUpdateStatus failed.";
                    }
                    Serial.println(lstrDevcOTAAckMessage);

                    lstrOTAAckretVal = objOTADevcAckResponseJson.containsKey(DEVICE_API_RESPONSE_CODE) ?
                                    objOTADevcAckResponseJson[DEVICE_API_RESPONSE_CODE].as<String>() : DEVICE_API_RESPONSE_UNKNOWN_STATUS;
                }

            }
        
        } else {

            lstrOTAAckretVal = DEVICE_API_RESPONSE_HTTP_NOT_OK;
            Serial.printf("\nHTTP Post failed while Sending the OTADeviceUpdateStatus. Result Not HTTP_CODE_OK");
        }
    } else {
 
        lstrOTAAckretVal = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
        Serial.printf("\nHTTP Post failed while Sending the OTADeviceUpdateStatus. Error: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();  //Free resources

    return lstrOTAAckretVal;

}
