#ifndef __V_CALIBRATIONSENSORCALCULATION_H__
#define __V_CALIBRATIONSENSORCALCULATION_H__

#include <Arduino.h>
#include <map>
#include "VSensors.h"

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
                            float PostCalibCalcValue);

void CalculateLowestAvgRSforTGSSensors(String ParamSensorType);
void CheckWriteToCalibrationMapStatus(String strStat);
void CheckWriteToCalibrationSensorMapStatus(String strStat);
void FireSaveCalibErrorToServer(String strPostBodyForTimeout);
void FiresaveCurrentCalibStepResultStat(String strPostBodyCalibStep);

void MoveDeviceOutOfCalibrationMode();


class SENSOR_RETRIEVE_MIN_VAL
{
protected:                          // Access Specifiers.            
    float NH3AvgvalArray[100];
    float NH3AvgVal;                // Data members.
    float NH3AvgValue;
    uint8_t NH3AvgCount;

    float EthAvgvalArray[100];
    float EthAvgVal;                // Data members.
    float EthAvgValue;
    uint8_t EthAvgCount;

public:
   SENSOR_RETRIEVE_MIN_VAL(){       // Constructor.
       NH3AvgVal   = 0;
       NH3AvgValue = 0;
       NH3AvgCount = 0;

       EthAvgVal   = 0;
       EthAvgValue = 0;
       EthAvgCount = 0;

    }
    
    void StoreAvgVal(uint8_t ParamName, float ParamActualVal){  // Member functions.
        switch (ParamName){
            case NH3OD_SENSOR:
                NH3AvgvalArray[NH3AvgCount] = ParamActualVal;
                Serial.printf("NH3 Val stored is: %0.3f at count number: %d\r\n",ParamActualVal,NH3AvgCount);
                NH3AvgCount ++;
                if(NH3AvgCount >= 99) {
                    NH3AvgCount = 0;
                }
                break;
            case SENSOR_2603:
                EthAvgvalArray[EthAvgCount] = ParamActualVal;
                Serial.printf("ETHNL Val stored is: %0.3f at count number: %d\r\n",ParamActualVal,EthAvgCount);
                EthAvgCount ++;
                if(EthAvgCount >= 99) {
                    EthAvgCount = 0;
                }
                break;

            default:
                break;
        }  

    }
    float RetrieveAvgVal(uint8_t ParamName){
        switch (ParamName){
            case NH3OD_SENSOR:
                Serial.println("Calculating average now for NH3OD");
                if(NH3AvgCount == 0) {
                    // Suppose the Count is 'Zero' as Sensor is not available then this function might not get executed and Count can remain Zero.
                    // So at such cases the Count needs to be kept as a non Zero number.
                    NH3AvgCount = 1;
                    Serial.println("Count is Zero.Considering it to be 'ONE' to avoid getting nan.");
                }
                for(int avg = 0; avg < NH3AvgCount; avg++) {

                    NH3AvgVal = NH3AvgVal + NH3AvgvalArray[avg];
                    Serial.printf("The value on adding is %0.3f\r\n",NH3AvgVal);
                }
                Serial.printf("The NH3OD value after adding %0.3f\r\n",NH3AvgVal);
                NH3AvgValue = (NH3AvgVal/NH3AvgCount);
                Serial.printf("The Average NH3OD is %0.3f of count %d\r\n",NH3AvgValue,NH3AvgCount);
                return NH3AvgValue;
                break;
            case SENSOR_2603:
                Serial.println("Calculating average now for ETHNL");
                if(EthAvgCount == 0) {
                    // Suppose the Count is 'Zero' as Sensor is not available then this function might not get executed and Count can remain Zero.
                    // So at such cases the Count needs to be kept as a non Zero number.
                    EthAvgCount = 1;
                    Serial.println("Count is Zero.Considering it to be 'ONE' to avoid getting nan.");
                }
                for(int avge = 0; avge < EthAvgCount; avge++) {

                    EthAvgVal = EthAvgVal + EthAvgvalArray[avge];
                    Serial.printf("The value on adding is %0.3f\r\n",EthAvgVal);
                }
                Serial.printf("The ETHNL value after adding %0.3f\r\n",EthAvgVal);
                EthAvgValue = (EthAvgVal/EthAvgCount);
                Serial.printf("The Average ETHNL is %0.3f of count %d\r\n",EthAvgValue,EthAvgCount);
                return EthAvgValue;
                break;

            default:
                return 0.0;
                break;
        }

    }
    void ClearContentsOfArray(uint8_t ParamName){
        switch (ParamName){
            case NH3OD_SENSOR:
                for(int cnt = 0; cnt < 100; cnt++){

                    NH3AvgvalArray[cnt] = 0;        
                }
                // Reset the Array Counter back to 'ZERO' when all the contents are cleared.Also clear the previously obtained Average Value(Very Imp otherwise the old value gets added up then).
                NH3AvgCount = 0;
                NH3AvgVal   = 0.0;
                Serial.println("NH3OD Array is cleared and count is made ZERO along with Avg Val");
                break;
            case SENSOR_2603:
                for(int cnte = 0; cnte < 100; cnte++){

                    EthAvgvalArray[cnte] = 0;        
                }
                // Reset the Array Counter back to 'ZERO' when all the contents are cleared.Also clear the previously obtained Average Value(Very Imp otherwise the old value gets added up then).
                EthAvgCount = 0;
                EthAvgVal   = 0.0;
                Serial.println("ETHNL Array is cleared and count is made ZERO along with Avg Val");
                break;

            default:
                break;

        }

    }

};




#endif // __V_CALIBRATIONSENSORCALCULATION_H__