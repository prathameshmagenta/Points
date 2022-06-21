#ifndef __MDELAY_H__
#define __MDELAY_H__

#include <Arduino.h>
#include <stdint.h>

#define BLE_CONN_WAIT_DELAY (1 * 60 * 1000) //@ 1Min (in MS)

class DELAY
{
public:
    void setDelay(uint32_t);
    int32_t getRemDelayinMS();
    bool isTimeOut();

protected:
    uint32_t lastTickinMS;
    int32_t delayinMS;
    bool timeout;
};

#endif