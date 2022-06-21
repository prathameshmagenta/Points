#include <ArduinoJson.h>
#include "VDefines.h"
#include "VSingleAlertSetting.h"

// Default private constructor
CVSingleAlertSetting::CVSingleAlertSetting() 
{
    mbNotify = false;
    mbIsLowNull = true;
    mbIsHighNull = true;
}


// This Constructor may not be required
CVSingleAlertSetting::CVSingleAlertSetting(float inflLow, float inflHigh, bool inbNotify) 
{
    mflLow = inflLow;
    mflHigh = inflHigh;
    mbNotify = inbNotify;
    mbIsLowNull = false;
    mbIsHighNull = false;
}

CVSingleAlertSetting::CVSingleAlertSetting(String instrLow, String instrHigh, bool inbNotify) 
{
    // Remove leading and trailing spaces and convert to lower case to make comparison safer
    String lstrLow = instrLow;
    lstrLow.trim();
    lstrLow.toLowerCase();
    String lstrHigh = instrHigh;
    lstrHigh.trim();
    lstrHigh.toLowerCase();

    if( lstrLow.equals("null") ) {
        mbIsLowNull = true;
        mflLow = 0.0f;
    } else {
        mbIsLowNull = false;
        mflLow = instrLow.toFloat();
    }

    if( lstrHigh.equals("null") ) {
        mbIsHighNull = true;
        mflHigh = 0.0f;
    } else {
        mbIsHighNull = false;
        mflHigh = instrHigh.toFloat();
    }

    mbNotify = inbNotify;
}

// Copy constructor
CVSingleAlertSetting::CVSingleAlertSetting(const CVSingleAlertSetting &inobjToCopy) 
{
    if (this != &inobjToCopy) // Prevent self assignment
    {
        mflLow = inobjToCopy.mflLow;
        mflHigh = inobjToCopy.mflHigh;
        mbNotify = inobjToCopy.mbNotify;
        mbIsLowNull = inobjToCopy.mbIsLowNull;
        mbIsHighNull = inobjToCopy.mbIsHighNull;
    }
}

// Assignment operator
CVSingleAlertSetting& CVSingleAlertSetting::operator=(const CVSingleAlertSetting &inobjRHS) 
{
    if (this != &inobjRHS) // Prevent self assignment
    {
        mflLow = inobjRHS.mflLow;
        mflHigh = inobjRHS.mflHigh;
        mbNotify = inobjRHS.mbNotify;
        mbIsLowNull = inobjRHS.mbIsLowNull;
        mbIsHighNull = inobjRHS.mbIsHighNull;
    }

    return *this;
}

// Function to initialize CVSingleAlertSetting object from JSON String.
// Returns false if there is any error during initialization.
bool CVSingleAlertSetting::initializeObjectFromJsonString(String instrSingleAlertSettingJSONString) 
{
    bool lbRetVal = true;

    DeserializationError err;
    DynamicJsonDocument docSingleAlertSetting(1024);
    JsonObject objJsonSingleAlertSetting;

    // Convert single param alert setting string to json object
    err = deserializeJson(docSingleAlertSetting, instrSingleAlertSettingJSONString);
    if (err) {
        Serial.print("Error while converting Single Param AlertSettingJSON String to JSON Document. Error: ");
        Serial.println(err.c_str());
        lbRetVal = false; // Indicate there is an Error.
    } else {
        objJsonSingleAlertSetting = docSingleAlertSetting.as<JsonObject>();
    }

    if( objJsonSingleAlertSetting.containsKey(ALERT_SETTING_LOW) == false || 
        objJsonSingleAlertSetting.containsKey(ALERT_SETTING_HIGH) == false ||
        objJsonSingleAlertSetting.containsKey(ALERT_SETTING_NOTIFY) == false ) {
            
        Serial.println( "Should not happen. Json for Alert Setting has missing attributes.");
        lbRetVal = false; // Indicate there is an Error.
    }

    // Intitialize this object using Assignment operator defined above.
    *this = CVSingleAlertSetting(
        objJsonSingleAlertSetting[ALERT_SETTING_LOW].as<String>(),
        objJsonSingleAlertSetting[ALERT_SETTING_HIGH].as<String>(),
        objJsonSingleAlertSetting[ALERT_SETTING_NOTIFY].as<bool>() );

    return lbRetVal;
}

float CVSingleAlertSetting::getLow()
{
    return mflLow;
}

float CVSingleAlertSetting::getHigh()
{
    return mflHigh;
}

bool CVSingleAlertSetting::canNotify()
{
    return mbNotify;
}

bool CVSingleAlertSetting::isLowNull()
{
    return mbIsLowNull;
}

bool CVSingleAlertSetting::isHighNull()
{
    return mbIsHighNull;
}

String CVSingleAlertSetting::toJsonString()
{
    String strRetJson = String("{") + 
                            "\"" + ALERT_SETTING_LOW + "\": " + ( isLowNull() ? "null" : String(mflLow, 2) ) + "," +
                            "\"" + ALERT_SETTING_HIGH + "\": " + ( isHighNull() ? "null" : String(mflHigh, 2) ) + "," +
                            "\"" + ALERT_SETTING_NOTIFY + "\": " + ( canNotify() ? "true" : "false" ) +
                        "}";

    return strRetJson;
}



