#ifndef __V_REGISTRATION_H__
#define __V_REGISTRATION_H__

#include<map>
#include <Arduino.h>

String registerDevice(std::map<String, String>& inMapConnectionInfo,
                      std::map<String, String>& inMapDeviceInfo,
                      std::map<String, uint32_t>& inMapCalibrationInfo);


#endif // __V_REGISTRATION_H__