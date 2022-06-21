#ifndef __V_SENSORS_H__
#define __V_SENSORS_H__

#include <Arduino.h>
#include <map>

#define VOC_SENSOR 1
#define TEMP_SENSOR 2
#define RH_SENSOR 3
#define NH3OD_SENSOR 4
#define OD_SENSOR_RS 5
#define SENSOR_2603 6

#define TEMP_SAMPL_FREQ (1 * 1000) //@ 1Sec (in MS)
#define HUM_SAMPL_FREQ TEMP_SAMPL_FREQ
#define VOC_SAMPL_FREQ
#define NH3_SAMPL_FREQ

#define BUFFER_SIZE 200
#define ALERT_BUFFER_SIZE 50
//#define SIZE_OF_ALERT_BUFF()    ALERT_BUFFER_SIZE

class SENSOR_AVAILABILITY
{
protected:
    bool CCS811status = false;
    bool SGPXXstatus = false;
    bool TGS2602status = false;
    bool TGS2603status = false;

public:
    void setCCS811Status(bool value)
    {
        CCS811status = value;
    }
    bool isCCS811Ready(void)
    {
        return CCS811status;
    }

    void setSGPXXStatus(bool value)
    {
        SGPXXstatus = value;
    }
    bool isSGPXXReady(void)
    {
        return SGPXXstatus;
    }

    void setTGS2602Status(bool value)
    {
        TGS2602status = value;
    }
    bool isTGS2602Ready(void)
    {
        return TGS2602status;
    }

    void setTGS2603Status(bool value)
    {
        TGS2603status = value;
    }
    bool isTGS2603Ready(void)
    {
        return TGS2603status;
    }
};

class SENSOR_DATA
{
protected:
    float avgVOC;
    float avgTEMP;
    float avgRH;
    float avgNH3;
    float avgNH3RS;
    float avgEthanol;

public:
    SENSOR_DATA()
    {
        avgVOC = 0;
        avgTEMP = 0;
        avgRH = 0;
        avgNH3 = 0;
        avgNH3RS = 0;
        avgEthanol = 0;
    }
    void setValue(uint32_t param, float value)
    {
        switch (param)
        {
        case VOC_SENSOR:
            avgVOC = value;
            break;
        case TEMP_SENSOR:
            avgTEMP = value;
            break;
        case RH_SENSOR:
            avgRH = value;
            break;
        case NH3OD_SENSOR:
            avgNH3 = value;
            break;
        case OD_SENSOR_RS:
            avgNH3RS = value;
            break;
        case SENSOR_2603:
            avgEthanol = value;
            break;
        default:
            break;
        }
    }
    float getValue(uint32_t param)
    {
        switch (param)
        {
        case VOC_SENSOR:
            return avgVOC;
            break;
        case TEMP_SENSOR:
            return avgTEMP;
            break;
        case RH_SENSOR:
            return avgRH;
            break;
        case NH3OD_SENSOR:
            return avgNH3;
            break;
        case OD_SENSOR_RS:
            return avgNH3RS;
            break;
        case SENSOR_2603:
            return avgEthanol;
            break;
        default:
            return 0.0;
            break;
        }
    }
};

void sensor_initAlertBuffers();
bool sensor_readBaselineOfCCS(uint16_t &baseLine);
bool sensor_readBaselineOfSGP(uint16_t &baseLine);
float sensor_readR0ofTGS(String inSensorType);
float getNH3ODReadings(float &RS_F2602);
float getEthanolReadings(float &RS_F2603);
bool getVOCReadings();

void sensor_initAllSensors();
void sensor_calculateNCollectReadings(std::map<String, double> &inMapSensorReadings);
void sensor_reinsertAveragedOutValues(std::map<String, double> &inMapSensorReadings);
void UpdateHighestResistanceValues();
float CheckForCrossSensitivity(float inflRSAmmonia, float inflRSEthanol);

void sensor_doVOCCalibration();

String sensor_getSensorReadings(std::map<String, double> &inMapSensorReadings);
String generateRandomSensorReadings(std::map<String, double> &inMapSensorReadings);

void getSensorInfo(String &TempStatretVal, String &HumStatretVal, String &CCSStatretVal, String &NH3ODStatretVal, String &EthanolStatretVal, String &RTCStatretVal);

float *sensor_getBufferBasedOnType(String inParamNameType);
uint32_t sensor_getWriteIndexBasedOnType(String inParamNameType);
uint32_t *sensor_getReadIndexBasedOnType(String inParamNameType);

uint8_t sensor_calibrateSensor(String sensor, float currentTemp, float refTemp);
bool sensor_getParamFromHDC2080(float &temp, float &hum);

bool sensor_readTempCali(uint8_t &data);
bool sensor_readHumCali(uint8_t &data);
bool sensor_validateCaliHDC2080(String param);

#endif // __V_SENSORS_H__
