#ifndef __V_COMMUNICATION_H__
#define __V_COMMUNICATION_H__

#include <stdint.h>

#include "VDelay.h"

enum esp32_state{
    ESP32_MISSING_CONFIG = 1,
//    ESP32_ALL_MODES_DISABLE,

    ESP32_WIFI_CONNECTED,
    ESP32_WIFI_NOT_CONNECTED,

    ESP32_AP_CONNECTED,
    ESP32_AP_NOT_CONNECTED,
};

class esp32: public DELAY
{
protected:
    uint16_t currentState;
    uint16_t noOfAttempt;

public:
    void setCurrentState(uint16_t state);
    uint16_t getCurrentState();

    void increaseAttemptNo();
    void resetAttemptNo();
    bool isMaxAttempt();

};

bool comm_communicateToServer(std::map<String, String>& inMapConnectionInfo,
                                    std::map<String, String>& inMapDeviceInfo,
                                    const String& uri,
                                    const String& postBody,
                                    String& response );

bool comm_communicateToServerForOTA(const String &url,
                                    const String &postBody,
                                    String &response);

#endif