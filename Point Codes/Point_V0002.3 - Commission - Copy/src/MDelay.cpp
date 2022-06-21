#include "MDelay.h"

// Setting delay in MS
void DELAY::setDelay(uint32_t delay)
{
    lastTickinMS = millis();
    delayinMS = delay;
    timeout = 0;
}

int32_t DELAY::getRemDelayinMS()
{
    return delayinMS;
}

bool DELAY::isTimeOut()
{
    uint32_t currentTickinMS = millis();
    if (timeout == 0)
    {
        delayinMS -= (currentTickinMS - lastTickinMS);
        lastTickinMS = currentTickinMS;
        if (delayinMS <= 0)
        {
            timeout = true;
        }
    }
    return timeout;
}