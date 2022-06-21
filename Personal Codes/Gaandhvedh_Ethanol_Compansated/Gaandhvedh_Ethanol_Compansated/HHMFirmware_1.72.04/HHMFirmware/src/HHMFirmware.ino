#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>
#include <xxtea-iot-crypt.h>
#include <map>
#include "VNH3.h"
#include "VDefines.h"
#include "VUtils.h"
#include "AccessPointManager.h"
#include "VRegistration.h"
#include "VInformationManager.h"
#include "VDeviceData.h"
#include "VAlertEngine.h"
#include "VResponseParser.h"
#include "VSensors.h"
#include "VDeviceModelSensorInfo.h"
#include "VCalibrationAPIExecution.h"
#include "VCalibrationSensorCalculation.h"
#include "VMemoryManagement.h"

#include "VCalculation.h"
#include "VCommunication.h"
#include "VDelay.h"
#include "VCmdLineInterface.h"
#include "VHandleGPIO.h"
#include "VImageUpgrade.h"
#include "VCrc.h"

#include <Wire.h>

std::map<String, String> mapDeviceInfo;                                   // DeviceID, DeviceName, OwnerID, PostalID, VicinityType etc.
std::map<String, String> mapConnectionInfo;                               // WiFiSSID, SSIDPassword, URL etc.
std::map<String, CVSingleAlertSetting> mapAlertSettings;                  // Alert setting relevant to this device.
std::map<String, double> mapDeviceValues;                                 // Sensor Reading Values that have been read by this device.
std::map<String, CVDeviceModelSingleSensorInfo> mapDeviceModelSensorInfo; // Model Sensor Info Values that have been read by this device.
std::map<String, long> mapSensorSanityInterval;                           // To avoid glitch in a Sensor (sometimes for Temp/Hum it suddenly goes to Min Range).
std::map<String, uint32_t> mapCalibrationInfo;
std::map<String, double> mapCalibrationSensorValueInfo;

// bool gbIsConnectedToWifi = false;
// bool gbIsDeviceInAPMode = false;
bool bIsDeviceRegistered = false;

// Flag for Firing the getAlertSettingsFromServer if the Alert Settings are found empty in SPIFFS.
// or if the getAlertSettingsFromServer API has never been fired then.If the Alert Settings are retreived/written successfully from/in SPIFFS,
// Then make the FiregetAlertSettingsFromServerAgain flag as false.
bool FiregetAlertSettingsFromServerAgain = true;
// Flag for Firing the getDeviceModelSensorInfoFromServer if the Device Model are found empty in SPIFFS.
// or if the getDeviceModelSensorInfoFromServer API has never been fired then. If the Device Model is retreived/written successfully from/in SPIFFS,
// Then make the FiregetDeviceModelSensorInfoFromServerAgain flag as false.
bool FiregetDeviceModelSensorInfoFromServerAgain = true;

/**************** FOR BACKUP *****************/
// Flag for sending the backed up data from device if any.
bool gbSendBackedUpData = false;
// When First HTTP fail occurs, then only start checking for Backed Up Data.
bool gbCheckForBackedUpData = false;

// Stop reading sensor data when the Webpage ie.192.168.4.1 is opened.
extern bool isWebPageAccessed;
// For the server to know whether it's fresh data or old Backlog data.
bool isBackLogData = false;

unsigned long glngSendDeviceDataTimer = 0;
unsigned long glngSendAlertLogTimer = 0;
unsigned long glngDailyCommonTaskTimer = 0;

DELAY objDataFiltrationDelay;
DELAY objAlertProcess;
DELAY objOtaUpdate;

esp32 esp32State;
DELAY objAPModeDelay;

DELAY objSendBackedUpData;

DELAY objCalibrationProcess;
DELAY objCalibrationWindow;
DELAY objinvokeCalibStepInfoAgain;
// Start the calculation timer as soon as you encounter NH3OD Param.
DELAY objTGSStartCalibDelay;
DELAY objTGSWaitCalibDelay;
DELAY objTGSReadDelay;

// RTC Correction will be done every 24 hrs.
DELAY objRTCcorrectionDelay;

String txStatus = "";
String AlertStatus = "";

String getstrRtcDate;
String getstrRtcTime;
String getstrRtcDateTime;
String getAlertTime;
String lstrRTCResult = "";
String lstrRTCResDeviceData = "";
uint32_t UNIXTimeStamp;

// Since its decided by user and sent from webpage, retreive from map.
uint8_t DataSendTimeInterval = 0;

extern String strDeviceDataStatus;

/******* FOR CALIBRATION *******/
extern bool isDevcinCalibrationMode;
// For Acknowledgement for Calibration.
bool isAckforcalibSent = false;
// For invoking getCurrentCalibStepInfo API again after 8 sec to check if user has pressed any button.
bool invokegetCalibStepInfoAgain = false;
// If two step gas is being measured, do not call any other API for calibration.Keep check timeout in that case.
extern bool BlockCaliAPIfor2StepGas;
// This flag checks whether sensor calculation has started or not.
bool hasSensorCalculationsBegun = false;
// Make the hasTimeOutOccuredduringSensorCal flag true if timeout occurs while a step was going on,after 30 minutes.
bool hasTimeOutOccuredDuringSensorCal = false;
// Make the hasTimeOutOccured flag true if timeout occurs after a step was completed,after 30 minutes.
bool hasTimeOutOccuredAfterSensorCal = false;
// Keep it Global because it's value will be required in every iteration.
String CalibRetVal = "";
String TimeoutRetVal = "";
uint8_t StatforInvokingfun;
String MeasuredParamName = "";
float UserInputStep1Val = 0.0f;
float UserInputStep2Val = 0.0f;
float UserInputStep3Val = 0.0f;
String CalibStepNumber = "";
String Step1StatusInfo = "";
String Step2StatusInfo = "";
String Step3StatusInfo = "";
float PostCalCalibOutput = 0.0f;

void setup()
{

  Serial.begin(115200); // Imp: Match Baud Rate with Monitor Speed present in Ini File

  String lstrResult = "";

  // float rs_REF1=0.0f, rs_REF2=0.0f; uint32_t *ptr_REF1, *ptr_REF2;

  // Init GPIO
  gpio_initGPIO();

  // Initialize SPIFFS so that files can be Read or Saved into it.
  // TODO: If initSPIFFS fails, Turn ON the LED (or give specific RGB color).
  // Wait for 10 seconds and Restart the Device.
  initSPIFFS();

  // Initialize CRC table
  command_Init_CRC32_Table();

  // Get the Device Information if it is present in SPIFFS file
  readDeviceInfoFromFile(mapDeviceInfo);

  // Save the current Firmware Version in a Map.
  mapDeviceInfo[FW_VER] = FW_VER_NUM;
  // Display Device-ID and FW-Ver on priority
  // Serial.begin(115200); // Imp: Match Baud Rate with Monitor Speed present in Ini File
  Serial.print("Device ID: ");
  // Serial.print(DEVICE);
  Serial.print(mapDeviceInfo[V_DEVICE_ID]);
  Serial.print(", Powered by FW Ver: ");
  Serial.println(mapDeviceInfo[FW_VER]);

  // Show the info of current image
  image_showRunningImage();

  // Get the Connection Information if it is present in SPIFFS file
  readConnectionInfoFromFile(mapConnectionInfo);
  // So that in production when a new firmware is flashed to a fresh device then device should automatically connect
  // to the hardcoded ie. available SSID and Password is also provided here.
  if (mapConnectionInfo[DEVC_NW_SSID].length() <= 0)
  {

    mapConnectionInfo[DEVC_NW_SSID] = "Viliso_2.4G";
    mapConnectionInfo[DEVC_NW_SSID_PASSWORD] = "shekhar19";
  }

  // Read Calibration info from Memory
  readCalibrationInfoFromFile(mapCalibrationInfo);

  // mapCalibrationSensorValueInfo[NH3_PPM_REF1] = 1.0;
  // mapCalibrationSensorValueInfo[NH3_PPM_REF2] = 5.0;
  // mapCalibrationSensorValueInfo[VMP_NH3OD_REF1] = 38.3599;
  // mapCalibrationSensorValueInfo[VMP_NH3OD_REF2] = 32.6827;
  // mapCalibrationSensorValueInfo[ETHANOL_PPM_REF1] = 1.0;
  // mapCalibrationSensorValueInfo[ETHANOL_PPM_REF2] = 5.0;
  // mapCalibrationSensorValueInfo[VMP_ETHANOL_REF1] = 13.03;
  // mapCalibrationSensorValueInfo[VMP_ETHANOL_REF2] = 10.80;
  // writeCalibDoubleValuesToFile(mapCalibrationSensorValueInfo);

  // Read Calibration Sensor Values info from Memory
  ReadCalibDoubleValuesFromFile(mapCalibrationSensorValueInfo);

  // Set the max datasets of sensor data that can be stored(240) in SPIFFS, when the wifi is down for the First File.
  mapCalibrationInfo[INDEX_LIMIT_FILE_ONE] = FILE_LIMIT_ONE;
  // For the Second File this limit will be 400 which will be the maximum limit.
  mapCalibrationInfo[INDEX_LIMIT_FILE_TWO] = FILE_LIMIT_TWO;

  // Initialize all Sensors
  sensor_initAllSensors();

  // Do not write in if condition.Else it takes the OLD URL stored in SPIFFS.
  mapConnectionInfo[DEVC_API_URL] = DEVICE_API_URL_VALUE;

#ifdef DEVICE
  // Vivek: Now going to set Device ID & Auth Key during first calibration process
  // if( mapDeviceInfo.find(V_DEVICE_ID) == mapDeviceInfo.end() ) {
  mapDeviceInfo[V_DEVICE_ID] = DEVICE; // Macro, check VDefines.h for it's value
  //}

  // if( mapDeviceInfo.find(V_DEVICE_AUTH) == mapDeviceInfo.end() ) {
  mapDeviceInfo[V_DEVICE_AUTH] = KEY; // Macro, check VDefines.h for it's value
  //}
  // TODO: End: Delete this Block Once CRM is implemented.
#endif

  // Debug print the values present in the above map
  debugPrintConnectionAndDeviceInfoFromGlobalMap();

  // Chcek if all necessary Deviceinfo and all connectioninfo are present.
  if (!ifAllDeviceConnectionConfigPresent())
  {

    if (mapDeviceInfo.find(V_DEVICE_ID) == mapDeviceInfo.end() ||
        mapDeviceInfo.find(V_DEVICE_AUTH) == mapDeviceInfo.end())
    {
      // If Device ID & Auth not present
      esp32State.setCurrentState(ESP32_MISSING_CONFIG);
      Serial.println("HHM-PT: Missing Device ID & Auth Key. Please configure.");
    }
    else if (mapConnectionInfo.find(DEVC_NW_SSID) == mapConnectionInfo.end() ||
             mapConnectionInfo.find(DEVC_NW_SSID_PASSWORD) == mapConnectionInfo.end() ||
             mapDeviceInfo.find(V_DEVICE_NAME) == mapDeviceInfo.end())
    {
      esp32State.setCurrentState(ESP32_WIFI_NOT_CONNECTED);
      Serial.println("HHM-PT: Missing WiFi config. Please configure.");
    }
  }
  else
  {
    // Connect to WiFi as required information is present
    if (connectToWifi(mapConnectionInfo[DEVC_NW_SSID], mapConnectionInfo[DEVC_NW_SSID_PASSWORD]) != STATUS_SUCCESS)
    {
      // Could not connect to WiFi. Need to connect to Access Point Mode
      esp32State.setCurrentState(ESP32_WIFI_NOT_CONNECTED);
    }
    else
    {
      // Connected to WiFI (i.e STA Mode)
      esp32State.setCurrentState(ESP32_WIFI_CONNECTED);

      // Once the Wifi is connected after reboot, send the backed up data immediately if any.
      gbSendBackedUpData = true;
      // Also the interval to send Backed Up Data will be of 1 minute so that there won't be any load on the Server.
      objSendBackedUpData.setDelay(BACK_UP_DATA_INTERVAL);
    }
  }

  if (esp32State.getCurrentState() == ESP32_WIFI_NOT_CONNECTED)
  {

    // Switch device to Access Point (AP) mode if WiFi connection failed.
    lstrResult = switchDeviceToAPServerMode(mapDeviceInfo);

    if (lstrResult != AP_STATUS_SUCCESS)
    {
      esp32State.setCurrentState(ESP32_AP_NOT_CONNECTED);

      Serial.println("Not able to connect to AP Server, nor in WiFi Mode. Please check ESP32.");

      // Keep LED Off to Indicate that the device is neither connected to WiFi nor is in AP Server Mode
      // digitalWrite(COMM_LED, LOW); // TODO: Show appropriate RGB color
      setSpecifiedIndicatorColorForLED(LED_COLOR_RED);
    }
    else
    {
      objAPModeDelay.setDelay(ESP32_AP_MODE_DELAY);
      esp32State.setCurrentState(ESP32_AP_CONNECTED);
      // Set the color of LED to Blue
      setSpecifiedIndicatorColorForLED(LED_COLOR_BLUE);
    }
  }
  else if (esp32State.getCurrentState() == ESP32_WIFI_CONNECTED)
  {
    // If WiFi is connected.

    // This API will be fired first on reboot of the device only if DEVC_SHARE_FW_INFO Flag is true, meaning OTA is completed successfully.Hence, check for the Flag.
    if (mapCalibrationInfo[DEVC_SHARE_FW_INFO])
    {

      SendStatusForOTAUpdate();
    }

    // Register the device in order to save device info (OR Re-register, if
    // some of the info has changed in case the device was in AP mode)2
    lstrResult = registerDevice(mapConnectionInfo, mapDeviceInfo, mapCalibrationInfo);

    // Fetch Alert (user) setting from Server, if WifI connedtion is available and
    // store it into SPIFFS. If WifI connedtion is not available, read from SPIFFS.
    fillAlertSettingsMap();

    // Fetch Model Senosor (company setting & default range) setting from Server, if WifI available
    // and store it into SPIFFS, otherwise read from SPIFFS.
    fillDeviceModelSensorInfoMap();

    // Set the color of LED to Green.
    setSpecifiedIndicatorColorForLED(LED_COLOR_GREEN);

    // Set the time during the initialization itself as the battery might be re-inserted again and the time might have got reset.
    FiregetCurrentUTCDateTime();
  }

  // Since its decided by user and sent from webpage, retreive from map.
  DataSendTimeInterval = mapCalibrationInfo[DATA_FILTRATION_DELAY];
  if (DataSendTimeInterval == 0)
  {

    // Hard code it as 'one' ie, One minute because then if its a freshly flashed device, interval will be zero, and data will be sent to server in microseconds.
    DataSendTimeInterval = 1;
  }
  objDataFiltrationDelay.setDelay(DataSendTimeInterval * 60 * 1000);

  // Defining Alert checking interval, but firing interval
  // of any alarm is dependent on that particular item.
  objAlertProcess.setDelay(ALERT_PROCESSING_DELAY);
  alert_initAlertSystem();

  // Defining freq to check any OTA updation request
  objOtaUpdate.setDelay(OTA_UPDATION_DELAY);

  // RTC will be set again after completing 24 hrs period so that there won't be a lag in time.
  objRTCcorrectionDelay.setDelay(RTC_CORRECTION_DELAY);

} //***************************End of Setup()*************************

void loop()
{

  // String lstrResult = "";

  // See if DeviceId or Auth Key need to be read or not.
  ReadDeviceIDAuthKey();

  // Read the Rtc Time and update the variable/array every two second.
  lstrRTCResult = ReadRTCTime(getstrRtcDateTime, getstrRtcDate, getstrRtcTime);

  if (esp32State.getCurrentState() == ESP32_AP_CONNECTED)
  {
    // Device is AP Server Mode.
    // Listen to client requests
    handleAPModeClient();

    // If all basic config is present and AP time out period is over.
    if (ifAllBasicConfigPresent() && objAPModeDelay.isTimeOut())
    {
      // isWebPageAccessed Flag is made false as the device has crossed 5 minute interval time.
      isWebPageAccessed = false;
      // Retry WiFi connectivity
      if (connectToWifi(mapConnectionInfo[DEVC_NW_SSID],
                        mapConnectionInfo[DEVC_NW_SSID_PASSWORD]) == STATUS_SUCCESS)
      {
        // Set the flag
        esp32State.setCurrentState(ESP32_WIFI_CONNECTED);

        // The most important call is 'coming out of AP Mode on first Reboot'.
        // Because there the SPIFFS might be empty as device never got connected to Wifi so these API's ie.fillAlertSettingsMap, fillDeviceModelSensorInfoMap will be fired in the above condition only if the SPIFFS doesn't have any Info of it.

        // This API will be fired first on reboot of the device only if DEVC_SHARE_FW_INFO Flag is true, meaning OTA is completed successfully.Hence, check for the Flag.
        if (mapCalibrationInfo[DEVC_SHARE_FW_INFO])
        {

          SendStatusForOTAUpdate();
        }

        // Register the Device, may be its first time registration after Reboot.
        // registerDevice(mapConnectionInfo, mapDeviceInfo, mapCalibrationInfo);

        if (FiregetAlertSettingsFromServerAgain)
        {

          // Fetch Alert (user) setting from Server again.
          fillAlertSettingsMap();
        }
        else
        {

          Serial.println("Alert Settings already stored in SPIFFS.\r\n");
        }

        if (FiregetDeviceModelSensorInfoFromServerAgain)
        {

          // Fetch Model Senosor (company setting & default range) setting from Server again
          fillDeviceModelSensorInfoMap();
        }
        else
        {

          Serial.println("Device Model Info already stored in SPIFFS.\r\n");
        }

        // Ideally once the Wifi has come up after the Internet went down, the backed up data should be sent immediately if any.Sometimes the wifi signal is only present
        // And there is no internet as such so unnecessarily the backup data will be sent every minute and the response will be HTTP_REQUEST_FAILED.
        // Again there will be unnecessary load on the Server so send the Backed up data only after the data/Alerts API Response is successfull.

        // Set the color of LED to Green
        setSpecifiedIndicatorColorForLED(LED_COLOR_GREEN);
        Serial.println("ESP32: moved to WiFi mode again");
      }
      else
      {
        // If not successfull, try again after timeout period.
        objAPModeDelay.setDelay(ESP32_AP_MODE_DELAY);
        Serial.printf("ESP32: Tried WiFi, not successfull, ");
        // Move to AP Mode again
        if (switchDeviceToAPServerMode(mapDeviceInfo) ==
            AP_STATUS_SUCCESS)
        {
          Serial.println("moved to AP-Mode again.\r\n");
        }
        else
        {
          Serial.println("failed to move to AP-Mode also.\r\n");
        }
      }
    }
  }
  else if (esp32State.getCurrentState() == ESP32_WIFI_CONNECTED)
  {
    // If Device is connected to Wifi
    // Check if any Http Post Request Failed because of WiFi connectivity.Even on first HTTP Fail, since esp32State.isTimeOut() is true, because
    // The Timer has not been set yet, on first HTTP Fail of data itself the attempt of connectivity will be done.So, it doesn't matter if the data Interval
    // Is increased, the attempt to connect will be of 1 minute only.But it will go on forever if txStatus doesn't become SUCCESS and that depends on data Interval.
    // Hence registerDevice fillDeviceModelSensorInfoMap will be unncessarily fired till txStatus doesn't become SUCCESS, which is a burden on the server.So these APIs won't be fired
    // on HTTP_REQUEST_FAILED as before that they have been fired anyways and data(eg. device model) is already in SPIFFS. The most important call 'coming out of AP Mode on first Reboot'
    // Because there the SPIFFS might be empty as device never got connected to Wifi so these API's will be fired in the above condition only.Exception is fillAlertSettingsMap, as it is a very critical case as its used to calculate alerts.
    // So if the SPIFFS never has any alert settings, then fire the getAlertSettingsFromServer API and fill the SPIFFS otherwise don't.

    if (ifAllBasicConfigPresent() &&
        txStatus.equals(DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED) &&
        esp32State.isTimeOut())
    {
      // i.e Flag is set, but in reality WiFi not connected
      Serial.println("ESP32: WiFi Connectivity error in STA Mode");
      // Try to connect to WiFi again
      if (connectToWifi(mapConnectionInfo[DEVC_NW_SSID],
                        mapConnectionInfo[DEVC_NW_SSID_PASSWORD]) == STATUS_SUCCESS)
      {
        // If success, reset the no of attempt.
        esp32State.resetAttemptNo();
        // Set the flag
        esp32State.setCurrentState(ESP32_WIFI_CONNECTED);

        // This API will be fired first on reboot of the device only if DEVC_SHARE_FW_INFO Flag is true, meaning OTA is completed successfully.Hence, check for the Flag.
        if (mapCalibrationInfo[DEVC_SHARE_FW_INFO])
        {

          SendStatusForOTAUpdate();
        }

        if (FiregetAlertSettingsFromServerAgain)
        {

          // Fetch Alert (user) setting from Server again.
          fillAlertSettingsMap();
        }
        else
        {

          Serial.println("Alert Settings already stored in SPIFFS.\r\n");
        }

        // Ideally once the Wifi has come up after the Internet went down, the backed up data should be sent immediately if any.Sometimes the wifi signal is only present
        // And there is no internet as such so unnecessarily the backup data will be sent every minute and the response will be HTTP_REQUEST_FAILED.
        // Again there will be unnecessary load on the Server so send the Backed up data only after the data/Alerts API Response is successfull.

        // Set the color of LED to Green.
        setSpecifiedIndicatorColorForLED(LED_COLOR_GREEN);
        Serial.println("ESP32: WiFi reconnected");
      }
      else
      {
        // If not succeed, increase no of attempt.
        esp32State.increaseAttemptNo();

        // If max attempt
        if (esp32State.isMaxAttempt())
        {
          esp32State.resetAttemptNo();
          Serial.println("ESP32: Max attempt to reconnect WiFi");

          // Switch to AP mode
          if (switchDeviceToAPServerMode(mapDeviceInfo) ==
              AP_STATUS_SUCCESS)
          {
            // Set the flag
            esp32State.setCurrentState(ESP32_AP_CONNECTED);
            // Set the color of LED to Blue
            setSpecifiedIndicatorColorForLED(LED_COLOR_BLUE);
            // Set the Delay period, during which ESP32 will be in AP Mode only
            objAPModeDelay.setDelay(ESP32_AP_MODE_DELAY);
            Serial.println("ESP32: Switched to AP mode");
          }
          else
          {
            esp32State.setCurrentState(ESP32_AP_NOT_CONNECTED);
            setSpecifiedIndicatorColorForLED(LED_COLOR_RED);
          }
        }
      }
      // Set the Delay period again
      esp32State.setDelay(RETRY_WIFI_CONN_DELAY);
    }
  }
  else
  {
    // Device is neither connected to WiFi nor is in AP Server Mode
    // Serial.println("Neither connected to WiFi nor in AP Server Mode, please check ESP32");
    setSpecifiedIndicatorColorForLED(LED_COLOR_RED);

    // Try AP mode again, if all basic info present.
    if (ifAllBasicConfigPresent())
    {
      if (switchDeviceToAPServerMode(mapDeviceInfo) ==
          AP_STATUS_SUCCESS)
      {
        // Set the flag
        esp32State.setCurrentState(ESP32_AP_CONNECTED);
        // Set the color of LED to Blue
        setSpecifiedIndicatorColorForLED(LED_COLOR_BLUE);
        Serial.println("ESP32: Switched to AP mode from blank state");
      }
    }
  }

  // Check if any key is pressed
  if (gpio_readGPIO() == ESP32_MODE_SW_ENABLED)
  {
    // Move to AP Mode, if its in STA mode
    if (esp32State.getCurrentState() != ESP32_AP_CONNECTED)
    {

      if (switchDeviceToAPServerMode(mapDeviceInfo) ==
          AP_STATUS_SUCCESS)
      {
        // Set the flag
        esp32State.setCurrentState(ESP32_AP_CONNECTED);
        // Set the color of LED to Blue
        setSpecifiedIndicatorColorForLED(LED_COLOR_BLUE);
        objAPModeDelay.setDelay(ESP32_AP_MODE_DELAY);
        Serial.println("ESP32: Button Pressed, Switched to AP mode.");
      }
      else
      {
        esp32State.setCurrentState(ESP32_AP_NOT_CONNECTED);
      }
    }
  }

  // If all config are present, then only process sensor's data
  if (ifAllBasicConfigPresent())
  {

    String lstrResult = "";

    // If it is the interval for Daily 24 hour task, then do those tasks
    // first. (Note: Currently there is only one task to get updated
    // alertSettings, but we can have more tasks in future)
    if ((millis() - glngDailyCommonTaskTimer) > DAILY_COMMON_TASK_INTERVAL)
    {
      // Register the Device, may be its first time registration after Reboot.
      registerDevice(mapConnectionInfo, mapDeviceInfo, mapCalibrationInfo);

      // Fill alert settings map by getting from server.
      // Also save in SPIFFS if successfully retrieved from server.
      // If alert settings not received successfully from server then retrieve
      // the same from SPIFFS (if already saved earlier in SPIFFS).
      fillAlertSettingsMap();

      // Fill Device Model SensorInfo map by getting from server.
      // Also save in SPIFFS if successfully retrieved from server.
      // If Device Model SensorInfo not received successfully from server then retrieve
      // the same from SPIFFS (if already saved earlier in SPIFFS).
      fillDeviceModelSensorInfoMap();

      // Note the current time in the DailyTaskTimer
      glngDailyCommonTaskTimer = millis();
    }

    // If the Webpage ie.192.168.4.1 is Accessed when the device is in AP Mode, then do not read sensor data.
    // Otherwise, the wifi scannning doesn't work if sensor reading is done simulataneously.
    if (!isWebPageAccessed)
    {

      sensor_getSensorReadings(mapDeviceValues);

      // For Calibration Mode, we are blocking executing of SendDeviceData and SendAlerts API even alerts wont be calculated.
      if (!isDevcinCalibrationMode)
      {
        // Calculate Avg and other Statistics @ 1Min interval and send to server
        if (objDataFiltrationDelay.isTimeOut())
        {
          // Since its decided by user and sent from webpage, retreive from map.
          DataSendTimeInterval = mapCalibrationInfo[DATA_FILTRATION_DELAY];
          if (DataSendTimeInterval == 0)
          {

            // Hard code it as 'one' ie, One minute because then if its a freshly flashed device, interval will be zero, and data will be sent to server in microseconds.
            // Also retreive the interval again from Map because user can change data interval any time, so pick the updated data interval.No need to reboot device.
            // Also now data interval is updated as 3 mins whenever the device is in sold/demo state and is also mapped to owner(when set in AP mode and device registration is done.)
            DataSendTimeInterval = 1;
          }
          objDataFiltrationDelay.setDelay(DataSendTimeInterval * 60 * 1000);

          // Calculate avg value and copy to buffer
          sensor_calculateNCollectReadings(mapDeviceValues);

          // Send to Server
          Serial.println("Normal Data sending to Server");

          // Also store the RTC Status in a separate variable so that there is no timing mismatch for RTC status for storing backup data.
          lstrRTCResDeviceData = lstrRTCResult;
          isBackLogData = false;
          txStatus = sendDeviceDataToServer(mapConnectionInfo, mapDeviceInfo, mapDeviceValues, getstrRtcDateTime, lstrRTCResDeviceData, isBackLogData);
          // If we get HTTP failed when there is no wifi, and RTC time is not corrupted, then save the sensor data in SPIFFS.
          if ((txStatus == DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED) && (lstrRTCResDeviceData == RTC_DATE_TIME_VALID))
          {
            // Start checking for Backed Up Data only when the gbCheckForBackedUpData flag is true after flashing new code.
            gbCheckForBackedUpData = true;
            mapCalibrationInfo[CHECK_BACKUP_DATA] = 1;
            Serial.println("The check for BackUp Data Flag has been made true");
            writeCalibrationInfoToFile(mapCalibrationInfo);
            // Now the wifi has gone down, so then do not send backed up data.
            gbSendBackedUpData = false;
            SaveSensorDataInSPIFFS(getstrRtcDate, getstrRtcTime);
          }

          // Since the data interval is changed according to the user preferance, send the backup data immediately after Device Data is sent successfully.
          // In this way, even if data interval is for 30-45 mins, if Device Data is sent successfully, then send backup data immediately too.
          if (!gbSendBackedUpData)
          {

            if (txStatus == DEVICE_API_RESPONSE_SUCCESS_CODE)
            {
              // Once the Wifi has come up after the Internet went down, send the backed up data immediately if any.
              gbSendBackedUpData = true;
            }
          }

          // Print Time in Min
          Serial.println();
          // Read the current RTC time and display it on serial.
          PrintRtcTime(getstrRtcDateTime, UNIXTimeStamp);
          Serial.print("Time(Min): ");
          Serial.println(millis() / DATA_SERIAL_DISPLAY_DELAY);
        }

        // Check all Alarms and if there is any valid case, fire it and also trigger a normal data send to server
        if (objAlertProcess.isTimeOut())
        {
          objAlertProcess.setDelay(ALERT_PROCESSING_DELAY);

          AlertStatus = alert_handleAllActiveAlerts(mapConnectionInfo, mapDeviceInfo, mapAlertSettings, mapDeviceValues, getstrRtcDateTime, getAlertTime, lstrRTCResult);

          if (AlertStatus == STATUS_SUCCESS)
          {
            Serial.println("Sending Data to Server after sending Alert");

            isBackLogData = false;
            txStatus = sendDeviceDataToServer(mapConnectionInfo, mapDeviceInfo, mapDeviceValues, getAlertTime, lstrRTCResult, isBackLogData);

            // Since the data interval is changed according to the user preferance, send the backup data immediately after successful data after alerts too.
            // In this way, even if data interval is for 30-45 mins, if data after alerts is successfull, then send backup data immediately too.
            if (!gbSendBackedUpData)
            {

              if (txStatus == DEVICE_API_RESPONSE_SUCCESS_CODE)
              {
                // Once the Wifi has come up after the Internet went down, send the backed up data immediately if any.
                gbSendBackedUpData = true;
              }
            }
          }
        }

        // Do necessary calibration to CCS811
        sensor_doVOCCalibration();

        // Check for OTA updates at every OTA_UPDATION_DELAY interval
        if (objOtaUpdate.isTimeOut())
        {
          objOtaUpdate.setDelay(OTA_UPDATION_DELAY);

          if (image_isNewFWAvailable())
          {

            Serial.println("New FW is available for updates.");
            // Now ask for clearance to download the new bin.
            if (image_getClearanceToDownloadBin(mapConnectionInfo, mapDeviceInfo) == STATUS_SUCCESS)
            {

              // Got clearance, now download vi-bin file.
              image_upgradeFirmware();
            }
            else
            {
              Serial.println("Didn't receive clearance from Server to download vi-bin file.");
            }
          }
        }
      }
      else
      {

        if (objCalibrationProcess.isTimeOut())
        {

          objCalibrationProcess.setDelay(CALIBRATION_PROCESSING_DELAY);

          // Do not send any other API while calibration of 2 step gas is going ON.
          if (!BlockCaliAPIfor2StepGas)
          {
            // The Device has been set to Calibration Mode.Start executing those APIs of calibration now.
            MoveDevicetoCalibrationMode();
          }
          else
          {
            // TODO: Need to check if this fun again points to NH3OD again and again.Yes it does!
            BeginSensorCalibration(mapCalibrationInfo, mapCalibrationSensorValueInfo, mapConnectionInfo, mapDeviceInfo, MeasuredParamName, UserInputStep1Val, UserInputStep2Val, UserInputStep3Val, CalibStepNumber, Step1StatusInfo, Step2StatusInfo, Step3StatusInfo, PostCalCalibOutput);
          }
          // Start the timer at the start itself ie. acknowledgement part and once ack is sent it is not executed again.So the timer too won't be reset again, so that there won't be immediate timeout in the void loop.
          if (objCalibrationWindow.isTimeOut())
          {

            // Various conditions to Fire Timeout API are written in the function below.
            FireTimeoutforCalibration();
          }
        }
      }
    }
  }

  // Check if any CLI request...don't comment it for now.
  // cli_processCMD(mapDeviceInfo, mapCalibrationInfo, mapCalibrationSensorValueInfo);

  // Start checking for Backed Up Data on First HTTP Fail, which means there is backed Up Data where CHECK_BACKUP_DATA is also made true.
  // Once the Wifi comes up, then send the Backed Up Data at every 3 Second if CHECK_BACKUP_DATA is true.
  if (mapCalibrationInfo[CHECK_BACKUP_DATA])
  {

    gbCheckForBackedUpData = true;
  }
  else
  {

    gbCheckForBackedUpData = false;
  }
  if (gbCheckForBackedUpData && gbSendBackedUpData && objSendBackedUpData.isTimeOut() && !isDevcinCalibrationMode)
  {

    // If Wifi has come up and the CHECK_BACKUP_DATA is true then only send the Back Up Data after every 1 minute.
    RetrieveAndSendSensorDataFromSPIFFS();
    objSendBackedUpData.setDelay(BACK_UP_DATA_INTERVAL);
  }

  // RTC will be set again after completing 24 hrs period so that there won't be a lag in time.
  if (objRTCcorrectionDelay.isTimeOut())
  {

    objRTCcorrectionDelay.setDelay(RTC_CORRECTION_DELAY);
    FiregetCurrentUTCDateTime();
  }

} //***************************End of loop()*************************

bool ifAllBasicConfigPresent()
{
  if (mapDeviceInfo.size() <= 0 || mapConnectionInfo.size() <= 0 ||
      mapConnectionInfo.find(DEVC_NW_SSID) == mapConnectionInfo.end() ||
      mapConnectionInfo.find(DEVC_NW_SSID_PASSWORD) == mapConnectionInfo.end())
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool ifAllDeviceConnectionConfigPresent()
{
  if (mapDeviceInfo.size() <= 0 || mapConnectionInfo.size() <= 0 ||
      mapDeviceInfo.find(V_DEVICE_ID) == mapDeviceInfo.end() ||
      mapDeviceInfo.find(V_DEVICE_AUTH) == mapDeviceInfo.end() ||
      mapConnectionInfo.find(DEVC_NW_SSID) == mapConnectionInfo.end() ||
      mapConnectionInfo.find(DEVC_NW_SSID_PASSWORD) == mapConnectionInfo.end())
  {
    return false;
  }
  else
  {
    return true;
  }
}

String fillAlertSettingsMap()
{
  String lstrResult = "";
  // Now keep the variables separate as HTTP can fail while gettings alerts from server ie. when getAlertSettingsFromServer is fired.
  // So the old data will be retreived from SPIFFS and the lstrResult will show SUCCESS which is wrong. So keep the variable separate 'lstrSPIFFSStatResult'
  // And check this variable if new Alert Settings have been written or not. In case of HTTP Failure, this variable will be blank as lstrResult will get updated then.
  String lstrSPIFFSStatResult = "";

  // Get latest Alert Settings from the Server
  lstrResult = getAlertSettingsFromServer(mapConnectionInfo, mapDeviceInfo, mapAlertSettings);
  if (lstrResult != STATUS_SUCCESS)
  {
    // Get Device Alert Settings failed. (Error already printed in the called function).
    Serial.println("Alert Setting could not be retrieved from the Server. Trying to retrieve the same from SPIFF file.");

    // Read Alert Settings from SPIFF File if it is present there
    lstrResult = readAlertSettingsFromFile(mapAlertSettings);
    if (lstrResult != STATUS_SUCCESS)
    {

      Serial.println("Should never happen. Alert Setting could neither be retrieved from Server nor from SPIFF file. Making the FiregetAlertSettingsFromServerAgain as true.");
      // MapAlertSettings will be empty or will be partially filled. (If partially filled, whatever alert setting
      // is available in it, will be used in the 'Loop')
      FiregetAlertSettingsFromServerAgain = true;
    }
    else if (lstrResult == STATUS_SUCCESS)
    {

      Serial.println("Successfully retreived the Alert Settings from SPIFF file. Making the FiregetAlertSettingsFromServerAgain as false.");
      FiregetAlertSettingsFromServerAgain = false;
    }
  }
  else
  {
    // Get Device Alert Settings Successful.
    Serial.println("Alert Setting successfully retrieved from the Server. Save the same to SPIFF file.");

    // Write the Alert Settings retrieved from Server to SPIFF File.
    lstrSPIFFSStatResult = writeAlertSettingsToFile(mapAlertSettings);
    if (lstrSPIFFSStatResult != STATUS_SUCCESS)
    {

      Serial.println("Failed to Write the Alert Settings retrieved from Server to SPIFF file. Making the FiregetAlertSettingsFromServerAgain as true.");
      FiregetAlertSettingsFromServerAgain = true;
      // No other actions can be taken.
    }
    else if (lstrSPIFFSStatResult == STATUS_SUCCESS)
    {

      Serial.println("Successfully written the Alert Settings retrieved from Server to SPIFF file. Making the FiregetAlertSettingsFromServerAgain as false.");
      FiregetAlertSettingsFromServerAgain = false;
    }

  } // If 'AlertSetting' not received from Server

  debugPrintAlertSettingsFromGlobalMap(); // TODO: Comment after testing

  // Return the SPIFFS Status whether the New Alert Settings have been written or not.
  return lstrSPIFFSStatResult;
}

void fillDeviceModelSensorInfoMap()
{
  String lstrResult = "";

  // Get latest Model Sensor Information for the Device from the Server
  lstrResult = getDeviceModelSensorInfoFromServer(mapConnectionInfo, mapDeviceInfo, mapDeviceModelSensorInfo);
  if (lstrResult != STATUS_SUCCESS)
  {

    // Get Device Model Sensor Information failed. (Error already printed in the called function).
    Serial.println("Device Model Information could not be retrieved from the Server. Trying to retrieve the same from SPIFF file.");

    // Read Device Model Sensor Information from SPIFF File if it is present there
    lstrResult = readDeviceModelSensorInfoFromFile(mapDeviceModelSensorInfo);
    if (lstrResult != STATUS_SUCCESS)
    {

      Serial.println("Should never happen. Model Information could neither be retrieved from Server nor from SPIFF file. Making the FiregetDeviceModelSensorInfoFromServerAgain as true.");
      // mapDeviceModelSensorInfo will be empty or will be partially filled. (If partially filled, whatever Model Sensor Infomation
      // is available in it, will be used in the 'Loop')
      FiregetDeviceModelSensorInfoFromServerAgain = true;
    }
    else if (lstrResult == STATUS_SUCCESS)
    {

      Serial.println("Model Information retrieved from SPIFF file. Making the FiregetDeviceModelSensorInfoFromServerAgain as false.");
      FiregetDeviceModelSensorInfoFromServerAgain = false;
    }
  }
  else
  {

    // Get Device Model Sensor Infomation Successful.
    Serial.println("Device Model Infomation successfully retrieved from the Server. Save the same to SPIFF file.");

    // Write the Device Model Sensor Information retrieved from Server to SPIFF File.
    lstrResult = writeDeviceModelSensorInfoToFile(mapDeviceModelSensorInfo);
    if (lstrResult != STATUS_SUCCESS)
    {

      Serial.println("Failed to Write the Device Model Infomation retrieved from Server to SPIFF file. Making the FiregetDeviceModelSensorInfoFromServerAgain as true.");
      FiregetDeviceModelSensorInfoFromServerAgain = true;
      // No other actions can be taken.
    }
    else if (lstrResult == STATUS_SUCCESS)
    {

      Serial.println("Successfully Written the Device Model Infomation retrieved from Server to SPIFF file. Making the FiregetDeviceModelSensorInfoFromServerAgain as false.");
      FiregetDeviceModelSensorInfoFromServerAgain = false;
    }

  } // If 'Model Sensor Information' not received from Server

  debugPrintDeviceModelSensorInfoFromGlobalMap(); // TODO: Comment after testing
}

void debugPrintConnectionAndDeviceInfoFromGlobalMap()
{

  std::map<String, String>::iterator itKeyValue;
  std::map<String, uint32_t>::iterator row;
  std::map<String, double>::iterator itrow;

  // To Hide Device Auth.
  // Serial.println("Printing Device Info from global map: START");
  // for ( itKeyValue = mapDeviceInfo.begin(); itKeyValue != mapDeviceInfo.end(); itKeyValue++ ) {
  //     Serial.println(itKeyValue->first + ":" + itKeyValue->second);
  // }
  // Serial.println("Printing Device Info from global map: END");

  Serial.println("Printing Connection Info from global map: START");
  for (itKeyValue = mapConnectionInfo.begin(); itKeyValue != mapConnectionInfo.end(); itKeyValue++)
  {
    Serial.println(itKeyValue->first + ":" + itKeyValue->second);
  }
  Serial.println("Printing Connection Info from global map: END");

  Serial.println("Printing Calibration Info from global map: START");
  for (row = mapCalibrationInfo.begin(); row != mapCalibrationInfo.end(); row++)
  {
    Serial.print(row->first + ":");
    Serial.printf("%x\r\n", row->second);
  }
  Serial.println("Printing Calibration Info from global map: END");

  Serial.println("Printing Sensor Calibration Sensor Info from global map: START");
  for (itrow = mapCalibrationSensorValueInfo.begin(); itrow != mapCalibrationSensorValueInfo.end(); itrow++)
  {
    Serial.print(itrow->first + ":");
    Serial.printf("%0.4f\r\n", itrow->second);
  }
  Serial.println("Printing Sensor Calibration Sensor Info from global map: END");
}

void debugPrintAlertSettingsFromGlobalMap()
{

  std::map<String, CVSingleAlertSetting>::iterator itAlertSettings;

  String lstrMeasuredParam;
  String lstrSingleAlertSettingJsonString;

  Serial.println("Printing Alert Settings after reading from global map: START");

  for (itAlertSettings = mapAlertSettings.begin(); itAlertSettings != mapAlertSettings.end(); itAlertSettings++)
  {

    lstrMeasuredParam = itAlertSettings->first; // Key is Measured Param
    lstrSingleAlertSettingJsonString = itAlertSettings->second.toJsonString();

    Serial.println(lstrMeasuredParam + ":" + lstrSingleAlertSettingJsonString);
  }

  Serial.println("Printing Alert Settings after reading from global map: END");
}

void debugPrintAlertValuesFromGlobalMap()
{

  std::map<String, CVSingleAlertSetting>::iterator itAlertSettings;

  String lstrMeasuredParam;
  String isAlarmSet, lowCutoff, highCutoff, isLowNull, isHighNull;

  Serial.println("Printing Alert Values after reading from global map: START");

  for (itAlertSettings = mapAlertSettings.begin(); itAlertSettings != mapAlertSettings.end(); itAlertSettings++)
  {

    lstrMeasuredParam = itAlertSettings->first; // Key is Measured Param

    isAlarmSet = String(itAlertSettings->second.canNotify());

    lowCutoff = String(itAlertSettings->second.getLow());
    highCutoff = String(itAlertSettings->second.getHigh());

    isLowNull = String(itAlertSettings->second.isLowNull());
    isHighNull = String(itAlertSettings->second.isHighNull());

    Serial.println(lstrMeasuredParam + ":" + isAlarmSet + ", " +
                   lowCutoff + ":" + isLowNull + ", " +
                   highCutoff + ":" + isHighNull);
  }

  Serial.println("Printing Alert Values after reading from global map: END");
}

void debugPrintDeviceModelSensorInfoFromGlobalMap()
{

  std::map<String, CVDeviceModelSingleSensorInfo>::iterator itDeviceModelSensorInfo;

  String lstrMeasuredParam;
  String lstrDeviceModelSensorInfoJsonString;

  Serial.println("Printing Device Model Sensor Info after reading from global map: START");

  for (itDeviceModelSensorInfo = mapDeviceModelSensorInfo.begin(); itDeviceModelSensorInfo != mapDeviceModelSensorInfo.end(); itDeviceModelSensorInfo++)
  {

    lstrMeasuredParam = itDeviceModelSensorInfo->first; // Key is Measured Param
    lstrDeviceModelSensorInfoJsonString = itDeviceModelSensorInfo->second.toJsonString();

    Serial.println(lstrMeasuredParam + ":" + lstrDeviceModelSensorInfoJsonString);
  }

  Serial.println("Printing Device Model Sensor Info after reading from global map: END");
}

void MoveDevicetoCalibrationMode()
{

  // STEP '1':
  // By default the Acknowledgement sent will be false.
  if (!isAckforcalibSent)
  {

    // Start the timer at the start itself, so that there won't be timeout in the void loop.
    objCalibrationWindow.setDelay(CALIBRATION_WINDOW);
    Serial.println("Calibration started.Opening calibration window of 1 hour");

    // Since it's in calibration mode, keep the LED Colour as raspberry.
    setSpecifiedIndicatorColorForLED(LED_COLOR_RASPBERRY);

    CalibRetVal = sendAcknowledgementForCalibration(mapConnectionInfo, mapDeviceInfo);
    if (CalibRetVal != STATUS_SUCCESS)
    {
      // If 'SUCCESS' is not received, print the error message.
      Serial.println("Could not receive 'SUCCESS' for Acknowledgement for Calibration");
      Serial.println(CalibRetVal);
    }
    else
    {
      // If 'SUCCESS' is received, execute the next API:getCurrentCalibStepInfo
      Serial.println("Received 'SUCCESS' for Acknowledgement for Calibration.Executing the next API, getCurrentCalibStepInfo.isAckforcalibSent Flag is made true,Ack won't be sent again.");
      // Acknowledgement has been sent successfully, so make the flag true now.
      isAckforcalibSent = true;
    }
  }

  // STEP '2':
  // If Ack sent successfully is true and call getCalibrationStepInfo for the first time.
  if (isAckforcalibSent && !invokegetCalibStepInfoAgain)
  {

    CalibRetVal = getCalibrationStepInfo(mapConnectionInfo, mapDeviceInfo, StatforInvokingfun, MeasuredParamName, UserInputStep1Val, UserInputStep2Val, UserInputStep3Val, CalibStepNumber, Step1StatusInfo, Step2StatusInfo, Step3StatusInfo, PostCalCalibOutput);
    if ((CalibRetVal == STATUS_SUCCESS) && !StatforInvokingfun)
    { //(&& Params are present)
      // If we get 'SUCCESS' then only go further for Sensor Calibration.Else, if we get blank param or user didn't invoke then keep a window of 1 hr
      // and keep pinging every 8 sec.Suppose we get Param Name and Value, then pass those to the function below for sensor calculations.
      if ((MeasuredParamName == VMP_NH3OD) || (MeasuredParamName == VMP_ETHANOL))
      {
        // These timers should not get reset.Anyways since we are blocking these APIs after encountering 2 Step gas, they won't get reset.
        StartTimersforTwoStepCaliGases();
      }

      Serial.println("Received the Measured Param, StepInfo and User Invoked Value.Starting sensor calculations.Making the hasSensorCalculationsBegun flag true.");
      hasSensorCalculationsBegun = true;
      BeginSensorCalibration(mapCalibrationInfo, mapCalibrationSensorValueInfo, mapConnectionInfo, mapDeviceInfo, MeasuredParamName, UserInputStep1Val, UserInputStep2Val, UserInputStep3Val, CalibStepNumber, Step1StatusInfo, Step2StatusInfo, Step3StatusInfo, PostCalCalibOutput);
    }
    else if (((CalibRetVal == STATUS_SUCCESS) && StatforInvokingfun) || ((CalibRetVal != STATUS_SUCCESS) && StatforInvokingfun))
    { //(&& Params are not present)

      // If the Params are empty, then start timer for 8 sec.
      objinvokeCalibStepInfoAgain.setDelay(INVOKE_CURRENT_CALIB_STEP);
      Serial.println("User has not entered anything.Firing getCurrentCalibStepInfo API again after 8 sec");
      // Make invokegetCalibStepInfoAgain Flag true, so that function will be executed after every 8 sec.
      invokegetCalibStepInfoAgain = true;
    }
    else if (CalibRetVal != STATUS_SUCCESS)
    {

      // If 'SUCCESS' is not received, print the error message.
      Serial.println("Could not receive 'SUCCESS' for getCalibrationStepInfo");
      Serial.println(CalibRetVal);
    }
  }

  // STEP '3':
  // Now if invokegetCalibStepInfoAgain is true execute getCalibrationStepInfo again and take further actions.
  if (invokegetCalibStepInfoAgain)
  {

    if (objinvokeCalibStepInfoAgain.isTimeOut())
    {

      // Set delay of 8 seconds again after Timeout.
      objinvokeCalibStepInfoAgain.setDelay(INVOKE_CURRENT_CALIB_STEP);

      Serial.println("8 Seconds are completed.Firing getCurrentCalibStepInfo API again.");

      CalibRetVal = getCalibrationStepInfo(mapConnectionInfo, mapDeviceInfo, StatforInvokingfun, MeasuredParamName, UserInputStep1Val, UserInputStep2Val, UserInputStep3Val, CalibStepNumber, Step1StatusInfo, Step2StatusInfo, Step3StatusInfo, PostCalCalibOutput);
      // If we get 'SUCCESS' then only go further for Sensor Calibration.Else, if we get blank param or user didn't invoke then keep a window of 1 hr
      // and keep pinging every 8 sec.Suppose we get Param Name and Value, then pass those to the function below for sensor calculations.
      if ((CalibRetVal == STATUS_SUCCESS) && !StatforInvokingfun)
      { //(&& Params are present)
        // If we get 'SUCCESS' then only go further for Sensor Calibration.Suppose we get Param Name and Value, then pass those to the function below for sensor calculations.
        if ((MeasuredParamName == VMP_NH3OD) || (MeasuredParamName == VMP_ETHANOL))
        {
          // These timers should not get reset.Anyways since we are blocking these APIs after encountering 2 Step gas, they won't get reset.
          StartTimersforTwoStepCaliGases();
        }

        Serial.println("Received the Measured Param, StepInfo and User Invoked Value.Starting sensor calculations.Making the hasSensorCalculationsBegun flag true.");
        hasSensorCalculationsBegun = true;
        BeginSensorCalibration(mapCalibrationInfo, mapCalibrationSensorValueInfo, mapConnectionInfo, mapDeviceInfo, MeasuredParamName, UserInputStep1Val, UserInputStep2Val, UserInputStep3Val, CalibStepNumber, Step1StatusInfo, Step2StatusInfo, Step3StatusInfo, PostCalCalibOutput);
      }
      else if (CalibRetVal != STATUS_SUCCESS)
      {

        // If 'SUCCESS' is not received, print the error message.
        Serial.println("Could not receive 'SUCCESS' for getCalibrationStepInfo");
        Serial.println(CalibRetVal);
      }
      else
      {

        // Suppose we do not get Param Name and User invoked Value, then fire the getCurrentCalibStepInfo API again after 8 seconds.
        Serial.println("User has still not invoked anything after 8 seconds.Firing getCurrentCalibStepInfo API again after 8 seconds");
      }
    }
    else
    {

      Serial.println("Sorry...8 seconds has not been completed yet");
    }
  }
}

void StartTimersforTwoStepCaliGases()
{

  // Start the timers for NH3OD/ETHNL. After 3 minutes, the 4th minute average data would be calculated where 60 samples of 1sec each would be averaged out.
  objTGSStartCalibDelay.setDelay(TGS_CALIB_DELAY);
  objTGSWaitCalibDelay.setDelay(TGS_WAIT_CALIB_DELAY);
  objTGSReadDelay.setDelay(TGS_READ_DELAY);
  Serial.println("The timers for TGS MOS Sensors have begun.");
}

void FireTimeoutforCalibration()
{

  // Now if the user has invoked any input and sensor calculations are going now then directly fire error JSON Body from that step itself.
  if (hasSensorCalculationsBegun && isDevcinCalibrationMode)
  {

    // Make the hasTimeOutOccuredduringSensorCal flag true indicating 1 hour is done now and also the sensor calculations are going on.
    hasTimeOutOccuredDuringSensorCal = true;
    // Also send the status of the TimeOut Flag through the function itself.
    Serial.println("hasSensorCalculationsBegun flag is still true meaning timeout has occured midway of sensor calculation.Firing Timeout API.");
    BeginSensorCalibration(mapCalibrationInfo, mapCalibrationSensorValueInfo, mapConnectionInfo, mapDeviceInfo, MeasuredParamName, UserInputStep1Val, UserInputStep2Val, UserInputStep3Val, CalibStepNumber, Step1StatusInfo, Step2StatusInfo, Step3StatusInfo, PostCalCalibOutput);
  }
  else if ((!hasSensorCalculationsBegun) && isDevcinCalibrationMode)
  {

    // Make the hasTimeOutOccuredduringSensorCal flag true indicating 1 hour is done now and also the sensor calculation hasn't started yet.
    hasTimeOutOccuredAfterSensorCal = true;
    // Also send the status of the TimeOut Flag through the function itself.
    Serial.println("hasSensorCalculationsBegun flag is false meaning timeout has occured after sensor calculation.Firing Timeout API.");
    BeginSensorCalibration(mapCalibrationInfo, mapCalibrationSensorValueInfo, mapConnectionInfo, mapDeviceInfo, MeasuredParamName, UserInputStep1Val, UserInputStep2Val, UserInputStep3Val, CalibStepNumber, Step1StatusInfo, Step2StatusInfo, Step3StatusInfo, PostCalCalibOutput);
  }
  else
  {

    Serial.println("Timeout conditions are not met.The device might be out of calibration mode");
  }
}
