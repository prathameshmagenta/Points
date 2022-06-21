#ifndef __V_ALERTENGINE_H__
#define __V_ALERTENGINE_H__

#include <map>
#include "VSingleAlertSetting.h"
#include "VDelay.h"
#include "VDefines.h"
#include <Arduino.h>


#define ALERT_PROCESSING_DELAY  (1*1000)       // @ 1 Sec(in MS)

//Don't set Alert trigger interval more than 30mints
#define ALERT_DELAY_NO2       (15*60*1000)
#define ALERT_DELAY_O3        (15*60*1000)
#define ALERT_DELAY_SO2       (15*60*1000)
#define ALERT_DELAY_VOC       (10*1000)      // @ checks every 10 sec, 15 min(in MS) interval
#define ALERT_DELAY_CO        (15*60*1000)
#define ALERT_DELAY_NH3       (15*60*1000)
#define ALERT_DELAY_CO2       (15*60*1000)
#define ALERT_DELAY_HUM       (10*1000)      // @ checks every 10 sec, 15 min(in MS) interval
#define ALERT_DELAY_PM1       (15*60*1000)
#define ALERT_DELAY_PM10      (15*60*1000)
#define ALERT_DELAY_PM25      (15*60*1000)
#define ALERT_DELAY_TEMP      (10*1000)      // @ checks every 10 sec, 15 min(in MS) interval
#define ALERT_DELAY_NH3OD     (10*1000)      // @ checks every 10 sec, 15 min(in MS) interval
#define ALERT_DELAY_DEFAULT   (24*60*60*1000)


struct alarm_settings{

    float lowCutoff;
    float highCutoff;

    bool isLowNull;
    bool isHighNull;
};

extern DELAY   objAlertDelay_TEMP;
extern DELAY   objAlertDelay_HUM;
extern DELAY   objAlertDelay_VOC;
extern DELAY   objAlertDelay_NH3OD;

class ALERT_DELAY_COUNTER
{
protected:                // Access specifiers.
    int8_t countforTEMP;
    int8_t countforHUM;
    int8_t countforVOC;
    int8_t countforNH3OD;

public:
    ALERT_DELAY_COUNTER() {  
        countforTEMP = 0;  // Constructor.
        countforHUM = 0;
        countforVOC = 0;
        countforNH3OD = 0;
    }

    // Member functions.
    void setCounterforTrigger(String param) {

        if(param == VMP_TEMP) { // The first delay is immediate, and the subsequent ones are after 15 mins.
            countforTEMP ++;
            if(countforTEMP >= 1) {
                objAlertDelay_TEMP.setDelay(15*60*1000);

                // The Count should not overflow at all if ResetCounterforNoTrigger is not encountered for a long period of time eg. Hum in rainy season.
                if(countforTEMP == 100) {

                    countforTEMP = 0;
                }
            }
        }

        if(param == VMP_HUM) {  // The first delay is immediate, and the subsequent ones are after 15 mins.
            countforHUM ++;
            if(countforHUM >= 1) {
                objAlertDelay_HUM.setDelay(15*60*1000);

                // The Count should not overflow at all if ResetCounterforNoTrigger is not encountered for a long period of time eg. Hum in rainy season.
                if(countforHUM == 100) {

                    countforHUM = 0;
                }
            }
        }

        if(param == VMP_VOC) {  // The first delay is immediate, and the subsequent ones are after 15 mins.
            countforVOC ++;
            if(countforVOC >= 1) {
                objAlertDelay_VOC.setDelay(15*60*1000);

                // The Count should not overflow at all if ResetCounterforNoTrigger is not encountered for a long period of time eg. Hum in rainy season.
                if(countforVOC == 100) {

                    countforVOC = 0;
                }
            }
        }

        if(param == VMP_NH3OD) { // The first delay is immediate, and the subsequent ones are after 15 mins.
            countforNH3OD ++;
            if(countforNH3OD >= 1) {
                objAlertDelay_NH3OD.setDelay(15*60*1000);

                // The Count should not overflow at all if ResetCounterforNoTrigger is not encountered for a long period of time eg. Hum in rainy season.
                if(countforNH3OD == 100) {

                    countforNH3OD = 0;
                }
            }
        }
    
    }

    void ResetCounterforNoTrigger(String param) {

        if(param == VMP_TEMP) {   // Reset the counter to 'zero'
            countforTEMP = 0;
        }

        if(param == VMP_HUM) {    // Reset the counter to 'zero'
            countforHUM = 0;
        }

        if(param == VMP_VOC) {    // Reset the counter to 'zero'
            countforVOC = 0;
        }

        if(param == VMP_NH3OD) {  // Reset the counter to 'zero'
            countforNH3OD = 0;
        }

    }

};

String getAlertSettingsFromServer( std::map<String, String>& inMapConnectionInfo, 
                                        std::map<String, String>& inMapDeviceInfo,
                                        std::map<String, CVSingleAlertSetting>& inMapAlertSettings );

String alert_handleAllActiveAlerts(std::map<String, String>& inMapConnectionInfo,
                                std::map<String, String>& inMapDeviceInfo,
                                std::map<String, CVSingleAlertSetting>& inMapAlertSettings,
                                std::map<String, double>& inMapSensorReadings,
                                String AlertTime,
                                String &retAlertTime,
                                String RTCStat);


void alert_initAlertSystem ();

String getParNameBasedOnType (String inParamNameType);
String getUnitsBasedOnParamtype( String inParamNameType);

void setDelayBasedOnType (String inParamNameType);
bool ifDelayExpired(String inParamNameType);

#endif // __V_ALERTSETTINGS_H__