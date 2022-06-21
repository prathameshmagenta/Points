#ifndef __V_CALIBRATIONAPIEXECUTION_H__
#define __V_CALIBRATIONAPIEXECUTION_H__

#include <map>
#include <Arduino.h>

String sendAcknowledgementForCalibration(std::map<String, String>& inMapConnectionInfo,
                                        std::map<String, String>& inMapDeviceInfo);

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
                            float &PostCalParamOutput);

String SaveCalibrationStepResult(std::map<String, String>& inMapConnectionInfo,
                                std::map<String, String>& inMapDeviceInfo,
                                String strResPostBody);


String SendSaveCalibErrorToServer(std::map<String, String>& inMapConnectionInfo,
                                std::map<String, String>& inMapDeviceInfo,
                                String strResPostErrorBody);

String SendAckForDeviceOutOfCalibMode();

String FiregetCurrentUTCDateTime();

String SendAcknowledgementforDeviceUpdates();

String SendStatusForOTAUpdate();

void ResetRTC();



#endif // __V_CALIBRATIONAPIEXECUTION_H__