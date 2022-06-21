#include <ArduinoJson.h>
#include "VDefines.h"
#include "VDeviceModelSingleSensorInfo.h"

// Default constructor
CVDeviceModelSingleSensorInfo::CVDeviceModelSingleSensorInfo() 
{
    mbIsDefMinNull = true;
    mbIsDefMaxNull = true;
    mbIsAlertBasedNull = true;
}

CVDeviceModelSingleSensorInfo::CVDeviceModelSingleSensorInfo(float inflDefMin, float inflDefMax, float inflRangeMin, float inflRangeMax, float inflStep, bool inbAlertBased)
{
    mflDefMin = inflDefMin;
    mflDefMax = inflDefMax;
    mflRangeMin = inflRangeMin;
    mflRangeMax = inflRangeMax;
    mflStep = inflStep;
    mbAlertBased = inbAlertBased;
    mbIsDefMinNull = false;
    mbIsDefMaxNull = false;
    mbIsAlertBasedNull = false;
}

CVDeviceModelSingleSensorInfo::CVDeviceModelSingleSensorInfo(String instrDefMin, String instrDefMax, String instrRangeMin, String instrRangeMax, String instrStep, String instrAlertBased) 
{
    // Remove leading and trailing spaces and convert to lower case to make comparison safer
    String lstrDefMin = instrDefMin;
    lstrDefMin.trim();
    lstrDefMin.toLowerCase();
    String lstrDefMax = instrDefMax;
    lstrDefMax.trim();
    lstrDefMax.toLowerCase();
    String lstrAlertbased = instrAlertBased;
    lstrAlertbased.trim();
    lstrAlertbased.toLowerCase();

    if( lstrDefMin.equals("null") ) {
        mbIsDefMinNull = true;
    } else {
        mbIsDefMinNull = false;
        mflDefMin = instrDefMin.toFloat(); // Note: No Exception handling required as toFloat() returns '0' if it couldn't convert. 
    }

    if( lstrDefMax.equals("null") ) {
        mbIsDefMaxNull = true;
    } else {
        mbIsDefMaxNull = false;
        mflDefMax = instrDefMax.toFloat();
    }

    if( lstrAlertbased.equals("true") ) {
        mbAlertBased = true;
        mbIsAlertBasedNull = false;
    } else if( lstrAlertbased.equals("false") ) {
        mbAlertBased = false;
        mbIsAlertBasedNull = false;
    } else {
        mbAlertBased = false;
        mbIsAlertBasedNull = true;
    }

    mflRangeMin = instrRangeMin.toFloat();
    mflRangeMax = instrRangeMax.toFloat();
    mflStep = instrStep.toFloat();
}


// copy constructor 
 CVDeviceModelSingleSensorInfo::CVDeviceModelSingleSensorInfo(const CVDeviceModelSingleSensorInfo &inobjToCopy) 
{
    if (this != &inobjToCopy) // Prevent self assignment
    {
        mflDefMin = inobjToCopy.mflDefMin;
        mflDefMax = inobjToCopy.mflDefMax;
        mflRangeMin = inobjToCopy.mflRangeMin;
        mflRangeMax = inobjToCopy.mflRangeMax;
        mflStep = inobjToCopy.mflStep;
        mbAlertBased = inobjToCopy.mbAlertBased;
        mbIsDefMinNull = inobjToCopy.mbIsDefMinNull;
        mbIsDefMaxNull = inobjToCopy.mbIsDefMaxNull;
        mbIsAlertBasedNull = inobjToCopy.mbIsAlertBasedNull;
    }
}


// Assignment operator
CVDeviceModelSingleSensorInfo& CVDeviceModelSingleSensorInfo::operator=(const CVDeviceModelSingleSensorInfo &inobjRHS) 
{
    if (this != &inobjRHS) // Prevent self assignment
    {
        mflDefMin = inobjRHS.mflDefMin;
        mflDefMax = inobjRHS.mflDefMax;
        mflRangeMin = inobjRHS.mflRangeMin;
        mflRangeMax = inobjRHS.mflRangeMax;
        mflStep = inobjRHS.mflStep;
        mbAlertBased = inobjRHS.mbAlertBased;
        mbIsDefMinNull = inobjRHS.mbIsDefMinNull;
        mbIsDefMaxNull = inobjRHS.mbIsDefMaxNull;
        mbIsAlertBasedNull = inobjRHS.mbIsAlertBasedNull;
    }

    return *this;
}

// Function to initialize CVDeviceModelSingleSensorInfo object from JSON String.
// Returns false if there is any error during initialization.
bool CVDeviceModelSingleSensorInfo::initializeObjectFromJsonString(String instrSingleSensorInfoJSONString) 
{
    bool lbRetVal = true;
    String lstrAlertBasedVal = "null";

    DeserializationError err;
    DynamicJsonDocument docSingleSnesorInfo(1024);
    JsonObject objJsonSingleSensorInfo;

    // Convert single param Model Info string to json object
    err = deserializeJson(docSingleSnesorInfo, instrSingleSensorInfoJSONString);
    if (err) {
        Serial.print("Error while converting Single Param ModelInfoJSON String to JSON Document. Error: ");
        Serial.println(err.c_str());
        lbRetVal = false; // Indicate there is an Error.
    } else {
        objJsonSingleSensorInfo = docSingleSnesorInfo.as<JsonObject>();
    }

    if( objJsonSingleSensorInfo.containsKey(MODEL_SENSOR_INFO_STEP) == false || 
        objJsonSingleSensorInfo.containsKey(MODEL_SENSOR_INFO_DEF_MAX) == false ||
        objJsonSingleSensorInfo.containsKey(MODEL_SENSOR_INFO_DEF_MIN) == false ||
        objJsonSingleSensorInfo.containsKey(MODEL_SENSOR_INFO_RANGE_MAX) == false ||
        objJsonSingleSensorInfo.containsKey(MODEL_SENSOR_INFO_RANGE_MIN) == false 
    ) {
        Serial.println( "Should not happen. Json for Model Sensor Info has missing attributes.");
        lbRetVal = false; // Indicate there is an Error.
    }

    if( objJsonSingleSensorInfo.containsKey(MODEL_SENSOR_INFO_ALERTBASED) == true ) {
        lstrAlertBasedVal = objJsonSingleSensorInfo[MODEL_SENSOR_INFO_ALERTBASED].as<String>();
    }
    
    // Intitialize this object using Assignment operator defined above.
    *this = CVDeviceModelSingleSensorInfo(
        objJsonSingleSensorInfo[MODEL_SENSOR_INFO_DEF_MIN].as<String>(),
        objJsonSingleSensorInfo[MODEL_SENSOR_INFO_DEF_MAX].as<String>(),
        objJsonSingleSensorInfo[MODEL_SENSOR_INFO_RANGE_MIN].as<String>(),
        objJsonSingleSensorInfo[MODEL_SENSOR_INFO_RANGE_MAX].as<String>(),
        objJsonSingleSensorInfo[MODEL_SENSOR_INFO_STEP].as<String>(),
        lstrAlertBasedVal
    );

    return lbRetVal;
}


float CVDeviceModelSingleSensorInfo::getDefMin()
{
    return mflDefMin;
}

float CVDeviceModelSingleSensorInfo::getDefMax()
{
    return mflDefMax;
}

float CVDeviceModelSingleSensorInfo::getRangeMin()
{
    return mflRangeMin;
}

float CVDeviceModelSingleSensorInfo::getRangeMax()
{
    return mflRangeMax;
}

float CVDeviceModelSingleSensorInfo::getStep()
{
    return mflStep;
}

bool CVDeviceModelSingleSensorInfo::isDefMinNull()
{
    return mbIsDefMinNull;
}

bool CVDeviceModelSingleSensorInfo::isDefMaxNull() 
{
    return mbIsDefMaxNull;
}

bool CVDeviceModelSingleSensorInfo::getAlertBased() 
{
    return mbAlertBased;
}

bool CVDeviceModelSingleSensorInfo::isAlertBasedNull() 
{
    return mbIsAlertBasedNull;
}

String CVDeviceModelSingleSensorInfo::toJsonString()
{
    String strRetJson = String("{") + 
                            "\"" + MODEL_SENSOR_INFO_DEF_MIN + "\": " + ( isDefMinNull() ? "null" : String(mflDefMin, 2) ) + "," +
                            "\"" + MODEL_SENSOR_INFO_DEF_MAX + "\": " + ( isDefMaxNull() ? "null" : String(mflDefMax, 2) ) + "," +
                            "\"" + MODEL_SENSOR_INFO_RANGE_MIN + "\": " + String(mflRangeMin, 2) + "," +
                            "\"" + MODEL_SENSOR_INFO_RANGE_MAX + "\": " + String(mflRangeMax, 2) + "," +
                            "\"" + MODEL_SENSOR_INFO_STEP + "\": " + String(mflStep, 2) +
                            ( mbIsAlertBasedNull ? "" : String(",\"") + MODEL_SENSOR_INFO_ALERTBASED + "\": " + String(mbAlertBased) ) +
                        "}";

    return strRetJson;
}