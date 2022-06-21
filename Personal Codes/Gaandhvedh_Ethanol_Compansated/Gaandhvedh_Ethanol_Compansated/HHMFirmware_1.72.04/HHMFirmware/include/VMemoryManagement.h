#ifndef __V_MEMEORYMANAGEMENT_H__
#define __V_MEMEORYMANAGEMENT_H__

#include <Arduino.h>
#include "VDeviceData.h"
#include "VInformationManager.h"

void SaveSensorDataInSPIFFS(String getRtcDate, String getRtcTime);
void RetrieveAndSendSensorDataFromSPIFFS();


#endif