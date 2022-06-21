#ifndef __V_MODELSENSORINFO_H__
#define __V_MODELSENSORINFO_H__

#include<map>
#include <Arduino.h>
#include "VDeviceModelSingleSensorInfo.h"


String getDeviceModelSensorInfoFromServer( std::map<String, String>& inMapConnectionInfo, 
                                        std::map<String, String>& inMapDeviceInfo,
                                        std::map<String, CVDeviceModelSingleSensorInfo>& inMapDeviceModelSensorInfo );


#endif // __V_MODELSENSORINFO_H__