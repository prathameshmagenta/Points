#ifndef __V_DEVICE_MODEL_SINGLE_SENSOR_INFO_H__
#define __V_DEVICE_MODEL_SINGLE_SENSOR_INFO_H__

#include <Arduino.h>

class CVDeviceModelSingleSensorInfo
{
private:
    float mflDefMin;
    float mflDefMax;
    float mflRangeMin;
    float mflRangeMax;
    float mflStep;
    bool mbAlertBased;
    bool mbIsDefMinNull;
    bool mbIsDefMaxNull;
    bool mbIsAlertBasedNull;
    
public:
    CVDeviceModelSingleSensorInfo(); // No one should call default constructor, except stl map
    CVDeviceModelSingleSensorInfo(const CVDeviceModelSingleSensorInfo &inobjToCopy); // Copy Constructor
    CVDeviceModelSingleSensorInfo &operator=(const CVDeviceModelSingleSensorInfo &inobjRHS); // Assignment operator
    // Constructor for creating the object from numeric values
    CVDeviceModelSingleSensorInfo(float inflDefMin, float inflDefMax, float inflRangeMin, float inflRangeMax, float inflStep, bool inbAlertBased);
    // Constructor for creating the object from string values
    CVDeviceModelSingleSensorInfo(String instrDefMin, String instrDefMax, String instrRangeMin, String instrRangeMax, String instrStep, String instrAlertBased);
    // Function to initialize CVSingleSensorInfo object from JSON String.
    // Returns false if there is any error during initialization.
    bool initializeObjectFromJsonString(String instrSingleSensorInfoJSONString);

    float getDefMin();
    float getDefMax();
    float getRangeMin();
    float getRangeMax();
    float getStep();
    bool getAlertBased();
    bool isDefMinNull();
    bool isDefMaxNull();
    bool isAlertBasedNull();

    String toJsonString();

};


#endif // __V_DEVICE_MODEL_SINGLE_SENSOR_INFO_H__