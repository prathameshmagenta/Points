#ifndef __V_RESPONSEPARSER_H__
#define __V_RESPONSEPARSER_H__

#include <map>
#include <Arduino.h>

#include "VSingleAlertSetting.h"
#include "VDeviceModelSingleSensorInfo.h"


enum Response{
    INVALID_DEVICE = -10,
    INVALID_AUTH = -20,
    INVALID_FWID = -30,
    SERVER_PROBLEM = -40,

    SUCCESS = 0,
    UNKNOWN = 1,

    NEW_SETTINGS = 10,
};
/*
String sendAlertMessageToServer(std::map<String, String>& inMapConnectionInfo, 
                                std::map<String, String>& inMapDeviceInfo,
                                std::map<String, CVSingleAlertSetting>& inMapAlertSettings, 
                                std::map<String, float>& inMapDeviceValues,
                                std::map<String, CVDeviceModelSingleSensorInfo>& inMapDeviceModelSensorInfo );
*/


int32_t parseAlertResponse(String &param, String &resposne,
                                    std::map<String, CVSingleAlertSetting>& inMapAlertSettings);

int32_t parseOtaResponse(String &response);

#endif // __V_ALERTLOG_H__