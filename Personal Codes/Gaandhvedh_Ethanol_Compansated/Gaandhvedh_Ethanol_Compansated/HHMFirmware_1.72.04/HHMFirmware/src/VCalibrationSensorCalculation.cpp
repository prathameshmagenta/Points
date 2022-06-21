#include <Arduino.h>
#include <map>
#include "VCalibrationSensorCalculation.h"
#include "VCalibrationAPIExecution.h"
#include "VInformationManager.h"
#include "VDefines.h"
#include "VSensors.h"
#include "VNH3.h"
#include "VDelay.h"
#include "VUtils.h"

extern std::map<String, String> mapDeviceInfo;                                  
extern std::map<String, String> mapConnectionInfo; 

extern DELAY objTGSStartCalibDelay;
extern DELAY objTGSWaitCalibDelay;
extern DELAY objTGSReadDelay;

String strCalibResultStat = "";
String strBodySaveCalibResult = "";
String strBodyDevcCalibError = "";
String strSaveTimeOutError = "";
String strSaveCalibStepRes = "";
String strCalibMapStat = "";
String strCalibSensorMapStat = "";
String strSPIFFSStat = CALIB_STATUS_OK;
String strSPIFFSSensorStat = CALIB_STATUS_OK;
float HDCTempVal = 0.0f;
float HDCHumVal = 0.0f;
float tempVal = 0.0f;
float humVal = 0.0f;
uint8_t tempCali = 0;

/********* NH3OD **********/
float MinRSValforNH3OD = 0.0f;
float referenceRS = 0.0f;
float newRefR0=0.0f;
float UserInputforNH3ODStep1 = 0.0f;
float MinRSValforNH3ODFirstPPM = 0.0f;
float UserInputforNH3ODStep2 = 0.0f;
float MinRSValforNH3ODSecondPPM = 0.0f;
extern String NH3ODStat;
String TGSCaliStat = "";
String CalibStatNH3Step1 = "";
String CalibStatNH3Step2 = "";
float NH3ODcalculatedPPMVal = 0.0f;
float NH3ODcalPPMRecalibVal = 0.0f;
float CurrentNH3ODRsVal = 0.0f;
float CurrentNH3ODRsValComp = 0.0f;

/********* ETHNL **********/
float MinRSValforETHNL = 0.0f;
float UserInputforETHNLStep1 = 0.0f;
float MinRSValforETHNLFirstPPM = 0.0f;
float UserInputforETHNLStep2 = 0.0f;
float MinRSValforETHNLSecondPPM = 0.0f;
extern String EthanolStat;
String CalibStatETHStep1 = "";
String CalibStatETHStep2 = "";
float ETHNLcalculatedPPMVal = 0.0f;
float ETHNLcalPPMRecalibVal = 0.0f;
float CurrentETHNLRsVal = 0.0f;
float CurrentETHNLRsValComp = 0.0f;

String UnixTimeStamp = "";
extern String RTCUnixTimeStamp;
extern String UTCDateTime;
String RtcCalibTimeDate = "";
String RtcCalibDate = "";
String RtcCalibTime = "";
uint32_t RTCSetUNIXTime;

// Do not send any other API while calibration of 2 step gas is going ON.
bool BlockCaliAPIfor2StepGas = false;
extern bool isDevcinCalibrationMode;
// For Acknowledgement for Calibration.
extern bool isAckforcalibSent;
// For invoking getCurrentCalibStepInfo API again after 8 sec to check if user has pressed any button.
extern bool invokegetCalibStepInfoAgain;
// This flag checks whether sensor calculation has started or not.
extern bool hasSensorCalculationsBegun;
// Make the hasTimeOutOccuredduringSensorCal flag true if timeout occurs while a step was going on,after 30 minutes.
extern bool hasTimeOutOccuredDuringSensorCal;
// Make the hasTimeOutOccured flag true if timeout occurs before a step was started,after 30 minutes.
extern bool hasTimeOutOccuredAfterSensorCal;
// Check if Step ONE is called again keeping Step TWO as it is.
extern bool invokeStepOneAgain;

String CalibStatTempStep1 = "";
String CalibStatTempStep2 = "";
String CalibStatTempStep3 = "";
String CalibStatRHStep1 = "";
String CalibStatRHStep2 = "";
String CalibStatRHStep3 = "";
String TimeOutStat = CALIB_TIMEOUT_WAITING_STEP_INPUT;

uint8_t CalibCount = 0;

SENSOR_RETRIEVE_MIN_VAL objRetrieveMinVal;

float BeginSensorCalibration(std::map<String, uint32_t>& inMapCalibrationInfo,
                            std::map<String, double>& inMapCalibrationSensorValInfo,
                            std::map<String, String>& inMapConnectionInfo,
                            std::map<String, String>& inMapDeviceInfo,
                            String strParamName,
                            float UserEnteredValStep1,
                            float UserEnteredValStep2, 
                            float UserEnteredValStep3,
                            String CalibStepNum,                             
                            String Step1StatInfo,
                            String Step2StatInfo,
                            String Step3StatInfo,
                            float PostCalibCalcValue) {

    if(strParamName == VMP_TEMP) {

        // Do not calculate again if timeout has occured.
        if((!hasTimeOutOccuredDuringSensorCal) && (!hasTimeOutOccuredAfterSensorCal)) {

            sensor_getParamFromHDC2080(HDCTempVal, HDCHumVal);
            Serial.printf("Current Temp: %0.2fdegC\r\n", HDCTempVal); 
            sensor_readTempCali(tempCali);
            Serial.printf("Curr Temp Cali: %x\r\n", tempCali);   tempCali = 0;
            inMapCalibrationInfo[VMP_TEMP] = sensor_calibrateSensor(VMP_TEMP, HDCTempVal, UserEnteredValStep1);
            // Save the calculations in SPIFFS.
            strCalibMapStat = writeCalibrationInfoToFile(inMapCalibrationInfo);
            CheckWriteToCalibrationMapStatus(strCalibMapStat);
            if(sensor_validateCaliHDC2080(VMP_TEMP)){

                Serial.printf("Temp Calibration Done, %x.\r\n", inMapCalibrationInfo[VMP_TEMP]);
                strCalibResultStat = CALIB_STATUS_OK;
            } else {

                Serial.printf("Temp Calibration Failed.\r\n");
                strCalibResultStat = CALIB_STATUS_FAILED;
            }

        } 
        // Forming the JSON Body according the Input Step.
        if(CalibStepNum == DEVICE_API_CALIB_STATUS_STEP1) {

            // Use the current status for the current step and Old Status from the API for inserting it in the JSON Body.
            CalibStatTempStep1 = strCalibResultStat;
            
            // If SPIFFS has encountered any error while writing then send that error as highest priority.
            CalibStatTempStep1 = (strSPIFFSStat != CALIB_STATUS_OK) ? strSPIFFSStat : CalibStatTempStep1;

            // If timeout has occured during the sensor calculation, then substitute here only and send the JSON body.
            CalibStatTempStep1 = (hasTimeOutOccuredDuringSensorCal == true) ? CALIB_TIMEOUT_WAITING_STEP_INPUT : CalibStatTempStep1;

            strBodySaveCalibResult = String("{") +
                                    "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                    "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                    "\"MeasuredParam\": \"" + strParamName + "\"," +
                                    "\"CalibValues\": {" "\"Step1Inp\": " + UserEnteredValStep1 + "}, " +
                                    "\"DevcCalibStatus\": {" "\"Step1\": \"" + CalibStatTempStep1 + "\" }, " +
                                    "\"CalibTime\": null "
                                    "}";

            if(hasTimeOutOccuredDuringSensorCal) {

                Serial.println("Timeout has occured while executing Temperature Step ONE.Firing TimeOut API");
                // If the timeout has occured during the calibration process, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.
                // This doesn't work.The variable doesn't get substituted with timeout condition.
                // CalibStatTempStep1 = TimeOutStat;
                FireSaveCalibErrorToServer(strBodySaveCalibResult);                            
            
            } else if(hasTimeOutOccuredAfterSensorCal) {
                
                // Now suppose user has pressed TEMP button but hasn't sent the TEMP value the other day and device has rebooted previously and timeout occurs,
                // Then the stat, inputs are obtained from API getCurrentCalibStepInfo except the calculated values.They are taken from maps.But if timeout occurs just
                // after the sensor calculation is done, then the present stat is sent.So check the String length.
                CalibStatTempStep1 = (CalibStatTempStep1.length() <= 0) ? Step1StatInfo : CalibStatTempStep1;

                strBodyDevcCalibError = String("{") +
                                        "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                        "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                        "\"MeasuredParam\": \"" + strParamName + "\"," +
                                        "\"CalibValues\": {" "\"Step1Inp\": " + UserEnteredValStep1 + "}, " +
                                        "\"DevcCalibStatus\": {" +
                                                            "\"Step1\": \"" + CalibStatTempStep1 + "\" ," +
                                                            "\"StepStatus\": \"" + TimeOutStat + "\" }, " +
                                        "\"CalibTime\": null "
                                        "}";

                Serial.println("Timeout has occured after executing Temperature Step ONE.Firing TimeOut API");
                // If the timeout has occured after the calibration step, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.                    
                FireSaveCalibErrorToServer(strBodyDevcCalibError);

            } else {
            
                // The HTTP Post Body is written just above the Timeout conditions.
                Serial.println("The HTTP Body for Step ONE for Temperature has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                hasSensorCalculationsBegun = false;

                // Send this Post Body and send it to the Server via HTTPS.
                FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);
                
                // Check error status of temperature
                // Come out of Calibration Mode if Temperature calibration has encountered any error.
                if(CalibStatTempStep1 != CALIB_STATUS_OK) {

                    Serial.println("Error encountered for Step ONE for Temperature.");
                }

            }   

        } else if(CalibStepNum == DEVICE_API_CALIB_STATUS_STEP2) {
            
            // Use the current status for the current step and Old Status from the API for inserting it in the JSON Body.
            CalibStatTempStep2 = strCalibResultStat;

            // If SPIFFS has encountered any error while writing then send that error as highest priority.
            CalibStatTempStep2 = (strSPIFFSStat != CALIB_STATUS_OK) ? strSPIFFSStat : CalibStatTempStep2;

            // If timeout has occured during the sensor calculation, then substitute here only and send the JSON body.
            CalibStatTempStep2 = (hasTimeOutOccuredDuringSensorCal == true) ? CALIB_TIMEOUT_WAITING_STEP_INPUT : CalibStatTempStep2;

            strBodySaveCalibResult = String("{") +
                                    "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                    "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                    "\"MeasuredParam\": \"" + strParamName + "\"," +
                                    "\"CalibValues\": {" +
                                                    "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                    "\"Step2Inp\": " + UserEnteredValStep2 + " }, " +
                                    "\"DevcCalibStatus\": {" +
                                                        "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                        "\"Step2\": \"" + CalibStatTempStep2 + "\" }, " +
                                    "\"CalibTime\": null "
                                    "}";

            if(hasTimeOutOccuredDuringSensorCal) {

                Serial.println("Timeout has occured while executing Temperature Step TWO.Firing TimeOut API");
                // If the timeout has occured during the calibration process, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.
                FireSaveCalibErrorToServer(strBodySaveCalibResult);                            
            
            } else if(hasTimeOutOccuredAfterSensorCal) {
                
                // Now suppose user has pressed TEMP button but hasn't sent the TEMP value the other day and device has rebooted previously and timeout occurs,
                // Then the stat, inputs are obtained from API getCurrentCalibStepInfo except the calculated values.They are taken from maps.But if timeout occurs just
                // after the sensor calculation is done, then the present stat is sent.So check the String length.
                CalibStatTempStep2 = (CalibStatTempStep2.length() <= 0) ? Step2StatInfo : CalibStatTempStep2;

                strBodyDevcCalibError = String("{") +
                                        "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                        "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                        "\"MeasuredParam\": \"" + strParamName + "\"," +
                                        "\"CalibValues\": {" +
                                                        "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                        "\"Step2Inp\": " + UserEnteredValStep2 + " }, " +
                                        "\"DevcCalibStatus\": {" +
                                                            "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                            "\"Step1\": \"" + CalibStatTempStep2 + "\" ," +
                                                            "\"StepStatus\": \"" + TimeOutStat + "\" }, " +
                                        "\"CalibTime\": null "
                                        "}";

                Serial.println("Timeout has occured after executing Temperature Step TWO.Firing TimeOut API");
                // If the timeout has occured after the calibration step, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.                    
                FireSaveCalibErrorToServer(strBodyDevcCalibError);

            } else {             
            
                // The HTTP Post Body is written just above the Timeout conditions.
                Serial.println("The HTTP Body for Step TWO for Temperature has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                hasSensorCalculationsBegun = false;

                // Send this Post Body and send it to the Server via HTTPS.
                FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);    
                
                // Check error status of temperature
                // Come out of Calibration Mode if Temperature calibration has encountered any error.
                if(CalibStatTempStep2 != CALIB_STATUS_OK) {

                    Serial.println("Error encountered for Step TWO for Temperature.");
                }

            }

        } else if(CalibStepNum == DEVICE_API_CALIB_STATUS_STEP3) {

            // Use the current status for the current step and Old Status from the API for inserting it in the JSON Body.
            CalibStatTempStep3 = strCalibResultStat;

            // Do not calculate again if timeout has occured.
            if((!hasTimeOutOccuredDuringSensorCal) && (!hasTimeOutOccuredAfterSensorCal)) {

                if(CalibStatTempStep3 == CALIB_STATUS_OK) {

                    sensor_getParamFromHDC2080(tempVal, humVal);
                    if(abs(tempVal - UserEnteredValStep3) <= 0.8) {

                        Serial.println("Also Sensor Temp Val is within tolerance");
                        CalibStatTempStep3 = CALIB_STATUS_OK;
                    } else {

                        Serial.println("But Sensor Temp Val is not within tolerance");
                        CalibStatTempStep3 = CALIBVAL_NOT_WITHIN_TOLERANCE;
                    }

                }
            }

            // If SPIFFS has encountered any error while writing then send that error as highest priority.
            CalibStatTempStep3 = (strSPIFFSStat != CALIB_STATUS_OK) ? strSPIFFSStat : CalibStatTempStep3;

            if((CalibStatTempStep3 != CALIB_STATUS_OK) || hasTimeOutOccuredDuringSensorCal || hasTimeOutOccuredAfterSensorCal) {

                // If timeout has occured during the sensor calculation, then substitute here only and send the JSON body.
                CalibStatTempStep3 = (hasTimeOutOccuredDuringSensorCal == true) ? CALIB_TIMEOUT_WAITING_STEP_INPUT : CalibStatTempStep3;

                // Send this ERROR Body Json for timeouts or for any Calibration Error.Check if the last step has 'OK' Status.If not, do not send the PostCalOutput and send the FinalParamCalibStatus as false.
                strBodySaveCalibResult = String("{") +
                                        "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                        "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                        "\"MeasuredParam\": \"" + strParamName + "\"," +
                                        "\"CalibValues\": {" +
                                                        "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                        "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                        "\"Step3Inp\": " + UserEnteredValStep3 + " }, " +
                                        "\"DevcCalibStatus\": {" +
                                                            "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                            "\"Step2\": \"" + Step2StatInfo + "\" ," +
                                                            "\"Step3\": \"" + CalibStatTempStep3 + "\" , " +
                                                            "\"AllStepsDone\": true ," +
                                                            "\"FinalParamCalibStatus\": false }, " +
                                        "\"CalibTime\": null "
                                        "}";

                if(hasTimeOutOccuredDuringSensorCal) {

                    Serial.println("Timeout has occured while executing Temperature Step THREE.Firing TimeOut API");
                    // If the timeout has occured during the calibration process, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.
                    FireSaveCalibErrorToServer(strBodySaveCalibResult);                            
                
                } else if(hasTimeOutOccuredAfterSensorCal) {

                    // No need to fire any API, just bring the device out of Calibration Mode thats all.
                    Serial.println("Timeout has occured after executing Temperature Step THREE.Moving Device out of Calibration Mode.");
                    MoveDeviceOutOfCalibrationMode();

                } else {

                    // The HTTP Post Body is written just above the Timeout conditions.
                    Serial.println("The HTTP Body for Step THREE failure for Temperature has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                    hasSensorCalculationsBegun = false;
                    
                    // Send this Post Body and send it to the Server via HTTPS.
                    FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);
                    
                    // Check error status of temperature
                    // Come out of Calibration Mode if Temperature calibration has encountered any error.
                    Serial.println("Error encountered for Step THREE for Temperature.");

                }

            } else if(CalibStatTempStep3 == CALIB_STATUS_OK) {
                // Check if the last step has 'OK' Status.Only then send the PostCalOutput and send the FinalParamCalibStatus as true.
                strBodySaveCalibResult = String("{") +
                                        "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                        "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                        "\"MeasuredParam\": \"" + strParamName + "\"," +
                                        "\"CalibValues\": {" +
                                                        "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                        "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                        "\"Step3Inp\": " + UserEnteredValStep3 + "," +
                                                        "\"PostCalOutput\": " + tempVal + " }, " +
                                        "\"DevcCalibStatus\": {" +
                                                            "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                            "\"Step2\": \"" + Step2StatInfo + "\" ," +
                                                            "\"Step3\": \"" + CalibStatTempStep3 + "\" , " +
                                                            "\"AllStepsDone\": true ," +
                                                            "\"FinalParamCalibStatus\": true }, " +
                                        "\"CalibTime\": null "
                                        "}";
            
                Serial.println("The HTTP Body for Step THREE for Temperature has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                hasSensorCalculationsBegun = false;

                // Send this Post Body and send it to the Server via HTTPS.
                FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);                           
                                
            } else {

                Serial.println("Please check the Step THREE Status for Temperature");
            }

        } else {

            Serial.println("Undefined Step for Temperature Param");
        }

    } else if(strParamName == VMP_HUM) {

        // Do not calculate again if timeout has occured.
        if((!hasTimeOutOccuredDuringSensorCal) && (!hasTimeOutOccuredAfterSensorCal)) {

            sensor_getParamFromHDC2080(HDCTempVal, HDCHumVal);
            Serial.printf("Current Humidity: %0.2f%%\r\n", HDCHumVal);
            sensor_readHumCali(tempCali);
            Serial.printf("Curr Temp Cali: %x\r\n", tempCali);   tempCali = 0; 
            inMapCalibrationInfo[VMP_HUM] = sensor_calibrateSensor(VMP_HUM, HDCHumVal, UserEnteredValStep1);
            // Save the calculations in SPIFFS.
            strCalibMapStat = writeCalibrationInfoToFile(inMapCalibrationInfo);
            CheckWriteToCalibrationMapStatus(strCalibMapStat);
            if(sensor_validateCaliHDC2080(VMP_HUM)){

                Serial.printf("RH Calibration Done, %x.\r\n", inMapCalibrationInfo[VMP_HUM]);
                strCalibResultStat = CALIB_STATUS_OK;
            } else {
            
                Serial.printf("RH Calibration Failed.\r\n");
                strCalibResultStat = CALIB_STATUS_FAILED;
            }

        }
        // Forming the JSON Body according the Input Step.
        if(CalibStepNum == DEVICE_API_CALIB_STATUS_STEP1) {

            // Use the current status for the current step and Old Status from the API for inserting it in the JSON Body.
            CalibStatRHStep1 = strCalibResultStat;

            // If SPIFFS has encountered any error while writing then send that error as highest priority.
            CalibStatRHStep1 = (strSPIFFSStat != CALIB_STATUS_OK) ? strSPIFFSStat : CalibStatRHStep1;

            // If timeout has occured during the sensor calculation, then substitute here only and send the JSON body.
            CalibStatRHStep1 = (hasTimeOutOccuredDuringSensorCal == true) ? CALIB_TIMEOUT_WAITING_STEP_INPUT : CalibStatRHStep1;

            strBodySaveCalibResult = String("{") +
                                    "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                    "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                    "\"MeasuredParam\": \"" + strParamName + "\"," +
                                    "\"CalibValues\": {" "\"Step1Inp\": " + UserEnteredValStep1 + "}, " +
                                    "\"DevcCalibStatus\": {" "\"Step1\": \"" + CalibStatRHStep1 + "\" }, " +
                                    "\"CalibTime\": null " 
                                    "}";

            if(hasTimeOutOccuredDuringSensorCal) {

                Serial.println("Timeout has occured while executing Humidity Step ONE.Firing TimeOut API");
                // If the timeout has occured during the calibration process, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.
                FireSaveCalibErrorToServer(strBodySaveCalibResult);                            
            
            } else if(hasTimeOutOccuredAfterSensorCal) {

                // Now suppose user has pressed RH button but hasn't sent the RH value the other day and device has rebooted previously and timeout occurs,
                // Then the stat, inputs are obtained from API getCurrentCalibStepInfo except the calculated values.They are taken from maps.But if timeout occurs just
                // after the sensor calculation is done, then the present stat is sent.So check the String length.
                CalibStatRHStep1 = (CalibStatRHStep1.length() <= 0) ? Step1StatInfo : CalibStatRHStep1;
                
                strBodyDevcCalibError = String("{") +
                                        "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                        "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                        "\"MeasuredParam\": \"" + strParamName + "\"," +
                                        "\"CalibValues\": {" "\"Step1Inp\": " + UserEnteredValStep1 + "}, " +
                                        "\"DevcCalibStatus\": {" +
                                                            "\"Step1\": \"" + CalibStatRHStep1 + "\" ," +
                                                            "\"StepStatus\": \"" + TimeOutStat + "\" }, " +
                                        "\"CalibTime\": null "
                                        "}";

                Serial.println("Timeout has occured after executing Humidity Step ONE.Firing TimeOut API");
                // If the timeout has occured after the calibration step, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.                    
                FireSaveCalibErrorToServer(strBodyDevcCalibError);
            } else {

                // The HTTP Post Body is written just above the Timeout conditions.
                Serial.println("The HTTP Body for Step ONE for Humidity has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                hasSensorCalculationsBegun = false;

                // Send this Post Body and send it to the Server via HTTPS.
                FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);
                
                // Check error status of Humidity.
                // Come out of Calibration Mode if Humidity calibration has encountered any error.
                if(CalibStatRHStep1 != CALIB_STATUS_OK) {

                    Serial.println("Error encountered for Step ONE for Humidity.");
                }

            }

        } else if(CalibStepNum == DEVICE_API_CALIB_STATUS_STEP2) {
            
            // Use the current status for the current step and Old Status from the API for inserting it in the JSON Body.
            CalibStatRHStep2 = strCalibResultStat;

            // If SPIFFS has encountered any error while writing then send that error as highest priority.
            CalibStatRHStep2 = (strSPIFFSStat != CALIB_STATUS_OK) ? strSPIFFSStat : CalibStatRHStep2;

            // If timeout has occured during the sensor calculation, then substitute here only and send the JSON body.
            CalibStatRHStep2 = (hasTimeOutOccuredDuringSensorCal == true) ? CALIB_TIMEOUT_WAITING_STEP_INPUT : CalibStatRHStep2;

            strBodySaveCalibResult = String("{") +
                                    "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                    "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                    "\"MeasuredParam\": \"" + strParamName + "\"," +
                                    "\"CalibValues\": {" +
                                                    "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                    "\"Step2Inp\": " + UserEnteredValStep2 + " }, " +
                                    "\"DevcCalibStatus\": {" +
                                                        "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                        "\"Step2\": \"" + CalibStatRHStep2 + "\" }, " +
                                    "\"CalibTime\": null " 
                                    "}";
            
            if(hasTimeOutOccuredDuringSensorCal) {

                Serial.println("Timeout has occured while executing Humidity Step TWO.Firing TimeOut API");
                // If the timeout has occured during the calibration process, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.
                FireSaveCalibErrorToServer(strBodySaveCalibResult);                            
            
            } else if(hasTimeOutOccuredAfterSensorCal) {

                // Now suppose user has pressed RH button but hasn't sent the RH value the other day and device has rebooted previously and timeout occurs,
                // Then the stat, inputs are obtained from API getCurrentCalibStepInfo except the calculated values.They are taken from maps.But if timeout occurs just
                // after the sensor calculation is done, then the present stat is sent.So check the String length.
                CalibStatRHStep2 = (CalibStatRHStep2.length() <= 0) ? Step2StatInfo : CalibStatRHStep2;
                
                strBodyDevcCalibError = String("{") +
                                        "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                        "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                        "\"MeasuredParam\": \"" + strParamName + "\"," +
                                        "\"CalibValues\": {" +
                                                        "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                        "\"Step2Inp\": " + UserEnteredValStep2 + " }, " +
                                        "\"DevcCalibStatus\": {" +
                                                            "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                            "\"Step2\": \"" + CalibStatRHStep2 + "\" ," +
                                                            "\"StepStatus\": \"" + TimeOutStat + "\" }, " +
                                        "\"CalibTime\": null "
                                        "}";

                Serial.println("Timeout has occured after executing Humidity Step TWO.Firing TimeOut API");
                // If the timeout has occured after the calibration step, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.                    
                FireSaveCalibErrorToServer(strBodyDevcCalibError);

            } else {

                // The HTTP Post Body is written just above the Timeout conditions.
                Serial.println("The HTTP Body for Step TWO for Humidity has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                hasSensorCalculationsBegun = false;

                // Send this Post Body and send it to the Server via HTTPS.
                FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);
                
                // Check error status of Humidity.
                // Come out of Calibration Mode if Humidity calibration has encountered any error.
                if(CalibStatRHStep2 != CALIB_STATUS_OK) {

                    Serial.println("Error encountered for Step TWO for Humidity.");
                }

            }

        } else if(CalibStepNum == DEVICE_API_CALIB_STATUS_STEP3) {

            // Use the current status for the current step and Old Status from the API for inserting it in the JSON Body.
            CalibStatRHStep3 = strCalibResultStat;

            // Do not calculate again if timeout has occured.
            if((!hasTimeOutOccuredDuringSensorCal) && (!hasTimeOutOccuredAfterSensorCal)) {

                if(CalibStatRHStep3 == CALIB_STATUS_OK) {

                    sensor_getParamFromHDC2080(tempVal, humVal);
                    if(abs(humVal - UserEnteredValStep3) <= 2) {

                        Serial.println("Also Sensor RH Val is within tolerance");
                        CalibStatRHStep3 = CALIB_STATUS_OK;
                    } else {

                        Serial.println("But Sensor RH Val is not within tolerance");
                        CalibStatRHStep3 = CALIBVAL_NOT_WITHIN_TOLERANCE;
                    }

                }

            }

            // If SPIFFS has encountered any error while writing then send that error as highest priority.
            CalibStatRHStep3 = (strSPIFFSStat != CALIB_STATUS_OK) ? strSPIFFSStat : CalibStatRHStep3;

            if((CalibStatRHStep3 != CALIB_STATUS_OK) || hasTimeOutOccuredDuringSensorCal || hasTimeOutOccuredAfterSensorCal) {
                // Send this ERROR Body Json for timeouts or for any Calibration Error.Check if the last step has 'OK' Status.If not, do not send the PostCalOutput and send the FinalParamCalibStatus as false.

                // If timeout has occured during the sensor calculation, then substitute here only and send the JSON body.
                CalibStatRHStep3 = (hasTimeOutOccuredDuringSensorCal == true) ? CALIB_TIMEOUT_WAITING_STEP_INPUT : CalibStatRHStep3;

                strBodySaveCalibResult = String("{") +
                                        "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                        "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                        "\"MeasuredParam\": \"" + strParamName + "\"," +
                                        "\"CalibValues\": {" +
                                                        "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                        "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                        "\"Step3Inp\": " + UserEnteredValStep3 + " }, " +
                                        "\"DevcCalibStatus\": {" +
                                                            "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                            "\"Step2\": \"" + Step2StatInfo + "\" ," +
                                                            "\"Step3\": \"" + CalibStatRHStep3 + "\" , " +
                                                            "\"AllStepsDone\": true ," +
                                                            "\"FinalParamCalibStatus\": false }, " +
                                        "\"CalibTime\": null "
                                        "}";

                if(hasTimeOutOccuredDuringSensorCal) {

                    Serial.println("Timeout has occured while executing Humidity Step THREE.Firing TimeOut API");
                    // If the timeout has occured during the calibration process, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.
                    FireSaveCalibErrorToServer(strBodySaveCalibResult);                            
            
                } else if(hasTimeOutOccuredAfterSensorCal) {
                    // No need to fire any API, just bring the device out of Calibration Mode thats all.
                    Serial.println("Timeout has occured after executing Humidity Step THREE.Bringing Device out of Calibration Mode.");
                    MoveDeviceOutOfCalibrationMode();

                } else {

                    // The HTTP Post Body is written just above the Timeout conditions.                
                    Serial.println("The HTTP Body for Step THREE failure for Humidity has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                    hasSensorCalculationsBegun = false;
                    
                    // Send this Post Body and send it to the Server via HTTPS.
                    FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);
                    
                    // Check error status of Humidity
                    // Come out of Calibration Mode if Humidity calibration has encountered any error.
                    Serial.println("Error encountered for Step THREE for Humidity.");

                }

            } else if(CalibStatRHStep3 == CALIB_STATUS_OK) {
                // Check if the last step has 'OK' Status.Only then send the PostCalOutput and send the FinalParamCalibStatus as true.
                strBodySaveCalibResult = String("{") +
                                        "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                        "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                        "\"MeasuredParam\": \"" + strParamName + "\"," +
                                        "\"CalibValues\": {" +
                                                        "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                        "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                        "\"Step3Inp\": " + UserEnteredValStep3 + "," +
                                                        "\"PostCalOutput\": " + humVal + " }, " +
                                        "\"DevcCalibStatus\": {" +
                                                            "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                            "\"Step2\": \"" + Step2StatInfo + "\" ," +
                                                            "\"Step3\": \"" + CalibStatRHStep3 + "\" , " +
                                                            "\"AllStepsDone\": true ," +
                                                            "\"FinalParamCalibStatus\": true }, " +
                                        "\"CalibTime\": null "
                                        "}";
            
                Serial.println("The HTTP Body for Step THREE for Humidity has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                hasSensorCalculationsBegun = false;

                // Send this Post Body and send it to the Server via HTTPS.
                FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);

            } else {

                Serial.println("Please check the Step THREE Status for Humidity");
            }
            
        } else {

            Serial.println("Undefined Step for Humidity Param");
        }

    } else if(strParamName == VMP_NH3OD) {

        // Do not send any other API while calibration of 2 step gas is going ON.
        BlockCaliAPIfor2StepGas = true;
        Serial.println("Blocking all APIs for Calibration for getting Min Avg Val of NH3.");

        // Retreive the minimum Rs value of NH3OD only after 4 minutes timeout or fire the API after 30 minute timeout has occured.
        if(objTGSStartCalibDelay.isTimeOut()) {

            // Retreive the Min Val.
            MinRSValforNH3OD = objRetrieveMinVal.RetrieveAvgVal(NH3OD_SENSOR);
            Serial.printf("The Min NH3OD value encountered is: %0.4f\r\n",MinRSValforNH3OD);

            // Also clear all contents of the Array and make the counter back to zero again.
            objRetrieveMinVal.ClearContentsOfArray(NH3OD_SENSOR);

            // Timeout of 4 minutes is done, make the BlockCaliAPIfor2StepGas flag as false, so that after saveCurrentCalibStepResult, Calib 
            // Func will be executed as usual.
            BlockCaliAPIfor2StepGas = false;

            if(CalibStepNum == DEVICE_API_CALIB_STATUS_STEP1) {

                // Do not calculate again if timeout has occured.
                if((!hasTimeOutOccuredDuringSensorCal) && (!hasTimeOutOccuredAfterSensorCal)) {

                    // Find out the status which the NH3OD has encountered.
                    if(NH3ODStat == CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL) {

                        CalibStatNH3Step1 = CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL;
                        Serial.println("The Status for NH3 Calibration of first step is:CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL");
                        // Flush the value once detected.
                        NH3ODStat = "";

                    } else if(TGSCaliStat == CALIB_SENSOR_NOT_AVAILABLE) {

                        CalibStatNH3Step1 = CALIB_SENSOR_NOT_AVAILABLE;
                        Serial.println("The Status for NH3 Calibration of first step is:CALIB_SENSOR_NOT_AVAILABLE");
                        // Flush the value once detected.
                        TGSCaliStat = "";

                    } else {

                        CalibStatNH3Step1 = CALIB_STATUS_OK;
                        Serial.println("The Status for NH3 Calibration of first step is:CALIB_STATUS_OK");
                    }

                }

                if((CalibStatNH3Step1 != CALIB_STATUS_OK) || hasTimeOutOccuredDuringSensorCal || hasTimeOutOccuredAfterSensorCal) {

                    // Send this ERROR Body Json for timeouts or for any Calibration Error.Check if the last step has 'OK' Status.

                    // If timeout has occured during the sensor calculation, then substitute here only and send the JSON body.This may not happen as ESP can't multitask.But if timeout is fired first after timer of 5 min is done and hasSensorCalculationsBegun is still true
                    // Then it may happen.This is rarest of rare case scenario.
                    CalibStatNH3Step1 = (hasTimeOutOccuredDuringSensorCal == true) ? CALIB_TIMEOUT_WAITING_STEP_INPUT : CalibStatNH3Step1;

                    if(!invokeStepOneAgain) {

                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + " }," +
                                                "\"DevcCalibStatus\": {" "\"Step1\": \"" + CalibStatNH3Step1 + "\" }, " +
                                                "\"CalibTime\": null "
                                                "}";
                    } else {

                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                                "\"Step2RestncVal\": " + inMapCalibrationSensorValInfo[VMP_NH3OD_REF2] + " }," +
                                                "\"DevcCalibStatus\": {" 
                                                                    "\"Step1\": \"" + CalibStatNH3Step1 + "\", " +
                                                                    "\"Step2\": \"" + Step2StatInfo + "\" ," +
                                                                    "\"AllStepsDone\": true ," +
                                                                    "\"FinalParamCalibStatus\": false }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("Step ONE is invoked again for NH3OD keeping Step TWO intact.Picking appropriate JSON Body for error condition.Making the invokeStepOneAgain flag false.");
                        invokeStepOneAgain = false;
                    }
                    
                    if(hasTimeOutOccuredDuringSensorCal) {

                        Serial.println("Timeout has occured while executing NH3OD Step ONE.Firing TimeOut API");
                        // If the timeout has occured during the calibration process, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.
                        FireSaveCalibErrorToServer(strBodySaveCalibResult);                            
                    
                    } else if(hasTimeOutOccuredAfterSensorCal) {

                        Serial.println("Timeout has occured after executing NH3OD Step ONE.Bringing Device out of Calibration Mode");
                        MoveDeviceOutOfCalibrationMode();

                    } else {

                        // The HTTP Post Body is written just above the Timeout conditions.
                        Serial.println("The HTTP Body for Step ONE for failure for NH3OD has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                        hasSensorCalculationsBegun = false;

                        // Send this Post Body and send it to the Server via HTTPS.
                        FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);
                                            
                        // Check error status of NH3OD
                        // Come out of Calibration Mode if NH3OD calibration has encountered any error.
                        Serial.println("Error encountered for Step ONE for NH3OD.");

                    }

                } else if(CalibStatNH3Step1 == CALIB_STATUS_OK) {

                    // Store it separately in another variable so that it can be used in the next JSON Body.
                    UserInputforNH3ODStep1 = UserEnteredValStep1;
                    // Store the FIRST PPM VALUE safely in a Map.Along with two digit precision if its a float.
                    inMapCalibrationSensorValInfo[NH3_PPM_REF1] = UserInputforNH3ODStep1;
                    // Save the calculations in SPIFFS.
                    writeCalibDoubleValuesToFile(inMapCalibrationSensorValInfo);
                    Serial.printf("First PPM Value (%0.2fPPM) Entered.\r\n", inMapCalibrationSensorValInfo[NH3_PPM_REF1]);

                    // Store it separately in another variable so that it can be used in the next JSON Body.
                    MinRSValforNH3ODFirstPPM = MinRSValforNH3OD;
                    // Store the Minimum RS Value encountered safely in a Map for the FIRST PPM VALUE.We are not using pointers like in old firmware,
                    // So, we will retain the floating point precision just like in SF in MGD because the Map can only store integer value.
                    inMapCalibrationSensorValInfo[VMP_NH3OD_REF1] = MinRSValforNH3ODFirstPPM;
                    strCalibSensorMapStat = writeCalibDoubleValuesToFile(inMapCalibrationSensorValInfo);
                    CheckWriteToCalibrationSensorMapStatus(strCalibSensorMapStat);
                    Serial.printf("NH3OD: RS(%0.2fPPM) %0.4f copied.\r\n", inMapCalibrationSensorValInfo[NH3_PPM_REF1], inMapCalibrationSensorValInfo[VMP_NH3OD_REF1]);
                                        
                    // If SPIFFS has encountered any error while writing then send that error as highest priority.
                    CalibStatNH3Step1 = (strSPIFFSSensorStat != CALIB_STATUS_OK) ? strSPIFFSSensorStat : CalibStatNH3Step1;

                    if((CalibStatNH3Step1 != CALIB_STATUS_OK) && !invokeStepOneAgain) {

                        // This is if SPIFFS encounters any error.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + " }," +
                                                "\"DevcCalibStatus\": {" "\"Step1\": \"" + CalibStatNH3Step1 + "\" }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("SPIFFS error and step not invoked again");

                    } else if(!invokeStepOneAgain) {

                        // Prepare a JSON Body for Firing the API:saveCurrentCalibStepResult along with the encountered Min RS Value for Calib Status 'OK'.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step1RestncVal\": " + MinRSValforNH3ODFirstPPM + " }, " +
                                                "\"DevcCalibStatus\": {" "\"Step1\": \"" + CalibStatNH3Step1 + "\" }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("step not invoked again");

                    } else if((CalibStatNH3Step1 != CALIB_STATUS_OK) && invokeStepOneAgain) {

                        // This is if SPIFFS encounters any error during Recalib of Step ONE keeping Step TWO intact.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                                "\"Step2RestncVal\": " + inMapCalibrationSensorValInfo[VMP_NH3OD_REF2] + " }," +
                                                "\"DevcCalibStatus\": {" 
                                                                    "\"Step1\": \"" + CalibStatNH3Step1 + "\", " +
                                                                    "\"Step2\": \"" + Step2StatInfo + "\" ," +
                                                                    "\"AllStepsDone\": true ," +
                                                                    "\"FinalParamCalibStatus\": false }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("SPIFFS error and step invoked again");

                    } else if(invokeStepOneAgain) {

                        // This JSON Body is fired when Step ONE is invoked again keeping Step TWO Intact.Hence, we need to recalculate PPM again as Step TWO is done previously.
                        // Immediately calculate the NH3OD Final PPM value and insert that in JSON Body too.Retreive the current RS value too.
                        CurrentNH3ODRsValComp = getNH3ODReadings(CurrentNH3ODRsVal);
                        sensor_getParamFromHDC2080(HDCTempVal, HDCHumVal);
                        NH3ODcalPPMRecalibVal = nh3_calculatePPM(CurrentNH3ODRsValComp, HDCTempVal, HDCHumVal);  
                        // Prepare a JSON Body for Firing the API:saveCurrentCalibStepResult along with the encountered Min RS Value for Calib Status 'OK'.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step1RestncVal\": " + inMapCalibrationSensorValInfo[VMP_NH3OD_REF1] + "," +
                                                                "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                                "\"Step2RestncVal\": " + inMapCalibrationSensorValInfo[VMP_NH3OD_REF2] + "," +
                                                                "\"PostCalOutput\": " + NH3ODcalPPMRecalibVal + " }, " +
                                                "\"DevcCalibStatus\": {" +
                                                                    "\"Step1\": \"" + CalibStatNH3Step1 + "\" ," +
                                                                    "\"Step2\": \"" + Step2StatInfo + "\" ," +
                                                                    "\"AllStepsDone\": true ," +
                                                                    "\"FinalParamCalibStatus\": true }, " +
                                                "\"CalibTime\": null "
                                                "}";                      

                        Serial.println("Recalibrated Step ONE again for NH3OD.Sending all final calculations back again to server.Making invokeStepOneAgain flag false too.");

                    }
                                    
                    Serial.println("The HTTP Body for Step ONE for NH3OD has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                    hasSensorCalculationsBegun = false;
                    invokeStepOneAgain = false;
                    
                    // Send this Post Body and send it to the Server via HTTPS.
                    FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);

                } else {

                    Serial.println("Please check the Step ONE Status for NH3OD");
                }
                
            } else if(CalibStepNum == DEVICE_API_CALIB_STATUS_STEP2) {

                float NH3ODRsFirst = inMapCalibrationSensorValInfo[VMP_NH3OD_REF1];

                // Do not calculate again if timeout has occured.
                if((!hasTimeOutOccuredDuringSensorCal) && (!hasTimeOutOccuredAfterSensorCal)) {

                    // Find out the status which the NH3OD has encountered.
                    if(NH3ODStat == CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL) {

                        CalibStatNH3Step2 = CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL;
                        Serial.println("The Status for NH3 Calibration of Second step is:CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL");
                        // Flush the value once detected.
                        NH3ODStat = "";

                    } else if(TGSCaliStat == CALIB_SENSOR_NOT_AVAILABLE) {

                        CalibStatNH3Step2 = CALIB_SENSOR_NOT_AVAILABLE;
                        Serial.println("The Status for NH3 Calibration of Second step is:CALIB_SENSOR_NOT_AVAILABLE");
                        // Flush the value once detected.
                        TGSCaliStat = "";

                    } else {

                        CalibStatNH3Step2 = CALIB_STATUS_OK;
                        Serial.println("The Status for NH3 Calibration of Second step is:CALIB_STATUS_OK");
                    }

                }

                if((CalibStatNH3Step2 != CALIB_STATUS_OK) || hasTimeOutOccuredDuringSensorCal || hasTimeOutOccuredAfterSensorCal) {

                    // Send this ERROR Body Json for timeouts or for any Calibration Error.Check if the last step has 'OK' Status.If not, do not send the PostCalOutput and send the FinalParamCalibStatus as false.

                    // If timeout has occured during the sensor calculation, then substitute here only and send the JSON body.This may not happen as ESP can't multitask.But if timeout is fired first after timer of 5 min is done and hasSensorCalculationsBegun is still true
                    // Then it may happen.This is rarest of rare case scenario. 
                    CalibStatNH3Step2 = (hasTimeOutOccuredDuringSensorCal == true) ? CALIB_TIMEOUT_WAITING_STEP_INPUT : CalibStatNH3Step2;

                    strBodySaveCalibResult = String("{") +
                                            "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                            "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                            "\"MeasuredParam\": \"" + strParamName + "\"," +
                                            "\"CalibValues\": {" +
                                                            "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                            "\"Step1RestncVal\": " + NH3ODRsFirst + "," +
                                                            "\"Step2Inp\": " + UserEnteredValStep2 + " }," +
                                            "\"DevcCalibStatus\": {" +
                                                                "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                                "\"Step2\": \"" + CalibStatNH3Step2 + "\" ," +
                                                                "\"AllStepsDone\": true ," +
                                                                "\"FinalParamCalibStatus\": false }, " +
                                            "\"CalibTime\": null "
                                            "}";
            
                     if(hasTimeOutOccuredDuringSensorCal) {

                        Serial.println("Timeout has occured while executing NH3OD Step TWO.Firing TimeOut API");
                        // If the timeout has occured during the calibration process, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.
                        FireSaveCalibErrorToServer(strBodySaveCalibResult);                            
                    
                    } else if(hasTimeOutOccuredAfterSensorCal) {

                        Serial.println("Timeout has occured after executing NH3OD Step TWO.Bringing Device out of Calibration Mode");
                        MoveDeviceOutOfCalibrationMode();

                    } else {

                        // The HTTP Post Body is written just above the Timeout conditions.
                        Serial.println("The HTTP Body for Step TWO failure for NH3OD has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                        hasSensorCalculationsBegun = false;

                        // Send this Post Body and send it to the Server via HTTPS.
                        FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);
                        
                        // Check error status of NH3OD
                        // Come out of Calibration Mode if NH3OD calibration has encountered any error.
                        Serial.println("Error encountered for Step TWO for NH3OD.");

                    }

                } else if(CalibStatNH3Step2 == CALIB_STATUS_OK) {

                    // Store it separately in another variable so that it can be used in the next JSON Body.
                    UserInputforNH3ODStep2 = UserEnteredValStep2;
                    // Store the SECOND PPM VALUE safely in a Map.
                    inMapCalibrationSensorValInfo[NH3_PPM_REF2] = UserInputforNH3ODStep2;
                    // Save the calculations in SPIFFS.
                    writeCalibDoubleValuesToFile(inMapCalibrationSensorValInfo);
                    Serial.printf("Second PPM Value (%0.2fPPM) Entered.\r\n", inMapCalibrationSensorValInfo[NH3_PPM_REF2]);

                    // Store it separately in another variable so that it can be used in the next JSON Body.
                    MinRSValforNH3ODSecondPPM = MinRSValforNH3OD;
                    // Store the Minimum RS Value encountered safely in a Map for the SECOND PPM VALUE.We are not using pointers like in old firmware,
                    // So, we will retain the floating point precision just like in SF in MGD because the Map can only store integer value.
                    inMapCalibrationSensorValInfo[VMP_NH3OD_REF2] = MinRSValforNH3ODSecondPPM;
                    strCalibSensorMapStat = writeCalibDoubleValuesToFile(inMapCalibrationSensorValInfo);
                    CheckWriteToCalibrationSensorMapStatus(strCalibSensorMapStat);
                    Serial.printf("NH3OD: RS(%0.2fPPM) %0.4f copied.\r\n", inMapCalibrationSensorValInfo[NH3_PPM_REF2], inMapCalibrationSensorValInfo[VMP_NH3OD_REF2]);

                    // Immediately calculate the NH3OD Final PPM value and insert that in JSON Body too.Retreive the current RS value too.
                    CurrentNH3ODRsValComp = getNH3ODReadings(CurrentNH3ODRsVal);
                    sensor_getParamFromHDC2080(HDCTempVal, HDCHumVal);
                    NH3ODcalculatedPPMVal = nh3_calculatePPM(CurrentNH3ODRsValComp, HDCTempVal, HDCHumVal);

                    // If SPIFFS has encountered any error while writing then send that error as highest priority.
                    CalibStatNH3Step2 = (strSPIFFSSensorStat != CALIB_STATUS_OK) ? strSPIFFSSensorStat : CalibStatNH3Step2;

                    if(CalibStatNH3Step2 != CALIB_STATUS_OK) {

                        // This is if SPIFFS encounters any error.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step1RestncVal\": " + NH3ODRsFirst + "," +
                                                                "\"Step2Inp\": " + UserEnteredValStep2 + " }," +
                                                "\"DevcCalibStatus\": {" +
                                                                    "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                                    "\"Step2\": \"" + CalibStatNH3Step2 + "\" ," +
                                                                    "\"AllStepsDone\": true ," +
                                                                    "\"FinalParamCalibStatus\": false }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("The HTTP Body for Step TWO for NH3OD(SPIFFS ERROR) has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");

                    } else {

                        // Prepare a JSON Body for Firing the API:saveCurrentCalibStepResult along with the encountered Min RS Value for Calib Status 'OK'.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step1RestncVal\": " + NH3ODRsFirst + "," +
                                                                "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                                "\"Step2RestncVal\": " + MinRSValforNH3ODSecondPPM + "," +
                                                                "\"PostCalOutput\": " + NH3ODcalculatedPPMVal + " }, " +
                                                "\"DevcCalibStatus\": {" +
                                                                    "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                                    "\"Step2\": \"" + CalibStatNH3Step2 + "\" ," +
                                                                    "\"AllStepsDone\": true ," +
                                                                    "\"FinalParamCalibStatus\": true }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("The HTTP Body for Step TWO for NH3OD has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                    }
                                                        
                    hasSensorCalculationsBegun = false;

                    // Send this Post Body and send it to the Server via HTTPS.
                    FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);

                } else {

                    Serial.println("Please check the Step TWO Status for NH3OD");
                }

            } else {

                Serial.println("Undefined Step for NH3OD Param");
            }                            

        } else if(objTGSWaitCalibDelay.isTimeOut()) {

            Serial.println("3 minutes are done, starting to store NH3OD values");
            if(objTGSReadDelay.isTimeOut()) {
                
                objTGSReadDelay.setDelay(TGS_READ_DELAY);
                // If 4 minutes are not done keep on executing the below func to get the Lowest Min Avg RS for NH3OD.
                CalculateLowestAvgRSforTGSSensors(VMP_NH3OD);

            }
                    
        }

    } else if(strParamName == VMP_ETHANOL) {

        // Do not send any other API while calibration of 2 step gas is going ON.
        BlockCaliAPIfor2StepGas = true;
        Serial.println("Blocking all APIs for Calibration for getting Min Avg Val of Ethanol.");

        // Retreive the minimum Rs value of ETHNL only after 4 minutes timeout or fire the API after 30 minute timeout has occured.
        if(objTGSStartCalibDelay.isTimeOut()) {

            // Retreive the Min Val.
            MinRSValforETHNL = objRetrieveMinVal.RetrieveAvgVal(SENSOR_2603);
            Serial.printf("The Min ETHNL value encountered is: %0.4f\r\n",MinRSValforETHNL);

            // Also clear all contents of the Array and make the counter back to zero again.
            objRetrieveMinVal.ClearContentsOfArray(SENSOR_2603);

            // Timeout of 4 minutes is done, make the BlockCaliAPIfor2StepGas flag as false, so that after saveCurrentCalibStepResult, Calib 
            // Func will be executed as usual.
            BlockCaliAPIfor2StepGas = false;

            if(CalibStepNum == DEVICE_API_CALIB_STATUS_STEP1) {

                // Do not calculate again if timeout has occured.
                if((!hasTimeOutOccuredDuringSensorCal) && (!hasTimeOutOccuredAfterSensorCal)) {

                    // Find out the status which the ETHNL has encountered.
                    if(EthanolStat == CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL) {

                        CalibStatETHStep1 = CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL;
                        Serial.println("The Status for ETHNL Calibration of first step is:CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL");
                        // Flush the value once detected.
                        EthanolStat = "";

                    } else if(TGSCaliStat == CALIB_SENSOR_NOT_AVAILABLE) {

                        CalibStatETHStep1 = CALIB_SENSOR_NOT_AVAILABLE;
                        Serial.println("The Status for ETHNL Calibration of first step is:CALIB_SENSOR_NOT_AVAILABLE");
                        // Flush the value once detected.
                        TGSCaliStat = "";

                    } else {

                        CalibStatETHStep1 = CALIB_STATUS_OK;
                        Serial.println("The Status for ETHNL Calibration of first step is:CALIB_STATUS_OK");
                    }

                }


                if((CalibStatETHStep1 != CALIB_STATUS_OK) || hasTimeOutOccuredDuringSensorCal || hasTimeOutOccuredAfterSensorCal) {

                    // Send this ERROR Body Json for timeouts or for any Calibration Error.Check if the last step has 'OK' Status.

                    // If timeout has occured during the sensor calculation, then substitute here only and send the JSON body.This may not happen as ESP can't multitask.But if timeout is fired first after timer of 5 min is done and hasSensorCalculationsBegun is still true
                    // Then it may happen.This is rarest of rare case scenario.
                    CalibStatETHStep1 = (hasTimeOutOccuredDuringSensorCal == true) ? CALIB_TIMEOUT_WAITING_STEP_INPUT : CalibStatETHStep1;

                    if(!invokeStepOneAgain) {

                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + " }," +
                                                "\"DevcCalibStatus\": {" "\"Step1\": \"" + CalibStatETHStep1 + "\" }, " +
                                                "\"CalibTime\": null "
                                                "}";
                    } else {

                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                                "\"Step2RestncVal\": " + inMapCalibrationSensorValInfo[VMP_ETHANOL_REF2] + " }," +
                                                "\"DevcCalibStatus\": {" 
                                                                    "\"Step1\": \"" + CalibStatETHStep1 + "\", " +
                                                                    "\"Step2\": \"" + Step2StatInfo + "\" ," +
                                                                    "\"AllStepsDone\": true ," +
                                                                    "\"FinalParamCalibStatus\": false }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("Step ONE is invoked again for ETHNL keeping Step TWO intact.Picking appropriate JSON Body for error condition.Making the invokeStepOneAgain flag false.");
                        invokeStepOneAgain = false;
                    }
                    
                    if(hasTimeOutOccuredDuringSensorCal) {

                        Serial.println("Timeout has occured while executing ETHNL Step ONE.Firing TimeOut API");
                        // If the timeout has occured during the calibration process, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.
                        FireSaveCalibErrorToServer(strBodySaveCalibResult);                            
                    
                    } else if(hasTimeOutOccuredAfterSensorCal) {

                        Serial.println("Timeout has occured after executing ETHNL Step ONE.Bringing Device out of Calibration Mode");
                        MoveDeviceOutOfCalibrationMode();

                    } else {

                        // The HTTP Post Body is written just above the Timeout conditions.
                        Serial.println("The HTTP Body for Step ONE for failure for ETHNL has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                        hasSensorCalculationsBegun = false;

                        // Send this Post Body and send it to the Server via HTTPS.
                        FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);
                                            
                        // Check error status of ETHNL.
                        // Come out of Calibration Mode if ETHNL calibration has encountered any error.
                        Serial.println("Error encountered for Step ONE for ETHNL.");

                    }

                } else if(CalibStatETHStep1 == CALIB_STATUS_OK) {

                    // Store it separately in another variable so that it can be used in the next JSON Body.
                    UserInputforETHNLStep1 = UserEnteredValStep1;
                    // Store the FIRST PPM VALUE safely in a Map.Along with two digit precision if its a float.
                    inMapCalibrationSensorValInfo[ETHANOL_PPM_REF1] = UserInputforETHNLStep1;
                    // Save the calculations in SPIFFS.
                    writeCalibDoubleValuesToFile(inMapCalibrationSensorValInfo);
                    Serial.printf("First PPM Value (%0.2fPPM) Entered.\r\n", inMapCalibrationSensorValInfo[ETHANOL_PPM_REF1]);

                    // Store it separately in another variable so that it can be used in the next JSON Body.
                    MinRSValforETHNLFirstPPM = MinRSValforETHNL;
                    // Store the Minimum RS Value encountered safely in a Map for the FIRST PPM VALUE.We are not using pointers like in old firmware,
                    // So, we will retain the floating point precision just like in SF in MGD because the Map can only store integer value.
                    inMapCalibrationSensorValInfo[VMP_ETHANOL_REF1] = MinRSValforETHNLFirstPPM;
                    strCalibSensorMapStat = writeCalibDoubleValuesToFile(inMapCalibrationSensorValInfo);
                    CheckWriteToCalibrationSensorMapStatus(strCalibSensorMapStat);
                    Serial.printf("ETHNL: RS(%0.2fPPM) %0.4f copied.\r\n", inMapCalibrationSensorValInfo[ETHANOL_PPM_REF1], inMapCalibrationSensorValInfo[VMP_ETHANOL_REF1]);
                                        
                    // If SPIFFS has encountered any error while writing then send that error as highest priority.
                    CalibStatETHStep1 = (strSPIFFSSensorStat != CALIB_STATUS_OK) ? strSPIFFSSensorStat : CalibStatETHStep1;

                    if((CalibStatETHStep1 != CALIB_STATUS_OK) && !invokeStepOneAgain) {

                        // This is if SPIFFS encounters any error.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + " }," +
                                                "\"DevcCalibStatus\": {" "\"Step1\": \"" + CalibStatETHStep1 + "\" }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("SPIFFS error and step not invoked again");

                    } else if(!invokeStepOneAgain) {

                        // Prepare a JSON Body for Firing the API:saveCurrentCalibStepResult along with the encountered Min RS Value for Calib Status 'OK'.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step1RestncVal\": " + MinRSValforETHNLFirstPPM + " }, " +
                                                "\"DevcCalibStatus\": {" "\"Step1\": \"" + CalibStatETHStep1 + "\" }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("step not invoked again");

                    } else if((CalibStatETHStep1 != CALIB_STATUS_OK) && invokeStepOneAgain) {

                        // This is if SPIFFS encounters any error during Recalib of Step ONE keeping Step TWO intact.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                                "\"Step2RestncVal\": " + inMapCalibrationSensorValInfo[VMP_ETHANOL_REF2] + " }," +
                                                "\"DevcCalibStatus\": {" 
                                                                    "\"Step1\": \"" + CalibStatETHStep1 + "\", " +
                                                                    "\"Step2\": \"" + Step2StatInfo + "\" ," +
                                                                    "\"AllStepsDone\": true ," +
                                                                    "\"FinalParamCalibStatus\": false }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("SPIFFS error and step invoked again");

                    } else if(invokeStepOneAgain) {

                        // This JSON Body is fired when Step ONE is invoked again keeping Step TWO Intact.Hence, we need to recalculate PPM again as Step TWO is done previously.
                        // Immediately calculate the ETHNL Final PPM value and insert that in JSON Body too. Retreive the current RS value too.
                        CurrentETHNLRsValComp = getEthanolReadings(CurrentETHNLRsVal);
                        sensor_getParamFromHDC2080(HDCTempVal, HDCHumVal);
                        ETHNLcalPPMRecalibVal = Ethanol_calculatePPM(CurrentETHNLRsValComp, HDCTempVal, HDCHumVal);  
                        // Prepare a JSON Body for Firing the API:saveCurrentCalibStepResult along with the encountered Min RS Value for Calib Status 'OK'.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step1RestncVal\": " + inMapCalibrationSensorValInfo[VMP_ETHANOL_REF1] + "," +
                                                                "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                                "\"Step2RestncVal\": " + inMapCalibrationSensorValInfo[VMP_ETHANOL_REF2] + "," +
                                                                "\"PostCalOutput\": " + ETHNLcalPPMRecalibVal + " }, " +
                                                "\"DevcCalibStatus\": {" +
                                                                    "\"Step1\": \"" + CalibStatETHStep1 + "\" ," +
                                                                    "\"Step2\": \"" + Step2StatInfo + "\" ," +
                                                                    "\"AllStepsDone\": true ," +
                                                                    "\"FinalParamCalibStatus\": true }, " +
                                                "\"CalibTime\": null "
                                                "}";                      

                        Serial.println("Recalibrated Step ONE again for ETHNL.Sending all final calculations back again to server.Making invokeStepOneAgain flag false too.");

                    }
                                    
                    Serial.println("The HTTP Body for Step ONE for ETHNL has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                    hasSensorCalculationsBegun = false;
                    invokeStepOneAgain = false;
                    
                    // Send this Post Body and send it to the Server via HTTPS.
                    FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);

                } else {

                    Serial.println("Please check the Step ONE Status for ETHNL");
                }

            } else if(CalibStepNum == DEVICE_API_CALIB_STATUS_STEP2) {

                float ETHNLRsFirst = inMapCalibrationSensorValInfo[VMP_ETHANOL_REF1];

                // Do not calculate again if timeout has occured.
                if((!hasTimeOutOccuredDuringSensorCal) && (!hasTimeOutOccuredAfterSensorCal)) {

                    // Find out the status which the ETHNL has encountered.
                    if(EthanolStat == CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL) {

                        CalibStatETHStep2 = CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL;
                        Serial.println("The Status for ETHNL Calibration of Second step is:CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL");
                        // Flush the value once detected.
                        EthanolStat = "";

                    } else if(TGSCaliStat == CALIB_SENSOR_NOT_AVAILABLE) {

                        CalibStatETHStep2 = CALIB_SENSOR_NOT_AVAILABLE;
                        Serial.println("The Status for ETHNL Calibration of Second step is:CALIB_SENSOR_NOT_AVAILABLE");
                        // Flush the value once detected.
                        TGSCaliStat = "";

                    } else {

                        CalibStatETHStep2 = CALIB_STATUS_OK;
                        Serial.println("The Status for ETHNL Calibration of Second step is:CALIB_STATUS_OK");
                    }

                }


                if((CalibStatETHStep2 != CALIB_STATUS_OK) || hasTimeOutOccuredDuringSensorCal || hasTimeOutOccuredAfterSensorCal) {

                    // Send this ERROR Body Json for timeouts or for any Calibration Error.Check if the last step has 'OK' Status.If not, do not send the PostCalOutput and send the FinalParamCalibStatus as false.

                    // If timeout has occured during the sensor calculation, then substitute here only and send the JSON body.This may not happen as ESP can't multitask.But if timeout is fired first after timer of 5 min is done and hasSensorCalculationsBegun is still true
                    // Then it may happen.This is rarest of rare case scenario. 
                    CalibStatETHStep2 = (hasTimeOutOccuredDuringSensorCal == true) ? CALIB_TIMEOUT_WAITING_STEP_INPUT : CalibStatETHStep2;

                    strBodySaveCalibResult = String("{") +
                                            "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                            "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                            "\"MeasuredParam\": \"" + strParamName + "\"," +
                                            "\"CalibValues\": {" +
                                                            "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                            "\"Step1RestncVal\": " + ETHNLRsFirst + "," +
                                                            "\"Step2Inp\": " + UserEnteredValStep2 + " }," +
                                            "\"DevcCalibStatus\": {" +
                                                                "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                                "\"Step2\": \"" + CalibStatETHStep2 + "\" ," +
                                                                "\"AllStepsDone\": true ," +
                                                                "\"FinalParamCalibStatus\": false }, " +
                                            "\"CalibTime\": null "
                                            "}";
            
                     if(hasTimeOutOccuredDuringSensorCal) {

                        Serial.println("Timeout has occured while executing ETHNL Step TWO.Firing TimeOut API");
                        // If the timeout has occured during the calibration process, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.
                        FireSaveCalibErrorToServer(strBodySaveCalibResult);                            
                    
                    } else if(hasTimeOutOccuredAfterSensorCal) {

                        Serial.println("Timeout has occured after executing ETHNL Step TWO.Bringing Device out of Calibration Mode");
                        MoveDeviceOutOfCalibrationMode();

                    } else {

                        // The HTTP Post Body is written just above the Timeout conditions.
                        Serial.println("The HTTP Body for Step TWO failure for ETHNL has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                        hasSensorCalculationsBegun = false;

                        // Send this Post Body and send it to the Server via HTTPS.
                        FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);
                        
                        // Check error status of ETHNL
                        // Come out of Calibration Mode if ETHNL calibration has encountered any error.
                        Serial.println("Error encountered for Step TWO for ETHNL.");

                    }

                } else if(CalibStatETHStep2 == CALIB_STATUS_OK) {

                    // Store it separately in another variable so that it can be used in the next JSON Body.
                    UserInputforETHNLStep2 = UserEnteredValStep2;
                    // Store the SECOND PPM VALUE safely in a Map.
                    inMapCalibrationSensorValInfo[ETHANOL_PPM_REF2] = UserInputforETHNLStep2;
                    // Save the calculations in SPIFFS.
                    writeCalibDoubleValuesToFile(inMapCalibrationSensorValInfo);
                    Serial.printf("Second PPM Value (%0.2fPPM) Entered.\r\n", inMapCalibrationSensorValInfo[ETHANOL_PPM_REF2]);

                    // Store it separately in another variable so that it can be used in the next JSON Body.
                    MinRSValforETHNLSecondPPM = MinRSValforETHNL;
                    // Store the Minimum RS Value encountered safely in a Map for the SECOND PPM VALUE.We are not using pointers like in old firmware,
                    // So, we will retain the floating point precision just like in SF in MGD because the Map can only store integer value.
                    inMapCalibrationSensorValInfo[VMP_ETHANOL_REF2] = MinRSValforETHNLSecondPPM;
                    strCalibSensorMapStat = writeCalibDoubleValuesToFile(inMapCalibrationSensorValInfo);
                    CheckWriteToCalibrationSensorMapStatus(strCalibSensorMapStat);
                    Serial.printf("ETHNL: RS(%0.2fPPM) %0.4f copied.\r\n", inMapCalibrationSensorValInfo[ETHANOL_PPM_REF2], inMapCalibrationSensorValInfo[VMP_ETHANOL_REF2]);

                    // Immediately calculate the ETHNL Final PPM value and insert that in JSON Body too.Retreive the current RS value too.
                    CurrentETHNLRsValComp = getEthanolReadings(CurrentETHNLRsVal);
                    sensor_getParamFromHDC2080(HDCTempVal, HDCHumVal);
                    ETHNLcalculatedPPMVal = Ethanol_calculatePPM(CurrentETHNLRsValComp, HDCTempVal, HDCHumVal);

                    // If SPIFFS has encountered any error while writing then send that error as highest priority.
                    CalibStatETHStep2 = (strSPIFFSSensorStat != CALIB_STATUS_OK) ? strSPIFFSSensorStat : CalibStatETHStep2;

                    if(CalibStatETHStep2 != CALIB_STATUS_OK) {

                        // This is if SPIFFS encounters any error.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step1RestncVal\": " + ETHNLRsFirst + "," +
                                                                "\"Step2Inp\": " + UserEnteredValStep2 + " }," +
                                                "\"DevcCalibStatus\": {" +
                                                                    "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                                    "\"Step2\": \"" + CalibStatETHStep2 + "\" ," +
                                                                    "\"AllStepsDone\": true ," +
                                                                    "\"FinalParamCalibStatus\": false }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("The HTTP Body for Step TWO for ETHNL(SPIFFS ERROR) has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");

                    } else {

                        // Prepare a JSON Body for Firing the API:saveCurrentCalibStepResult along with the encountered Min RS Value for Calib Status 'OK'.
                        strBodySaveCalibResult = String("{") +
                                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                                "\"MeasuredParam\": \"" + strParamName + "\"," +
                                                "\"CalibValues\": {" +
                                                                "\"Step1Inp\": " + UserEnteredValStep1 + "," +
                                                                "\"Step1RestncVal\": " + ETHNLRsFirst + "," +
                                                                "\"Step2Inp\": " + UserEnteredValStep2 + "," +
                                                                "\"Step2RestncVal\": " + MinRSValforETHNLSecondPPM + "," +
                                                                "\"PostCalOutput\": " + ETHNLcalculatedPPMVal + " }, " +
                                                "\"DevcCalibStatus\": {" +
                                                                    "\"Step1\": \"" + Step1StatInfo + "\" ," +
                                                                    "\"Step2\": \"" + CalibStatETHStep2 + "\" ," +
                                                                    "\"AllStepsDone\": true ," +
                                                                    "\"FinalParamCalibStatus\": true }, " +
                                                "\"CalibTime\": null "
                                                "}";

                        Serial.println("The HTTP Body for Step TWO for ETHNL has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");
                    }
                                                        
                    hasSensorCalculationsBegun = false;

                    // Send this Post Body and send it to the Server via HTTPS.
                    FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);

                } else {

                    Serial.println("Please check the Step TWO Status for ETHNL");
                }

            } else {

                Serial.println("Undefined Step for ETHNL Param");
            }

        } else if(objTGSWaitCalibDelay.isTimeOut()) {

            Serial.println("3 minutes are done, starting to store ETHNL values");
            if(objTGSReadDelay.isTimeOut()) {
                
                objTGSReadDelay.setDelay(TGS_READ_DELAY);
                // If 4 minutes are not done keep on executing the below func to get the Lowest Min Avg RS for ETHNL.
                CalculateLowestAvgRSforTGSSensors(VMP_ETHANOL);

            }
                    
        }

    } else if(strParamName == VMP_RTC) {

        // Refer TEMP Step 3 for guidance.
        // Do not calculate again if timeout has occured.
        if((!hasTimeOutOccuredDuringSensorCal) && (!hasTimeOutOccuredAfterSensorCal)) {
    
            int num = RTCUnixTimeStamp.length();
            // Declaring character array 
            char Unixchar[num + 1]; 
            // Copying the contents of the string to char array 
            strcpy(Unixchar, RTCUnixTimeStamp.c_str()); 
            setRTCTimeFromUNIX(Unixchar);
            // Check if this function gives time immediately.
            strCalibResultStat = ReadRTCTime(RtcCalibTimeDate, RtcCalibDate, RtcCalibTime);
            Serial.println("The Time after setting RTC is:");
            Serial.println(RtcCalibTimeDate);
            PrintRtcTime(RtcCalibTimeDate, RTCSetUNIXTime);
            Serial.println("The UNIX TimeStamp after setting RTC is:");
            Serial.println(RTCSetUNIXTime);

            if(strCalibResultStat == RTC_DATE_TIME_VALID) {

                Serial.println("Proper time set in RTC");
                strCalibResultStat = CALIB_STATUS_OK;

            } else if(strCalibResultStat == RTC_DATE_TIME_INVALID) {

                Serial.println("Garbage Time set in RTC");
                strCalibResultStat = CALIB_STATUS_ERROR_SETTING_RTC;
            }

        }

        if(hasTimeOutOccuredDuringSensorCal || hasTimeOutOccuredAfterSensorCal) {

            // If timeout has occured during the RTC calculation, then substitute here only and send the JSON body.
            strCalibResultStat = (hasTimeOutOccuredDuringSensorCal == true) ? CALIB_TIMEOUT_WAITING_STEP_INPUT : strCalibResultStat;

            // Send this ERROR Body Json for timeouts or for any Calibration Error.Check if the last step has 'OK' Status.If not, do not send the PostCalOutput and send the FinalParamCalibStatus as false.
            // UserEnteredValStep1 won't be from API cause only param will be provided.This will be of getcurrentUTCdatetime API.
            strBodySaveCalibResult = String("{") +
                                    "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                    "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                    "\"MeasuredParam\": \"" + strParamName + "\"," +
                                    "\"CalibValues\": {" +
                                                    "\"Step1Inp\": \"" + RTCUnixTimeStamp + "\"," +
                                                    "\"SrvrCurrentUTCDtTm\": \"" + UTCDateTime + "\" }," +
                                    "\"DevcCalibStatus\": {" +
                                                        "\"Step1\": \"" + strCalibResultStat + "\" ," +
                                                        "\"AllStepsDone\": true ," +
                                                        "\"FinalParamCalibStatus\": false }, " +
                                    "\"CalibTime\": null "
                                    "}";

            if(hasTimeOutOccuredDuringSensorCal) {

                Serial.println("Timeout has occured while executing RTC Step.Firing TimeOut API");
                // If the timeout has occured during the calibration process, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.
                FireSaveCalibErrorToServer(strBodySaveCalibResult);                            
            
            } else if(hasTimeOutOccuredAfterSensorCal) {

                Serial.println("Timeout has occured after executing RTC Step ONE.Bringing Device out of Calibration Mode.");
                MoveDeviceOutOfCalibrationMode();

            } 

        } else if((strCalibResultStat != CALIB_STATUS_OK) || (strCalibResultStat == CALIB_STATUS_OK)) {

            // We need to check the error time too so send everything to server if we get any error.
            if(strCalibResultStat != CALIB_STATUS_OK) {

                strBodySaveCalibResult = String("{") +
                                        "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                        "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                        "\"MeasuredParam\": \"" + strParamName + "\"," +
                                        "\"CalibValues\": {" +
                                                        "\"Step1Inp\": \"" + RTCUnixTimeStamp + "\"," +
                                                        "\"SrvrCurrentUTCDtTm\": \"" + UTCDateTime + "\"," + 
                                                        "\"PostCalOutput\": \"" + RTCSetUNIXTime + "\"," +
                                                        "\"DevcCurrentUTCDtTm\": \"" + RtcCalibTimeDate + "\" }, " +
                                        "\"DevcCalibStatus\": {" +
                                                            "\"Step1\": \"" + strCalibResultStat + "\" ," +
                                                            "\"AllStepsDone\": true ," +
                                                            "\"FinalParamCalibStatus\": false }, " +
                                        "\"CalibTime\": null "
                                        "}";

                Serial.println("The HTTP Body for Step ONE failure for RTC has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");

            } else if(strCalibResultStat == CALIB_STATUS_OK) {

                strBodySaveCalibResult = String("{") +
                                        "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                        "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                        "\"MeasuredParam\": \"" + strParamName + "\"," +
                                        "\"CalibValues\": {" +
                                                        "\"Step1Inp\": \"" + RTCUnixTimeStamp + "\"," +
                                                        "\"SrvrCurrentUTCDtTm\": \"" + UTCDateTime + "\"," + 
                                                        "\"PostCalOutput\": \"" + RTCSetUNIXTime + "\"," +
                                                        "\"DevcCurrentUTCDtTm\": \"" + RtcCalibTimeDate + "\" }, " +
                                        "\"DevcCalibStatus\": {" +
                                                            "\"Step1\": \"" + strCalibResultStat + "\" ," +
                                                            "\"AllStepsDone\": true ," +
                                                            "\"FinalParamCalibStatus\": true }, " +
                                        "\"CalibTime\": null "
                                        "}";

                Serial.println("The HTTP Body for Step ONE for RTC has been formed.Firing the API:saveCurrentCalibStepResult.Making the hasSensorCalculationsBegun flag false.");

            }

            hasSensorCalculationsBegun = false;
            // Send this Post Body and send it to the Server via HTTPS.
            FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);

        } else {

            Serial.println("Please check the Step ONE Status for RTC");
        }

    } else if(strParamName == "") {

        Serial.println("Encountered an Empty Param Name.Please Check");

        // Suppose the timeout has occured but the user has not invoked anything uptil 30 minutes and its a first time calibration for a new PCB,
        // Then the Param Name will be empty string.So check for that condition.
        if(hasTimeOutOccuredAfterSensorCal) {
                
            strBodyDevcCalibError = String("{") +
                                    "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\"," +
                                    "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\"," +
                                    "\"MeasuredParam\": 'NOPARAM', " +
                                    "\"CalibValues\": {""}, " +
                                    "\"DevcCalibStatus\": {" +
                                                        "\"StepStatus\": \"" + TimeOutStat + "\" }, " +
                                    "\"CalibTime\": null "
                                    "}";

            Serial.println("Timeout has occured before executing any of the Step.Firing TimeOut API");
            // If the timeout has occured before starting any of the calibration step, then change the current status to TIMEOUT_WAITING_STEP_INPUT and fire the API.                    
            FireSaveCalibErrorToServer(strBodyDevcCalibError);
        } else {

            Serial.println("Timeout has not occured before executing any of the Step");
        }

    } else {

        Serial.println("Unable to get MeasuredParamName for Calibration. Unknown Param Type: " + strParamName);
    }
    
    return 0.0f;
}

void CalculateLowestAvgRSforTGSSensors(String ParamSensorType) {

    referenceRS = sensor_readR0ofTGS(ParamSensorType);

    if(referenceRS > 0) {

        newRefR0 = referenceRS;

        // Store the current RS Value in a variable.
        Serial.printf("New RS @ 20dC & 65%%: %0.4f\r\n", newRefR0);                                

        if(ParamSensorType.equals(VMP_NH3OD)) {

            // Store the incoming values safely in an Array.
            objRetrieveMinVal.StoreAvgVal(NH3OD_SENSOR, newRefR0);

        } else if(ParamSensorType.equals(VMP_ETHANOL)) {

            // Store the incoming values safely in an Array.
            objRetrieveMinVal.StoreAvgVal(SENSOR_2603, newRefR0);
        }

    } else {

        Serial.println("Sorry TGS2602/TGS2603 not ready. Try after some time.");
        TGSCaliStat = CALIB_SENSOR_NOT_AVAILABLE;
    }

}

void CheckWriteToCalibrationMapStatus(String strStat) {

    if(strStat == STATUS_SUCCESS) {

        strSPIFFSStat = CALIB_STATUS_OK;
        Serial.println("Successfully written to SPIFFS");
    } else {

        strSPIFFSStat = CALIB_STATUS_SPIFFS_ERROR;
        Serial.println("Some error encountered while writing to SPIFFS");
        Serial.println(strStat);
    }

}

void CheckWriteToCalibrationSensorMapStatus(String strStat) {

    if(strStat == STATUS_SUCCESS) {

        strSPIFFSSensorStat = CALIB_STATUS_OK;
        Serial.println("Successfully written to SPIFFS");
    } else {

        strSPIFFSSensorStat = CALIB_STATUS_SPIFFS_ERROR;
        Serial.println("Some error encountered while writing to SPIFFS");
        Serial.println(strStat);
    }

}

void FireSaveCalibErrorToServer(String strPostBodyForTimeout) {

    strSaveTimeOutError = SendSaveCalibErrorToServer(mapConnectionInfo, mapDeviceInfo, strPostBodyForTimeout);
    if(strSaveTimeOutError == STATUS_SUCCESS) {

        Serial.println("Received SUCCESS for API:saveDeviceCalibError");
    } else {

        // Suppose there is internet issue too, bring the device out of calibration mode.
        Serial.println("Could not receive 'SUCCESS' for API:saveDeviceCalibError");
        Serial.println(strSaveTimeOutError);
    }

    // Move device out of calibration Mode after timeout has occured.
    MoveDeviceOutOfCalibrationMode();

}

void FiresaveCurrentCalibStepResultStat(String strPostBodyCalibStep) {

    strSaveCalibStepRes = SaveCalibrationStepResult(mapConnectionInfo, mapDeviceInfo, strPostBodyCalibStep);
    if(strSaveCalibStepRes == STATUS_SUCCESS) {

        Serial.println("Received SUCCESS for API:saveCurrentCalibStepResult");
        // Since it's trying again and again for 5 times it may try for 30 sec untill timeout occurs for http fail.So keep LED colour as YELLOW so we get an idea about it.And then when its successful maybe within those 5 tries then turn
        // The LED COLOUR as RASPBERRY again.Suppose its unsuccessful even after 5 tries then device comes out of Calib Mode and LED turns GREEN again.
        setSpecifiedIndicatorColorForLED(LED_COLOR_RASPBERRY);
        // TODO: Need to check whether to come out of calibration mode after every step.NO
        CalibCount = 0;
    } else {

        // TODO: Need to check whether to try to hit server again, because if API is not sucessful and if getCalibrationStepInfo 
        // Is invoked again, then sensor calculations will begin again which will cause any issue.DONE.
        Serial.println("Could not receive 'SUCCESS' for API:saveCurrentCalibStepResult");
        Serial.println(strSaveCalibStepRes);

        CalibCount ++;
        Serial.printf("CalibCount is %d\r\n", CalibCount);
        if(CalibCount >= 6) {

            // Still not successful, then come out of Cali Mode.
            Serial.println("Max 5 tries are done.Bringing device out of calibration mode");
            CalibCount = 0;
            MoveDeviceOutOfCalibrationMode();
        } else {

            // Since it's trying again and again for 5 times it may try for 30 sec untill timeout occurs for http fail.So keep LED colour as YELLOW so we get an idea about it.
            setSpecifiedIndicatorColorForLED(LED_COLOR_YELLOW);
            // Function will call itself again and again.This is function recursion.But try only for 5 times.Still not successful, then come out of Cali Mode.
            FiresaveCurrentCalibStepResultStat(strBodySaveCalibResult);
        }

    }

}

void MoveDeviceOutOfCalibrationMode() {

    Serial.println("Moving the Device out of the Calibration Mode.Making all Flags false");

    // Since it's out of Calib mode, keep the LED Colour as green.
    setSpecifiedIndicatorColorForLED(LED_COLOR_GREEN);

    // All Eight flags for calibration purpose are made false.
    isDevcinCalibrationMode = false;
    isAckforcalibSent = false;
    invokegetCalibStepInfoAgain = false;
    BlockCaliAPIfor2StepGas = false;
    invokeStepOneAgain = false;
    // Flags related to timeouts.
    hasSensorCalculationsBegun = false;
    hasTimeOutOccuredDuringSensorCal = false;
    hasTimeOutOccuredAfterSensorCal = false;

}