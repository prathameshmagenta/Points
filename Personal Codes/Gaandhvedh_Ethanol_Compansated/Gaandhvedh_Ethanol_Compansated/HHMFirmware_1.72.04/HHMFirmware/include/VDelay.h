#ifndef __V_DELAY_H__
#define __V_DELAY_H__

#include <Arduino.h>
#include <stdint.h>


#define RETRY_WIFI_CONN_DELAY   (1*60*1000)         //@ 1Min (in MS)
#define ESP32_AP_MODE_DELAY     (5*60*1000)         //@ 5Min (in MS)

#define BACK_UP_DATA_INTERVAL   (1*60*1000)         //@ 1Min (in MS)

#define OTA_UPDATION_DELAY      (1*60*1000)         //@ 30Min (in Ms)
#define OTA_TIMEOUT             (2*60*1000)         //@ 2Min (in Ms)

// Do not use this data interval now as it is now decided by the user from the webpage.Default for new PCB will be one minute.
// #define DATA_FILTRATION_DELAY       (1*60*1000)         //@ 1Min (in MS)
#define DATA_SERIAL_DISPLAY_DELAY   (1*60*1000)         //@ 1Min (in MS)

#define VOC_STARTUP_DELAY        (20*60*1000)        //@ 20Min (in MS)
#define VOC_ENVDATA_UPDATE_PRD   (2*60*1000)         //@ 2Min (in MS)
#define VOC_BASELINE_DAILY_UPDATE_PRD  (24*60*60*1000)  //@ 24 hrs (in MS)
#define VOC_BASELINE_UPDATE_PRD  (8*24*60*60*1000)       //@ 8days(in MS)


#define NH3OD_STARTUP_DELAY     (5*60*1000)         //@ 5Min (in MS)
#define NH3OD_SAMPLING_DELAY    (1*1000)            //@ 1Sec (in MS)

#define NH3OD_COMPENSATION_DELAY  (2*60*1000)       //@ 2Min (in MS)

#define ETHANOL_STARTUP_DELAY     (5*60*1000)         //@ 5Min (in MS)
#define ETHANOL_SAMPLING_DELAY    (1*1000)            //@ 1Sec (in MS)

#define HDC_SAMPLING_DELAY      (1*1000)            //@ 1Sec (in MS)

#define CALIBRATION_PROCESSING_DELAY (1*1000)       //@ 1Sec (in MS)
#define CALIBRATION_WINDOW      (60*60*1000)        //@ 60Min (in Ms)
#define INVOKE_CURRENT_CALIB_STEP (8*1000)          //@ 8Sec (in MS)

#define TGS_CALIB_DELAY         (4*60*1000)       //@ 4Min (in MS)
#define TGS_WAIT_CALIB_DELAY    (3*60*1000)       //@ 3Min (in MS)
#define TGS_READ_DELAY          (1*1000)          //@ 1Sec (in MS)

#define RTC_CORRECTION_DELAY   (24*60*60*1000)      //@ 24 hrs (in MS)

class DELAY
{
public:
    void setDelay( uint32_t );
    int32_t getRemDelayinMS();
    bool isTimeOut();
protected:
    uint32_t lastTickinMS;
    int32_t delayinMS;
    bool timeout;
};

#endif