#ifndef __V_SINGLEALERT_SETTING_H__
#define __V_SINGLEALERT_SETTING_H__

#include <Arduino.h>

class CVSingleAlertSetting
{
private:
    float mflLow;
    float mflHigh;
    bool mbNotify;
    bool mbIsLowNull;
    bool mbIsHighNull;

public:
    CVSingleAlertSetting(); // No one should call default constructor, except stl map
    CVSingleAlertSetting(const CVSingleAlertSetting &inobjToCopy); // Copy Constructor
    CVSingleAlertSetting &operator=(const CVSingleAlertSetting &inobjRHS); // Assignment operator
    // Constructor for creating the object from numeric values
    CVSingleAlertSetting(float inflLow, float inflHigh, bool inbNotify);
    // Constructor for creating the object from string values
    CVSingleAlertSetting(String instrLow, String instrHigh, bool inbNotify);
    // Function to initialize CVSingleAlertSetting object from JSON String.
    // Returns false if there is any error during initialization.
    bool initializeObjectFromJsonString(String instrSingleAlertSettingJSONString);

    float getLow();
    float getHigh();
    bool canNotify();
    bool isLowNull();
    bool isHighNull();
    String toJsonString();

};


#endif // __V_SINGLEALERT_SETTING_H__