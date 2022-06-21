#include "VSensors.h"
#include "VDefines.h"

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include "SparkFunCCS811.h" // Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include "SparkFun_SGP30_Arduino_Library.h"

#include <HDC2080.h> // Temp sensor
#include <RTCx.h>    // DS1307 RTC

#include "VDeviceModelSensorInfo.h"
#include "VInformationManager.h"

#include "VDelay.h"
#include "VCalculation.h"
#include "VResponseParser.h"
#include "VCmdLineInterface.h"

#include "VCalibrationSensorCalculation.h"

// Since its a combined firmware.
#include "VNH3.h"

DELAY objHDCReadDelay;

DELAY objVOCStartupDelay;
DELAY objVOCEnvDataUpdationDelay;
DELAY objVOCBaselineDailyUpdationDelay;
DELAY objVOCBaselineUpdationDelay;

DELAY objTGS2602StartupDelay;
DELAY objTGS2602ReadDelay;

DELAY objTGS2603StartupDelay;
DELAY objTGS2603ReadDelay;

DELAY objNH3CompensationDelay;

#define CCS811_ADDR 0x5B
#define HDC2080_ADDR 0x40
#define ADS1115_ADDR 0x48 // ADDR is connected to GND.

SENSOR_AVAILABILITY sensor;
SENSOR_DATA SensorValue;
// Create an object first to initialize slave address
CCS811 objVOCSensor(CCS811_ADDR);
//SGP30 objVOCSensor_1;
HDC2080 objTempHumSensor(HDC2080_ADDR);
Adafruit_ADS1115 ads(ADS1115_ADDR);
SENSOR_RETRIEVE_MIN_VAL objStoreRetrieveAvgVal;

extern std::map<String, long> mapSensorSanityInterval; // To avoid glitch in a Sensor.
extern std::map<String, uint32_t> mapCalibrationInfo;
extern std::map<String, double> mapCalibrationSensorValueInfo;

float buffer_VOC[BUFFER_SIZE];
float buffer_CO2[BUFFER_SIZE];
float buffer_NH3OD[BUFFER_SIZE];
float buffer_Ethanol[BUFFER_SIZE];
float buffer_OD_RS[BUFFER_SIZE];
float buffer_TEMP[BUFFER_SIZE];
float buffer_RH[BUFFER_SIZE];

uint32_t writeVocIndex = 0;
uint32_t readVocIndex = 0;
uint32_t writeCo2Index = 0;
uint32_t readCo2Index = 0;
uint32_t writeNh3ODIndex = 0;
uint32_t readNh3ODIndex = 0;
uint32_t writeEthanolIndex = 0;
uint32_t readEthanolIndex = 0;
uint32_t writeODRSIndex = 0;
uint32_t readODRSIndex = 0;
uint32_t writeTempIndex = 0;
uint32_t readTempIndex = 0;
uint32_t writeRhIndex = 0;
uint32_t readRhIndex = 0;

float alert_buffer_VOC[ALERT_BUFFER_SIZE];
float alert_buffer_TEMP[ALERT_BUFFER_SIZE];
float alert_buffer_RH[ALERT_BUFFER_SIZE];
float alert_buffer_NH3OD[ALERT_BUFFER_SIZE];

uint32_t alert_writeVocIndex = 0;
uint32_t alert_readVocIndex = 0;
uint32_t alert_writeTempIndex = 0;
uint32_t alert_readTempIndex = 0;
uint32_t alert_writeRhIndex = 0;
uint32_t alert_readRhIndex = 0;

uint32_t alert_writeNh3ODIndex = 0;
uint32_t alert_readNh3ODIndex = 0;

int32_t ADSChannel0, ADSChannel3;
float RS_F2602 = 0.0f;
float ppm_Ethanol = 0.0f;
float flAvgAmmoniaRSVal = 0.0f;

#define VOLT_PER_ADC 0.0000625f // Vref/2^15 (2.048/32768)

#define RL_F2602 1 // (kilo ohms).... LOAD RESISTANCE
#define R6 1.0f    // In KOhm (Please check Schematics)
#define R7 1.0f    // In KOhm (Load Resistor)
#define R9 2.0f    // In KOhm

#define FACTOR1 (R6 + R9) / R9 // Voltage Divider Rule.
#define RL (R7 * (R6 + R9) / (R7 + R6 + R9))

// VOC Baseline
uint16_t CorrectedBaseline;
float ResVocValue = 0.0f;
uint16_t CCSbaseLine;
uint8_t CCScount = 0;

// For NH3OD.
String NH3ODStat = "";
// For Ethanol.
String EthanolStat = "";

float TemVal, HumVal = 0.0f;
bool NH3ODStatNegRs = false;
bool EthanolStatNegRs = false;
bool CCSStatVal = false;
bool SGPXXStatVal = false;
bool RTCStatVal = false;
String TempStatretVal = "";
String HumStatretVal = "";
String CCSStatretVal = "";
String NH3ODStatretVal = "";
String RTCStatretVal = "";

// Flag check whether the average of data values has been calculated or not.
bool isAverageSensorValuesCalcualted = false;

void initADS1115()
{

  ads.begin();
  ads.setGain(GAIN_TWO);
  sensor.setTGS2602Status(false);
  objTGS2602StartupDelay.setDelay(NH3OD_STARTUP_DELAY);
  objTGS2602ReadDelay.setDelay(NH3OD_SAMPLING_DELAY);
  objTGS2603StartupDelay.setDelay(ETHANOL_STARTUP_DELAY);
  objTGS2603ReadDelay.setDelay(ETHANOL_SAMPLING_DELAY);
}

void initCCS811ForVOC()
{
  // Initialize CCS811
  Wire.begin(); // Inialize I2C Hardware

  if (objVOCSensor.begin() == false)
  {
    Serial.print("CCS811 sensor error. Please check wiring. Freezing...");
    CCSStatVal = false;
  }
  else
  {
    Serial.println("VOC Initialized");
    CCSStatVal = true;
  }

  // CCS811 will not be availble during conditioning period
  sensor.setCCS811Status(false);
  // Define how much time(in MS) CCS811 should wait after
  // power up, ENV_DATA updation and BASE_LINE updation
  objVOCStartupDelay.setDelay(VOC_STARTUP_DELAY);
  objVOCEnvDataUpdationDelay.setDelay(VOC_ENVDATA_UPDATE_PRD);
  objVOCBaselineDailyUpdationDelay.setDelay(VOC_BASELINE_DAILY_UPDATE_PRD);
  objVOCBaselineUpdationDelay.setDelay(VOC_BASELINE_UPDATE_PRD);
}
/*
void initSGPXXForVOC()
{
  // Initialize CCS811
  Wire.begin(); // Inialize I2C Hardware
  objVOCSensor_1.initAirQuality();
  if (objVOCSensor_1.begin() == false)
  {
    Serial.print("TVOC sensor error. Please check wiring. Freezing...");
    SGPXXStatVal = false;
  }
  else
  {
    Serial.println("VOC Initialized");
    SGPXXStatVal = true;
  }

  // CCS811 will not be availble during conditioning period
  sensor.setSGPXXStatus(false);
  // Define how much time(in MS) CCS811 should wait after
  // power up, ENV_DATA updation and BASE_LINE updation
  objVOCStartupDelay.setDelay(VOC_STARTUP_DELAY);
  objVOCEnvDataUpdationDelay.setDelay(VOC_ENVDATA_UPDATE_PRD);
  objVOCBaselineDailyUpdationDelay.setDelay(VOC_BASELINE_DAILY_UPDATE_PRD);
  objVOCBaselineUpdationDelay.setDelay(VOC_BASELINE_UPDATE_PRD);
}
*/
// Validate the calibration value and re-write, if failed
bool sensor_validateCaliHDC2080(String param)
{

  // Observed some times calibration is not written, to make sure just read it and compare.
  uint8_t cali_data = 0, i = 0;
  bool retValue = true;

  if (param.equals(VMP_TEMP))
  {

    objTempHumSensor.readTempCalibration(cali_data);
    while (((mapCalibrationInfo[VMP_TEMP] & 0xFF) != cali_data) &&
           (i < 5))
    {
      objTempHumSensor.calibrateTemp(mapCalibrationInfo[VMP_TEMP] & 0xFF);
      objTempHumSensor.readTempCalibration(cali_data);
      i++;
    }
    if (i >= 5)
    {
      Serial.printf("Failed to write Temp Cali of HDC2080. Old Val %x\r\n", cali_data);
      retValue = false;
    }
  }
  else if (param.equals(VMP_HUM))
  {

    objTempHumSensor.readHumCalibration(cali_data);
    while (((mapCalibrationInfo[VMP_HUM] & 0xFF) != cali_data) &&
           (i < 5))
    {
      objTempHumSensor.calibrateHum(mapCalibrationInfo[VMP_HUM] & 0xFF);
      objTempHumSensor.readHumCalibration(cali_data);
      i++;
    }
    if (i >= 5)
    {
      Serial.printf("Failed to write Hum Cali of HDC2080. Old Val %x\r\n", cali_data);
      retValue = false;
    }
  }
  return retValue;
}

void initHDC2080ForTRH()
{

  // Begin with a device reset
  // objTempHumSensor.reset(); //Not reqd, also suggested by TI support team

  // Set up the comfort zone (disabling these as we don't need threshold intr on HDC2080)
  // objTempHumSensor.setHighTemp(28);         // High temperature of 28C
  // objTempHumSensor.setLowTemp(22);          // Low temperature of 22C
  // objTempHumSensor.setHighHumidity(55);     // High humidity of 55%
  // objTempHumSensor.setLowHumidity(40);      // Low humidity of 40%

  // Configure Measurements
  objTempHumSensor.setMeasurementMode(TEMP_AND_HUMID); // Set measurements to temperature and humidity
  objTempHumSensor.setRate(ONE_HZ);                    // Set measurement frequency to 1 Hz
  objTempHumSensor.setTempRes(FOURTEEN_BIT);
  objTempHumSensor.setHumidRes(FOURTEEN_BIT);

  // Write Temp Offset info, make sure it is updated
  objTempHumSensor.calibrateTemp(mapCalibrationInfo[VMP_TEMP] & 0xFF);
  sensor_validateCaliHDC2080(VMP_TEMP);

  // Write Hum Offset info, make sure it is updated
  objTempHumSensor.calibrateHum(mapCalibrationInfo[VMP_HUM] & 0xFF);
  sensor_validateCaliHDC2080(VMP_HUM);

  // Begin measuring
  objTempHumSensor.triggerMeasurement();

  objHDCReadDelay.setDelay(HDC_SAMPLING_DELAY + 500); // Set higher delay compare to sampling delay
}

void initRTCDS1307()
{

  Serial.println("Autoprobing for a RTC...");
  if (rtc.autoprobe())
  {
    // Found something, hopefully a clock.
    Serial.print("Autoprobe found ");
    Serial.print(rtc.getDeviceName());
    Serial.print(" at 0x");
    Serial.println(rtc.getAddress(), HEX);
    RTCStatVal = true;
  }
  else
  {
    // Nothing found at any of the addresses listed.
    Serial.println("No RTCx found, check I2C");
    RTCStatVal = false;
  }
  // Enable the battery backup. This happens by default on the DS1307
  // but needs to be enabled on the MCP7941x.
  rtc.enableBatteryBackup();
  // rtc.clearPowerFailFlag();
  // Ensure the oscillator is running.
  rtc.startClock();
  rtc.setSQW(RTCx::freq32768Hz); // This is the frequency of the crystal too.
}

void sensor_initAllSensors()
{
  // As same I2C bus is used by multiple sensors, so initialization of Bus is only one
  // time and at the begining.
  Wire.begin();

  // Initialize ADS1115 with a specific gain in Vsensors.cpp.
  initADS1115();
  delay(100);

  // Initialise the CCS811 for TVOC & ECO2.
  initCCS811ForVOC();
  delay(100);
  //initSGPXXForVOC();
  //delay(100);

  // Initialise the HDC2080 (for Temp & RH) sensor
  initHDC2080ForTRH();

  // Initialise the RTC(DS1307)
  initRTCDS1307();
}

bool isCCS811Available()
{

  uint32_t baseline = 0;

  if (!sensor.isCCS811Ready())
  {
    if (objVOCStartupDelay.isTimeOut())
    {

      // Write the Base-line for the first time after power up.
      objVOCSensor.setBaseline(mapCalibrationInfo[VMP_VOC] & 0xFFFF);

      sensor.setCCS811Status(true);
      Serial.print("CCS811 is now ready for interaction, Base-line: ");
      Serial.println(baseline);
    }
  }
  return sensor.isCCS811Ready();
}

bool sensor_readBaselineOfCCS(uint16_t &baseLine)
{

  if (isCCS811Available())
  {
    baseLine = objVOCSensor.getBaseline();
    return 1;
  }
  else
  {
    return 0;
  }
}
/*
bool isSGPXXAvailable()
{
  uint32_t baseline = 0;
  if (!sensor.isSGPXXReady())
  {
    if (objVOCStartupDelay.isTimeOut())
    {
      // Write the Base-line for the first time after power up.
      objVOCSensor_1.setBaseline(400, mapCalibrationInfo[VMP_VOC] & 0xFFFF);
      sensor.setSGPXXStatus(true);
      Serial.print("SBG40 is now ready for interaction, Base-line: ");
      Serial.println(baseline);
    }
  }
  return sensor.isSGPXXReady();
}

bool sensor_readBaselineOfSGP(uint16_t &baseLine)
{

  if (isSGPXXAvailable())
  {
    baseLine = objVOCSensor_1.baselineTVOC;
    return 1;
  }
  else
  {
    return 0;
  }
}
*/
bool isTGS2602Available()
{

  if (!sensor.isTGS2602Ready())
  {
    if (objTGS2602StartupDelay.isTimeOut())
    {

      sensor.setTGS2602Status(true);
      Serial.print("TGS2602 is now ready for interaction, ADC(@ 20degC & 65%): ");
      Serial.println();
    }
  }
  return sensor.isTGS2602Ready();
}

bool isTGS2603Available()
{

  if (!sensor.isTGS2603Ready())
  {
    if (objTGS2603StartupDelay.isTimeOut())
    {

      sensor.setTGS2603Status(true);
      Serial.print("TGS2603(Ethanol) is now ready for interaction, ADC(@ 20degC & 65%): ");
      Serial.println();
    }
  }
  return sensor.isTGS2603Ready();
}

uint8_t isHDC2080DataAvailable()
{
  return (objTempHumSensor.readInterruptStatus());
}

// Initialize all Alert Buffers
void sensor_initAlertBuffers()
{
  uint32_t i;
  for (i = 0; i < ALERT_BUFFER_SIZE; i++)
  {
    alert_buffer_VOC[i] = 0.0;
    alert_buffer_TEMP[i] = 0.0;
    alert_buffer_RH[i] = 0.0;
  }
}

/*
//Copy to Alert Buffer
void copyToAlertBuffer(){

  if(ALERT_DELAY_TEMP > DATA_FILTRATION_DELAY) {
    alert_buffer_TEMP[alert_writeTempIndex] = SensorValue.getValue(TEMP_SENSOR);
    alert_writeTempIndex = ((alert_writeTempIndex + 1) % ALERT_BUFFER_SIZE);
  }

  if(ALERT_DELAY_HUM > DATA_FILTRATION_DELAY) {
    alert_buffer_RH[alert_writeRhIndex] = SensorValue.getValue(RH_SENSOR);
    alert_writeRhIndex = ((alert_writeRhIndex + 1) % ALERT_BUFFER_SIZE);
  }

  if(ALERT_DELAY_VOC > DATA_FILTRATION_DELAY) {
    alert_buffer_VOC[alert_writeVocIndex] = SensorValue.getValue(VOC_SENSOR);
    alert_writeVocIndex = ((alert_writeVocIndex + 1) % ALERT_BUFFER_SIZE);
  }

  if(ALERT_DELAY_NH3OD > DATA_FILTRATION_DELAY) {
    alert_buffer_NH3OD[alert_writeNh3ODIndex] = SensorValue.getValue(NH3OD_SENSOR);
    alert_writeNh3ODIndex = ((alert_writeNh3ODIndex + 1) % ALERT_BUFFER_SIZE);
  }
}
*/

// Read VOC Reading, if CCS811 is avilable
bool getVOCReadings()
{
  // If Data is available
  if (objVOCSensor.dataAvailable())
  {
    // Read the result from CCS811
    objVOCSensor.readAlgorithmResults();
    objVOCSensor.readRawData();
    return true;
  }
  return false;
}
/*
bool getVOCReadingsSPGXX()
{
  objVOCSensor_1.measureAirQuality();
  return true;
}
*/
float getNH3ODReadings(float &RS_F2602)
{

  float RSTempHumComp = 0.0f, RS_2602Temp = 0.0f, RS_2602Hum = 0.0f, HDCNH3Temp = 0.0f, HDNH3Hum = 0.0f;
  float volt_R9 = 0.0f, volt_R7 = 0.0f;

  ADSChannel0 = ads.readADC_SingleEnded(0); // Voltage is read from channel 'ZERO' of ADS1115 for NH3.

  if (ADSChannel0 > 0)
  {
    volt_R9 = ADSChannel0 * VOLT_PER_ADC;
    volt_R7 = volt_R9 * FACTOR1; // VRL or Voltage across the Load resistance.

    RS_F2602 = (float)RL * ((5.0f / volt_R7) - 1.0f); // RS value of the TGS2602 sensor.

    if (RS_F2602 < 0)
    {

      Serial.println("The obtained RS Value is negative for TGS2602.Please check...");
      NH3ODStatNegRs = true;
    }

    // Get the actual Temperature and Humidity for proper accuracy.
    sensor_getParamFromHDC2080(HDCNH3Temp, HDNH3Hum);

    // Now since we are doing ppm calculation, compensate the RS values by diving it with the respective factor obtained.
    RS_2602Temp = (RS_F2602 / nh3_getRSAt20degC(HDCNH3Temp));
    RS_2602Hum = (RS_F2602 / nh3_getRSAt65percentRH(HDNH3Hum));

    // Calculate the difference between the Raw RS and the compensated RS.
    RSTempHumComp = CalculateCompensatedRS(RS_F2602, RS_2602Temp, RS_2602Hum);

    Serial.printf("V(R9) = %0.2f,", volt_R9);
    Serial.printf("V(R7) = %0.2f,", volt_R7);
    Serial.printf("RS(Raw) = %0.2f,", RS_F2602);
    Serial.printf("RS(TempCompensated) = %0.2f,", RS_2602Temp);
    Serial.printf("RS(HumCompensated) = %0.2f", RS_2602Hum);
    Serial.println();
  }
  else
  {
    Serial.println("Negative value obtained from TGS 2602.");
    NH3ODStat = CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL;
  }
  return RSTempHumComp;
}

float getEthanolReadings(float &RS_F2603)
{

  float RSETempHumComp = 0.0f, RS2603Temp = 0.0f, RS2603Hum = 0.0f, HDCTempe = 0.0f, HDCHume = 0.0f,
        volt_E_R9 = 0.0f, volt_E_R7 = 0.0f;

  ADSChannel3 = ads.readADC_SingleEnded(3); // Voltage is read from channel 'THREE' of ADS1115 for Ethanol.

  if (ADSChannel3 > 0)
  {
    volt_E_R9 = ADSChannel3 * VOLT_PER_ADC;
    volt_E_R7 = volt_E_R9 * FACTOR1; // VRL or Voltage across the Load resistance.

    RS_F2603 = (float)RL * ((5.0f / volt_E_R7) - 1.0f); // RS value of the TGS2603 sensor.

    if (RS_F2603 < 0)
    {

      Serial.println("The obtained RS Value is negative for TGS2603.Please check...");
      EthanolStatNegRs = true;
    }

    // Get the actual Temperature and Humidity for proper accuracy.
    sensor_getParamFromHDC2080(HDCTempe, HDCHume);

    // Now since we are doing ppm calculation, compensate the RS values by diving it with the respective factor obtained.
    RS2603Temp = (RS_F2603 / ethanol_getRSAt20degC(HDCTempe));
    RS2603Hum = (RS_F2603 / ethanol_getRSAt65percentRH(HDCHume));

    // Calculate the difference between the Raw RS and the compensated RS.
    RSETempHumComp = CalculateCompensatedRS(RS_F2603, RS2603Temp, RS2603Hum);

    Serial.printf("V(R9_E) = %0.2f,", volt_E_R9);
    Serial.printf("V(R7_E) = %0.2f,", volt_E_R7);
    Serial.printf("RS(Raw_E) = %0.2f,", RS_F2603);
    Serial.printf("RS(ETempCompensated) = %0.2f,", RS2603Temp);
    Serial.printf("RS(EHumCompensated) = %0.2f,", RS2603Hum);
    Serial.printf("RS(EOverallCompensated) = %0.2f", RSETempHumComp);
    Serial.println();
  }
  else
  {
    Serial.println("Negative value obtained from TGS 2603.");
    EthanolStat = CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL;
  }

  return RSETempHumComp;
}

// Calculate R0 @ 20degC and 65% RH of TGS2602 as well TGS2603.
// Will Call during Calibration only.
float sensor_readR0ofTGS(String inSensorType)
{

  float CurrentRS = 0.0f, CurrentRSTemp = 0.0f, CurrentRSHum = 0.0f,
        CurrentRSCorrected = 0.0f, TempCalifac = 0.0f, HumCalifac = 0.0f,
        CalibTemp = 0.0f, CalibHum = 0.0f;
  sensor_getParamFromHDC2080(CalibTemp, CalibHum);

  if (isTGS2602Available() && inSensorType.equals(VMP_NH3OD))
  {
    // Get the current value of RS.
    getNH3ODReadings(CurrentRS);
    TempCalifac = nh3_getRSAt20degC(CalibTemp);
    HumCalifac = nh3_getRSAt65percentRH(CalibHum);
  }
  else if (isTGS2603Available() && inSensorType.equals(VMP_ETHANOL))
  {
    // Get the current value of RS.
    getEthanolReadings(CurrentRS);
    TempCalifac = ethanol_getRSAt20degC(CalibTemp);
    HumCalifac = ethanol_getRSAt65percentRH(CalibHum);
  }
  else
  {
    // TGS 2603/2602 Sensor not available then return from here only.
    return -1.0f;
  }

  // Since we are going from ambient to 20 degrees we need to Divide and store it in SPIFFS.
  CurrentRSTemp = (CurrentRS / TempCalifac);
  CurrentRSHum = (CurrentRS / HumCalifac);
  CurrentRSCorrected = CalculateCompensatedRS(CurrentRS, CurrentRSTemp, CurrentRSHum);
  return CurrentRSCorrected;
  // For calibration purpose the current RS is passed to CLI.
}

//  Get readings from all sensors
String sensor_getSensorReadings(std::map<String, double> &inMapSensorReadings)
{

  // float *ptr_refR0, refR0=0.0f;
  float lflCurrentTemperatureValue = 0, lflCurrentHumidityValue = 0;
  uint8_t temp_cali = 0, hum_cali = 0;

  // Real time VOC readings, after conditioning period
  if (isCCS811Available())
  {
    if (getVOCReadings())
    {
      // Save it into a circular buffer for filtering
      buffer_VOC[writeVocIndex] = objVOCSensor.getTVOC();
      writeVocIndex = ((writeVocIndex + 1) % BUFFER_SIZE);
      // Fill it in an alert buffer too.
      alert_buffer_VOC[alert_writeVocIndex] = objVOCSensor.getTVOC();
      alert_writeVocIndex = ((alert_writeVocIndex + 1) % ALERT_BUFFER_SIZE);

      buffer_CO2[writeCo2Index] = objVOCSensor.getCO2();
      writeCo2Index = ((writeCo2Index + 1) % BUFFER_SIZE);

      Serial.print("TVOC Reading: ");
      Serial.print(objVOCSensor.getTVOC());
      Serial.print(",");
      Serial.print("eCO2 Reading: ");
      Serial.print(objVOCSensor.getCO2());
      Serial.print(",");
      Serial.print("RAW-DATA: ");
      Serial.print(objVOCSensor.getRawData());
      Serial.println();
    }
  }
  /*if (isSGPXXAvailable())
  {
    if (getVOCReadingsSPGXX())
    {
      // Save it into a circular buffer for filtering
      buffer_VOC[writeVocIndex] = objVOCSensor_1.TVOC;
      writeVocIndex = ((writeVocIndex + 1) % BUFFER_SIZE);
      // Fill it in an alert buffer too.
      alert_buffer_VOC[alert_writeVocIndex] = objVOCSensor_1.TVOC;
      alert_writeVocIndex = ((alert_writeVocIndex + 1) % ALERT_BUFFER_SIZE);

      buffer_CO2[writeCo2Index] = objVOCSensor_1.CO2;
      writeCo2Index = ((writeCo2Index + 1) % BUFFER_SIZE);

      Serial.print("TVOC Reading: ");
      Serial.print(objVOCSensor_1.TVOC);
      Serial.print(",");
      Serial.print("eCO2 Reading: ");
      Serial.print(objVOCSensor_1.CO2);
      Serial.println();
    }
  }*/

  if (isTGS2603Available() && objTGS2603ReadDelay.isTimeOut())
  {

    objTGS2603ReadDelay.setDelay(ETHANOL_SAMPLING_DELAY);
    float RSEthanol = 0.0f, Rs_EthanolComp = 0.0f, HDCEthppmT = 0.0f, HDCEthppmH = 0.0f;
    Rs_EthanolComp = (float)getEthanolReadings(RSEthanol);

    // Get the actual Temperature and Humidity for proper accuracy.
    sensor_getParamFromHDC2080(HDCEthppmT, HDCEthppmH);
    ppm_Ethanol = Ethanol_calculatePPM(Rs_EthanolComp, HDCEthppmT, HDCEthppmH);
    if (ppm_Ethanol >= 0.0f)
    {
      // Read RS from TGS2602, calculate PPM and save into buffer.
      buffer_Ethanol[writeEthanolIndex] = ppm_Ethanol;
      writeEthanolIndex = ((writeEthanolIndex + 1) % BUFFER_SIZE);
    }
    else
    {
      // Suppose we get a negative return value if calibration is not done or if Map is empty, then consider the PPM to be 'ZERO'.
      ppm_Ethanol = 0.0f;
    }
  }

  if (isTGS2602Available() && objTGS2602ReadDelay.isTimeOut())
  {
    objTGS2602ReadDelay.setDelay(NH3OD_SAMPLING_DELAY);
    float rs_NH3ODComp = 0.0f, RsOdour, ppm_NH3OD = 0.0f, RSVoidCrossSensitivity = 0.0f;
    float HDCNH3ppmT = 0.0f, HDCNH3ppmH = 0.0f;
    rs_NH3ODComp = (float)getNH3ODReadings(RsOdour);

    // Save current compensated RS into a circular buffer for filtering.
    buffer_OD_RS[writeODRSIndex] = rs_NH3ODComp;
    writeODRSIndex = ((writeODRSIndex + 1) % BUFFER_SIZE);

    // Get the actual Temperature and Humidity for proper accuracy.
    sensor_getParamFromHDC2080(HDCNH3ppmT, HDCNH3ppmH);

    ppm_NH3OD = nh3_calculatePPM(rs_NH3ODComp, HDCNH3ppmT, HDCNH3ppmH);
    if (ppm_NH3OD >= 0.0f)
    {
      RSVoidCrossSensitivity = CheckForCrossSensitivity(ppm_NH3OD, ppm_Ethanol);
      // If value obtained is below 0 ie. -1 meaning the condition for cross-sensitivity is not satisfied, hence the original RS Value of Ammonia is taken.
      // But if its above 'Zero' meaning there was Coss-Sensitivity, hence compensating it with Highest Ammonia RS Value Stored in the map and calculating Ammonia PPM again.
      // Even though Ethanol comes down very soon, Ammonia may not. Hence keep on compensating till 2 minutes is done.
      if (RSVoidCrossSensitivity >= 0.0f || (!objNH3CompensationDelay.isTimeOut()))
      {

        rs_NH3ODComp = RSVoidCrossSensitivity;

        if (!objNH3CompensationDelay.isTimeOut())
        {

          rs_NH3ODComp = mapCalibrationSensorValueInfo[VMP_NH3OD_RS];
          Serial.printf("Yet to timeout. Still compensating Ammonia with %0.3f.\r\n", rs_NH3ODComp);
        }

        Serial.printf("RS Calculated for Cross Sensitivity is not Original value but compensated with Highest RS. %0.3f\r\n", rs_NH3ODComp);

        Serial.println("Calculating the Ammonia PMM Val again.");

        ppm_NH3OD = nh3_calculatePPM(rs_NH3ODComp, HDCNH3ppmT, HDCNH3ppmH);
      }
      else
      {

        Serial.println("Ammonia Value is not compensated.");
      }

      // Read RS from TGS2602, calculate PPM and save into buffer.
      buffer_NH3OD[writeNh3ODIndex] = ppm_NH3OD;
      writeNh3ODIndex = ((writeNh3ODIndex + 1) % BUFFER_SIZE);

      alert_buffer_NH3OD[alert_writeNh3ODIndex] = ppm_NH3OD;
      alert_writeNh3ODIndex = ((alert_writeNh3ODIndex + 1) % ALERT_BUFFER_SIZE);
    }
    else
    {
      // Suppose we get a negative return value if calibration is not done or if Map is empty, then consider the PPM to be 'ZERO'.
      ppm_NH3OD = 0.0f;
    }
  }

  // Check if HDC2080 Time-out period is over.
  if (objHDCReadDelay.isTimeOut())
  { // HDC_SAMPLING_DELAY
    // TO DO: Check the working of this Flag.
    // uint8_t hdc2080Flag= isHDC2080DataAvailable();
    // if(hdc2080Flag != 0) {

    // Read Temp & hum calibration value
    objTempHumSensor.readTempCalibration(temp_cali);
    objTempHumSensor.readHumCalibration(hum_cali);

    // It is observed sometimes HDC2080 lost calibration, if it lost
    if (((mapCalibrationInfo[VMP_TEMP] & 0xFF) == temp_cali) &&
        ((mapCalibrationInfo[VMP_HUM] & 0xFF) == hum_cali))
    {
      // if(hdc2080Flag == 0x80) {
      // Read 4 bytes from HDC2080
      objTempHumSensor.readHDC2080TRH();

      // Get Temp & RH
      lflCurrentTemperatureValue = objTempHumSensor.getTemp();
      lflCurrentHumidityValue = objTempHumSensor.getHumidity();

      // Save it into a circular buffer for filtering
      buffer_TEMP[writeTempIndex] = lflCurrentTemperatureValue;
      writeTempIndex = ((writeTempIndex + 1) % BUFFER_SIZE);
      // Fill it in an alert buffer too.
      alert_buffer_TEMP[alert_writeTempIndex] = lflCurrentTemperatureValue;
      alert_writeTempIndex = ((alert_writeTempIndex + 1) % ALERT_BUFFER_SIZE);

      // Save it into a circular buffer for filtering
      buffer_RH[writeRhIndex] = lflCurrentHumidityValue;
      writeRhIndex = ((writeRhIndex + 1) % BUFFER_SIZE);
      // Fill it in an alert buffer too.
      alert_buffer_RH[alert_writeRhIndex] = lflCurrentHumidityValue;
      alert_writeRhIndex = ((alert_writeRhIndex + 1) % ALERT_BUFFER_SIZE);

      Serial.print("TEMP: ");
      Serial.print(lflCurrentTemperatureValue);
      Serial.print(",");
      Serial.print("RH: ");
      Serial.println(lflCurrentHumidityValue);
      Serial.println();

      // Set HDC2080 Time-out period again
      objHDCReadDelay.setDelay(HDC_SAMPLING_DELAY);

      // } else {
      //   // Improper response from HDC2080,
      //   // Need to Reset HDC2080 and Config again
      //   objTempHumSensor.reset();
      //   initHDC2080ForTRH();
      // }
    }
    else
    {
      if ((mapCalibrationInfo[VMP_TEMP] & 0xFF) != temp_cali)
      {
        Serial.println("Found wrong Temp cali in run time, doing re-calibration.");
        sensor_validateCaliHDC2080(VMP_TEMP);
      }

      if ((mapCalibrationInfo[VMP_HUM] & 0xFF) != hum_cali)
      {
        Serial.println("Found wrong Hum cali in run time, doing re-calibration.");
        sensor_validateCaliHDC2080(VMP_HUM);
      }
    }
    // }
  }

  // TODO, need to check the purpose of Sanity Interval
  if (mapSensorSanityInterval.find(VMP_TEMP) == mapSensorSanityInterval.end())
  {
    Serial.println("Last Sensor Data Sanity ignored Time is not present in mapSensorSanityInterval for Measured Param Temperature. Adding Current millis() in map.");

    // Note the current time (for the very first Time) in the sensor sanity for the MeasuredParam.
    mapSensorSanityInterval[VMP_TEMP] = millis();
  }

  if (mapSensorSanityInterval.find(VMP_HUM) == mapSensorSanityInterval.end())
  {
    Serial.println("Last Sensor Data Sanity ignored Time is not present in mapSensorSanityInterval for Measured Param Humidity. Adding Current millis() in map.");

    // Note the current time (for the very first Time) in the sensor sanity for the MeasuredParam.
    mapSensorSanityInterval[VMP_HUM] = millis();
  }

  return STATUS_SUCCESS;
}

void calculateAvgValue()
{

  // Calculate Avg VOC
  Serial.println("Calculating Avg VOC: ");
  SensorValue.setValue(VOC_SENSOR, calculation_calAvgAfterFilter(buffer_VOC, BUFFER_SIZE,
                                                                 writeVocIndex, readVocIndex));
  // Calculate Avg Temp
  Serial.print("Calculating Avg Temp: ");
  SensorValue.setValue(TEMP_SENSOR, calculation_calAvgAfterFilter(buffer_TEMP, BUFFER_SIZE,
                                                                  writeTempIndex, readTempIndex));
  // Calculate Avg RH
  Serial.print("Calculating Avg RH: ");
  SensorValue.setValue(RH_SENSOR, calculation_calAvgAfterFilter(buffer_RH, BUFFER_SIZE,
                                                                writeRhIndex, readRhIndex));
  // Calculate Avg NH3
  Serial.print("Calculating Avg NH3OD: ");
  // Buffer contains PPM of NH3OD.
  SensorValue.setValue(NH3OD_SENSOR, calculation_calAvgAfterFilter(buffer_NH3OD, BUFFER_SIZE,
                                                                   writeNh3ODIndex, readNh3ODIndex));
  // Calculate Avg RS of Ammonia
  Serial.print("Calculating Avg RS of Ammonia: ");
  // Buffer contains Compensated RS Values of NH3OD.
  SensorValue.setValue(OD_SENSOR_RS, calculation_calAvgAfterFilter(buffer_OD_RS, BUFFER_SIZE,
                                                                   writeODRSIndex, readODRSIndex));
  // Calculate Avg RS of Ethanol
  Serial.print("Calculating Avg PPM of Ethanol: ");
  // Buffer contains Compensated RS Values of Ethanol.
  SensorValue.setValue(SENSOR_2603, calculation_calAvgAfterFilter(buffer_Ethanol, BUFFER_SIZE,
                                                                  writeEthanolIndex, readEthanolIndex));
}

void sensor_calculateNCollectReadings(std::map<String, double> &inMapSensorReadings)
{

  // Calculate Avg value of all sensor readings.
  calculateAvgValue();

  // Initially clear the Map.Very Important step.Because we are sending old backup data too. We will require this while sending data after Alerts.
  // We are sending the 'Trigger Point' of Alerts along with the most recently calculated averaged out data.
  inMapSensorReadings.clear();

  // Copy value of VOC into MAP, if CCS811 is ready only
  if (isCCS811Available())
  {
    inMapSensorReadings[VMP_VOC] = SensorValue.getValue(VOC_SENSOR);
  }

  // Copy NH3OD, if TGS2602 is ready
  if (isTGS2602Available())
  {
    inMapSensorReadings[VMP_NH3OD] = SensorValue.getValue(NH3OD_SENSOR);
    inMapSensorReadings[VMP_NH3OD_RS] = SensorValue.getValue(OD_SENSOR_RS);
  }

  // Copy TEMP & RH into MAP,
  inMapSensorReadings[VMP_TEMP] = SensorValue.getValue(TEMP_SENSOR);
  inMapSensorReadings[VMP_HUM] = SensorValue.getValue(RH_SENSOR);

  UpdateHighestResistanceValues();

  // Since Average Sensor Values have been calculated, the flag is made high to indicate this.
  isAverageSensorValuesCalcualted = true;
}

// These Average Values stored in class will be re-inserted into the map as old backup data can be stored and mix with alerts data if alerts are sent just
// prior to backlog. Hence clear the map again and insert the values so that alerts data won't be mismatched.
void sensor_reinsertAveragedOutValues(std::map<String, double> &inMapSensorReadings)
{

  // Initially clear the Map.Very Important step.Because we are sending old backup data too. We will require this while sending data after Alerts.
  // So the inMapSensorReadings map might be having old backlog data so clear the map initially and reinsert the values.
  // We are sending the 'Trigger Point' of Alerts along with the most recently calculated averaged out data.
  inMapSensorReadings.clear();

  // Copy value of VOC into MAP, if CCS811 is ready only
  if (isCCS811Available())
  {
    inMapSensorReadings[VMP_VOC] = SensorValue.getValue(VOC_SENSOR);
  }

  // Copy NH3OD, if TGS2602 is ready
  if (isTGS2602Available())
  {
    inMapSensorReadings[VMP_NH3OD] = SensorValue.getValue(NH3OD_SENSOR);
    inMapSensorReadings[VMP_NH3OD_RS] = SensorValue.getValue(OD_SENSOR_RS);
  }

  // Copy TEMP & RH into MAP,
  inMapSensorReadings[VMP_TEMP] = SensorValue.getValue(TEMP_SENSOR);
  inMapSensorReadings[VMP_HUM] = SensorValue.getValue(RH_SENSOR);
}

void UpdateHighestResistanceValues()
{

  float HighestAmmoniaRSVal = 0.0f;

  HighestAmmoniaRSVal = SensorValue.getValue(OD_SENSOR_RS);

  if (HighestAmmoniaRSVal > mapCalibrationSensorValueInfo[VMP_NH3OD_RS])
  {

    Serial.printf("Highest value of Ammonia RS Obtained. Replacing it with %0.3f\r\n", HighestAmmoniaRSVal);

    mapCalibrationSensorValueInfo[VMP_NH3OD_RS] = HighestAmmoniaRSVal;

    writeCalibDoubleValuesToFile(mapCalibrationSensorValueInfo);
  }
}

float CheckForCrossSensitivity(float inflppmAmmonia, float inflppmEthanol)
{

  // % is value obtained/overall(30 ppm) multiplied by 100.
  float flx1 = inflppmAmmonia;
  float fly1 = 30;
  float flx2 = inflppmEthanol;
  float fly2 = 10;

  float AmmoniaDiffPercent = (flx1 / fly1) * 100;
  float EthanolDiffPercent = (flx2 / fly2) * 100;

  flAvgAmmoniaRSVal = mapCalibrationSensorValueInfo[VMP_NH3OD_RS];

  // Compare the percentages.
  // If Ethanol is greater, then take Stored Avg value of Ammonia means its volatile gas spike else if less, then it should satisfy 5.0% only.
  // Calculate the crosssensitivity only when all the Params are available. Since these are stored in SPIFFS it will be empty for a fresh device only.
  if (mapCalibrationSensorValueInfo[VMP_NH3OD_RS] == 0)
  {

    Serial.println("Highest Ammonia RS Value not stored yet.");

    return -1.0f;
  }
  else if (EthanolDiffPercent >= AmmoniaDiffPercent)
  {

    Serial.println("Ethanol Reacted more than Ammonia, compensating it by Sending Highest Avg RS Value of Ammonia.");
    return flAvgAmmoniaRSVal;
  }
  else if (EthanolDiffPercent < AmmoniaDiffPercent)
  {

    // If less than 5.0 then Ammonia is actually detected so keep RS as it is by sending Zero, else if its not below 5.0 meaning
    // due to spike of TVOC Ethanol might have got less reacted than Ammonia so 'compensate' and send Avg Ammonia RS Value.
    (EthanolDiffPercent <= 5.0) ? (flAvgAmmoniaRSVal = -1.0) : flAvgAmmoniaRSVal;

    Serial.printf("Ethanol Reacted less than Ammonia, compensating it by %0.3f\r\n", flAvgAmmoniaRSVal);
    if (flAvgAmmoniaRSVal > 0.0f)
    {

      Serial.println("Starting a Timer of 2 minutes.");
      objNH3CompensationDelay.setDelay(NH3OD_COMPENSATION_DELAY);
    }

    return flAvgAmmoniaRSVal;
  }

  return 0.0f;
}

bool isAirClean(uint32_t tVOC, uint32_t CO2)
{

  if ((tVOC < 5) && (CO2 < 500))
  {
    return true;
  }
  return false;
}

void sensor_doVOCCalibration()
{

  // uint16_t baseline;
  // float rh, temp;

  // If CCS811 is available for interaction, then only consider
  if (isCCS811Available())
  {

    // Write Temp & RH in CCS811 at a certain interval
    if (objVOCEnvDataUpdationDelay.isTimeOut())
    {
      objVOCEnvDataUpdationDelay.setDelay(VOC_ENVDATA_UPDATE_PRD);

      // Get the avg Temp and avg RH
      // temp = SensorValue.getValue(TEMP_SENSOR);
      // rh = SensorValue.getValue(RH_SENSOR);

      sensor_getParamFromHDC2080(TemVal, HumVal);

      // Write the avg values into ENV_DATA Reg of CCS811
      if (objVOCSensor.setEnvironmentalData(HumVal, TemVal) ==
          CCS811Core::CCS811_Stat_SUCCESS)
      {
        Serial.print("Written ENV-DATA to CCS811");
      }
    }

    // Read baseline produced by VOC itself when cleanest air is encountered ie., at highest resistance in 24hrs period.
    // Do it only for start 7 days every 24 hrs and then update it after every 7 days periodically.
    if (mapCalibrationInfo[VOC_COUNT] <= 7)
    {
      if (objVOCBaselineDailyUpdationDelay.isTimeOut())
      {
        objVOCBaselineDailyUpdationDelay.setDelay(VOC_BASELINE_DAILY_UPDATE_PRD);

        CCSbaseLine = objVOCSensor.getBaseline();
        mapCalibrationInfo[VMP_VOC] = CCSbaseLine;
        writeCalibrationInfoToFile(mapCalibrationInfo);
        CCScount++;
        mapCalibrationInfo[VOC_COUNT] = CCScount;
        writeCalibrationInfoToFile(mapCalibrationInfo);
        Serial.println("24 hrs complete.Saving the clean air captured baseline onto a Map.");
      }
    }

    // Re-Write BASE-Line of CCS811 at long interval (@ 8days)
    if (objVOCBaselineUpdationDelay.isTimeOut())
    {
      objVOCBaselineUpdationDelay.setDelay(VOC_BASELINE_UPDATE_PRD);

      CCSbaseLine = objVOCSensor.getBaseline();
      mapCalibrationInfo[VMP_VOC] = CCSbaseLine;
      writeCalibrationInfoToFile(mapCalibrationInfo);
      Serial.println("7 days complete.Saving the clean air captured baseline onto a Map.");
    }
  }
}

bool sensor_readTempCali(uint8_t &data)
{
  return objTempHumSensor.readTempCalibration(data);
}
bool sensor_readHumCali(uint8_t &data)
{
  return objTempHumSensor.readHumCalibration(data);
}

uint8_t sensor_calibrateSensor(String sensor, float currentValue, float refValue)
{

  uint8_t newOffset = 0;
  uint8_t regOffset = 0;

  uint32_t i;
  float diff = 0.0f, existingOffset = 0.0f, retvalue = 0.0f;

  if (currentValue == refValue)
  {
    return 0;
  }

  if (sensor.equals(VMP_TEMP))
  {

    // Read existing Offset value from HDC2080
    if (objTempHumSensor.readTempCalibration(regOffset))
    {
      // Covert the Reg Offset value into float value
      for (i = 0; i < 8; i++)
      {
        if (regOffset & (1 << i))
        {
          switch (i)
          {
          case 0:
            existingOffset += TEMP_CALI_BIT_0;
            break;
          case 1:
            existingOffset += TEMP_CALI_BIT_1;
            break;
          case 2:
            existingOffset += TEMP_CALI_BIT_2;
            break;
          case 3:
            existingOffset += TEMP_CALI_BIT_3;
            break;
          case 4:
            existingOffset += TEMP_CALI_BIT_4;
            break;
          case 5:
            existingOffset += TEMP_CALI_BIT_5;
            break;
          case 6:
            existingOffset += TEMP_CALI_BIT_6;
            break;
          case 7:
            existingOffset -= TEMP_CALI_BIT_7;
            break;
          default:
            break;
          }
        }
      }
      Serial.printf("Current Offset in Reg %x %f\r\n", regOffset, existingOffset);
    }
    else
    {
      Serial.printf("Not able to read from Temp Calibration Reg\r\n");
    }

    diff = refValue - currentValue + existingOffset;

    // If current reading is higher than reference
    if (diff < 0)
    {
      // Set the bit 7 (-20.62 degC)
      newOffset = 0x80;
      diff += TEMP_CALI_BIT_7;
      Serial.printf("Adjusted -ve diff, Offset %x, diff %f\r\n", newOffset, diff);
    }

    if (diff > TEMP_CALI_BIT_6)
    {
      newOffset |= (1 << 6);
      diff -= TEMP_CALI_BIT_6;
      Serial.printf("Adjusted 10.32, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > TEMP_CALI_BIT_5)
    {
      newOffset |= (1 << 5);
      diff -= TEMP_CALI_BIT_5;
      Serial.printf("Adjusted 5.16, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > TEMP_CALI_BIT_4)
    {
      newOffset |= (1 << 4);
      diff -= TEMP_CALI_BIT_4;
      Serial.printf("Adjusted 2.58, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > TEMP_CALI_BIT_3)
    {
      newOffset |= (1 << 3);
      diff -= TEMP_CALI_BIT_3;
      Serial.printf("Adjusted 1.28, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > TEMP_CALI_BIT_2)
    {
      newOffset |= (1 << 2);
      diff -= TEMP_CALI_BIT_2;
      Serial.printf("Adjusted 0.64, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > TEMP_CALI_BIT_1)
    {
      newOffset |= (1 << 1);
      diff -= TEMP_CALI_BIT_1;
      Serial.printf("Adjusted 0.32, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > (TEMP_CALI_BIT_0 / 2))
    {
      newOffset |= (1 << 0);
      Serial.printf("Adjusted 0.16 for more accuracy Offset %x\r\n", newOffset);
    }

    objTempHumSensor.calibrateTemp(newOffset);
    retvalue = newOffset;
  }
  else if (sensor.equals(VMP_HUM))
  {
    // Read existing Offset value from HDC2080
    if (objTempHumSensor.readHumCalibration(regOffset))
    {

      for (i = 0; i < 8; i++)
      {
        if (regOffset & (1 << i))
        {
          switch (i)
          {
          case 0:
            existingOffset += HUM_CALI_BIT_0;
            break;
          case 1:
            existingOffset += HUM_CALI_BIT_1;
            break;
          case 2:
            existingOffset += HUM_CALI_BIT_2;
            break;
          case 3:
            existingOffset += HUM_CALI_BIT_3;
            break;
          case 4:
            existingOffset += HUM_CALI_BIT_4;
            break;
          case 5:
            existingOffset += HUM_CALI_BIT_5;
            break;
          case 6:
            existingOffset += HUM_CALI_BIT_6;
            break;
          case 7:
            existingOffset -= HUM_CALI_BIT_7;
            break;
          default:
            break;
          }
        }
      }
      Serial.printf("Current Offset in RH Reg %x %f\r\n", regOffset, existingOffset);
    }
    else
    {
      Serial.printf("Not able to read from RH Calibration Reg\r\n");
    }

    diff = refValue - currentValue + existingOffset;

    // If current reading is higher than reference
    if (diff < 0)
    {
      // Set the bit 7 (-20.62 degC)
      newOffset = 0x80;
      diff += HUM_CALI_BIT_7;
      Serial.printf("Adjusted -ve diff, Offset %x, diff %f\r\n", newOffset, diff);
    }

    if (diff > HUM_CALI_BIT_6)
    {
      newOffset |= (1 << 6);
      diff -= HUM_CALI_BIT_6;
      Serial.printf("Adjusted 10.32, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > HUM_CALI_BIT_5)
    {
      newOffset |= (1 << 5);
      diff -= HUM_CALI_BIT_5;
      Serial.printf("Adjusted 5.16, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > HUM_CALI_BIT_4)
    {
      newOffset |= (1 << 4);
      diff -= HUM_CALI_BIT_4;
      Serial.printf("Adjusted 2.58, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > HUM_CALI_BIT_3)
    {
      newOffset |= (1 << 3);
      diff -= HUM_CALI_BIT_3;
      Serial.printf("Adjusted 1.28, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > HUM_CALI_BIT_2)
    {
      newOffset |= (1 << 2);
      diff -= HUM_CALI_BIT_2;
      Serial.printf("Adjusted 0.64, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > HUM_CALI_BIT_1)
    {
      newOffset |= (1 << 1);
      diff -= HUM_CALI_BIT_1;
      Serial.printf("Adjusted 0.32, Offset %x, diff %f\r\n", newOffset, diff);
    }
    if (diff > (HUM_CALI_BIT_0 / 2))
    {
      newOffset |= (1 << 0);
      Serial.printf("Adjusted 0.16 for more accuracy Offset %x\r\n", newOffset);
    }

    objTempHumSensor.calibrateHum(newOffset);
    retvalue = newOffset;
  }
  return retvalue;
}

// This isHDC2080DataAvailable() gives ZERO on 100% Humidity and hence we get HDC2080 not available.
// Hence, we are excluding this Flag and taking the readings directly.
/*
bool sensor_getParamFromHDC2080(float &temp, float &hum) {

  if(isHDC2080DataAvailable() == 0x80){
      // Read 4 bytes from HDC2080
      objTempHumSensor.readHDC2080TRH();
  } else {
    Serial.println("HDC2080 not available");
    return 0;
  }

  temp = objTempHumSensor.getTemp();
  hum = objTempHumSensor.getHumidity();

  return 1;
}
*/

bool sensor_getParamFromHDC2080(float &temp, float &hum)
{

  // Read 4 bytes from HDC2080
  objTempHumSensor.readHDC2080TRH();
  temp = objTempHumSensor.getTemp();
  hum = objTempHumSensor.getHumidity();
  return 1;
}

void getSensorInfo(String &TempStatretVal, String &HumStatretVal, String &CCSStatretVal, String &NH3ODStatretVal, String &EthanolStatretVal, String &RTCStatretVal)
{

  float RS_2602 = 0.0f;
  float RS_2603 = 0.0f;

  sensor_getParamFromHDC2080(TemVal, HumVal);

  // Space to avoid confusion for Hardware Testing related SysOut.
  Serial.println();

  // Check Temperature Status First.
  if ((TemVal <= 0))
  {

    TempStatretVal = STATUS_NOT_OK;
    Serial.println("Temperature value from HDC2080 is either 0 or -40");
  }
  else
  {

    TempStatretVal = STATUS_OK;
    Serial.println("Temperature value from HDC2080 is OK.");
  }

  // Check Humidity Status Next.
  if ((HumVal <= 0))
  {

    HumStatretVal = STATUS_NOT_OK;
    Serial.println("Humidity value from HDC2080 is either 0 or some negative value");
  }
  else
  {

    HumStatretVal = STATUS_OK;
    Serial.println("Humidity value from HDC2080 is OK.");
  }

  // Check CCS811 Status Next.
  if (!CCSStatVal)
  {

    CCSStatretVal = STATUS_NOT_OK;
    Serial.println("CCS811 Sensor has not been initialized");
  }
  else if (CCSStatVal)
  {

    CCSStatretVal = STATUS_OK;
    Serial.println("CCS811 Sensor has been initialized");
  }

  // Check NH3OD Status Next.
  getNH3ODReadings(RS_2602);
  if ((NH3ODStat == CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL) || (NH3ODStatNegRs == true))
  {

    NH3ODStatretVal = STATUS_NOT_OK;
    Serial.println("Negative RS/Voltage value obtained from TGS 2602");
    // Clear the flag statuses.
    NH3ODStat = "";
    NH3ODStatNegRs = false;
  }
  else
  {

    NH3ODStatretVal = STATUS_OK;
    Serial.println("Positive RS and Voltage value obtained from TGS 2602");
  }

  // Check ETHNL Status Next.
  getEthanolReadings(RS_2603);
  if ((EthanolStat == CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL) || (EthanolStatNegRs == true))
  {

    EthanolStatretVal = STATUS_NOT_OK;
    Serial.println("Negative RS/Voltage value obtained from TGS 2603");
    // Clear the flag statuses.
    EthanolStat = "";
    EthanolStatNegRs = false;
  }
  else
  {

    EthanolStatretVal = STATUS_OK;
    Serial.println("Positive RS and Voltage value obtained from TGS 2603");
  }

  // Check RTC Status Next.
  if (!RTCStatVal)
  {

    RTCStatretVal = STATUS_NOT_OK;
    Serial.println("RTC not found.Check Hardware.");
  }
  else if (RTCStatVal)
  {

    RTCStatretVal = STATUS_OK;
    Serial.println("RTC found.");
  }
}

float *sensor_getBufferBasedOnType(String inParamNameType)
{

  if (inParamNameType.equals(VMP_NO2))
  {
  }
  else if (inParamNameType.equals(VMP_O3))
  {
  }
  else if (inParamNameType.equals(VMP_SO2))
  {
  }
  else if (inParamNameType.equals(VMP_VOC))
  {
    return alert_buffer_VOC;
  }
  else if (inParamNameType.equals(VMP_CO))
  {
  }
  else if (inParamNameType.equals(VMP_NH3))
  {
  }
  else if (inParamNameType.equals(VMP_CO2))
  {
  }
  else if (inParamNameType.equals(VMP_HUM))
  {
    return alert_buffer_RH;
  }
  else if (inParamNameType.equals(VMP_PM10))
  {
  }
  else if (inParamNameType.equals(VMP_PM25))
  {
  }
  else if (inParamNameType.equals(VMP_TEMP))
  {
    return alert_buffer_TEMP;
  }
  else if (inParamNameType.equals(VMP_NH3OD))
  {
    return alert_buffer_NH3OD;
  }
  return 0;
}

uint32_t sensor_getWriteIndexBasedOnType(String inParamNameType)
{

  if (inParamNameType.equals(VMP_NO2))
  {
  }
  else if (inParamNameType.equals(VMP_O3))
  {
  }
  else if (inParamNameType.equals(VMP_SO2))
  {
  }
  else if (inParamNameType.equals(VMP_VOC))
  {
    return alert_writeVocIndex;
  }
  else if (inParamNameType.equals(VMP_CO))
  {
  }
  else if (inParamNameType.equals(VMP_NH3))
  {
  }
  else if (inParamNameType.equals(VMP_CO2))
  {
  }
  else if (inParamNameType.equals(VMP_HUM))
  {
    return alert_writeRhIndex;
  }
  else if (inParamNameType.equals(VMP_PM10))
  {
  }
  else if (inParamNameType.equals(VMP_PM25))
  {
  }
  else if (inParamNameType.equals(VMP_TEMP))
  {
    return alert_writeTempIndex;
  }
  else if (inParamNameType.equals(VMP_NH3OD))
  {
    return alert_writeNh3ODIndex;
  }
  return 0;
}

uint32_t *sensor_getReadIndexBasedOnType(String inParamNameType)
{

  if (inParamNameType.equals(VMP_NO2))
  {
  }
  else if (inParamNameType.equals(VMP_O3))
  {
  }
  else if (inParamNameType.equals(VMP_SO2))
  {
  }
  else if (inParamNameType.equals(VMP_VOC))
  {
    return &alert_readVocIndex;
  }
  else if (inParamNameType.equals(VMP_CO))
  {
  }
  else if (inParamNameType.equals(VMP_NH3))
  {
  }
  else if (inParamNameType.equals(VMP_CO2))
  {
  }
  else if (inParamNameType.equals(VMP_HUM))
  {
    return &alert_readRhIndex;
  }
  else if (inParamNameType.equals(VMP_PM10))
  {
  }
  else if (inParamNameType.equals(VMP_PM25))
  {
  }
  else if (inParamNameType.equals(VMP_TEMP))
  {
    return &alert_readTempIndex;
  }
  else if (inParamNameType.equals(VMP_NH3OD))
  {
    return &alert_readNh3ODIndex;
  }
  return nullptr;
}
