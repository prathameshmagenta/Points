#ifndef __V_CALCULATION_H__
#define __V_CALCULATION_H__

#include "stdint.h"

#include "VAlertEngine.h"

enum Direction{

    NO_TRIGGER = 0,
    ALARM_DISABLED = 1,
    INVALID = 2,

    NO_VALID_TRIGGER = 5,

    VALID_POSITIVE_TRIGGER = 0x10,   

    VALID_NEGATIVE_TRIGGER = 0x20,   
};



#define REJECTION_PERCENT   20


float calculation_calculateAvg (float *buffer, uint32_t bufferSize,
                                        uint32_t writePointer, uint32_t &readPointer);

float calculation_calAvgAfterFilter (float *buffer, uint32_t bufferSize,
                                uint32_t writePointer, uint32_t &readPointer);


//template<typename T>
int32_t calculation_detectValidTriggerPoint (float *buffer, uint32_t bufferSize,
                                uint32_t writePointer, uint32_t* readPointer,
                                struct alarm_settings *a_setting, 
                                float &validTriggerPoint);


#endif