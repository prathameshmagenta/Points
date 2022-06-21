#ifndef __V_DEFINES_H__
#define __V_DEFINES_H__

#define FW_VER  "FirmwareVersion"
#define FW_VER_NUM "1.72.04"

// Now Device will generate Device Id using MAC, don't enable it
// #define DEVICE  "DA0020"


// Now Device will ask KEY during Calibration and user will provide, don't enable it.
// #define KEY     "$2a$10$HGTzdnqbuJApVI2kchTDvupA0.hsUUp9jDYXo.wP9I8ZdmEOTSNWC"        // DA0020




#define DEVICE_API_URL_VALUE "https://device.smarthhm.com/" 

#define BROWSER_SERVER_URL "https://www.smarthhm.com/"  // Client browser url will be different from Device API Url






// #define DEVICE_API_URL_VALUE "https://device.vilisohhm.com/" 

// #define BROWSER_SERVER_URL "https://www.vilisohhm.com/"  // Client browser url will be different from Device API Url


// #define DEVICE_DATA_SEND_INTERVAL    (5*60*1000)     // 5 Minutes
// #define ALERT_LOG_SEND_INTERVAL      (10*1000)       // 10 Seconds
// #define DAILY_COMMON_TASK_INTERVAL   (24*60*60*1000) // 24 Hours

#define DEVICE_DATA_SEND_INTERVAL    (1*60*1000)        // 1 Min interval (in MS)
#define ALERT_LOG_SEND_INTERVAL      (1000)             // 1 Seconds
#define DAILY_COMMON_TASK_INTERVAL   (24*60*60*1000)    // 24 Hours
#define SENSOR_SANITY_INTERVAL       (2000)             // 2 Seconds

#define DEVICE_DEFAULT_NAME_IN_SERVER_MODE      "HHM_DEVICE_TO_SETUP"
#define DEVICE_DEFAULT_PASSWORD_IN_SERVER_MODE  "00000000"
#define DEVICE_NAME_PREFIX_IN_SERVER_MODE       "HHM Devc"

#define COMM_LED 4

// RGB Pins...for new PCB Board.
// Blue and Green pins have been interchanged in the schematic..these are the right ones.
// #define RED_LIGHT_PIN       16
// #define GREEN_LIGHT_PIN     18
// #define BLUE_LIGHT_PIN      17

// New Pin Outs according to February Schematics.Follow this Pinouts only for new Gandhvedh PCB Boards.
#define RED_LIGHT_PIN       17
#define GREEN_LIGHT_PIN     16
#define BLUE_LIGHT_PIN      18


// Colors
#define LED_COLOR_RED       1
#define LED_COLOR_GREEN     2
#define LED_COLOR_BLUE      3
#define LED_COLOR_YELLOW    4
#define LED_COLOR_RASPBERRY 5
#define LED_COLOR_WHITE     6
#define LED_COLOR_BLACK     7

#define FILE_CONTENT_ENC_KEY "ENV1"

#define FORMAT_SPIFFS_IF_FAILED true

// Measured Params Hash Defines (VMP - Viliso Measured Param)
#define VMP_CO          "CO"
#define VMP_O3          "O3"
#define VMP_CH4         "CH4"
#define VMP_CO2         "CO2"
#define VMP_HUM         "HUM"
#define VMP_NO2         "NO2"
#define VMP_VOC         "VOC"
#define VMP_PM1         "PM1"
#define VMP_PM10        "PM10"
#define VMP_PM25        "PM25"
#define VMP_TEMP        "TEMP"
#define VMP_SMOKE       "SMOKE"
#define VMP_FIRE        "FIRE"
#define VMP_LAT         "LAT"
#define VMP_LON         "LON"
#define VMP_PM1         "PM1"
#define VMP_NH3         "NH3"
#define VMP_LPG         "LPG"
#define VMP_RTC         "RTC"

#define VMP_NH3OD       "NH3OD"
#define VMP_NH3OD_REF1  "NH3OD_REF1"
#define VMP_NH3OD_REF2  "NH3OD_REF2"
#define VMP_NH3OD_RS    "NH3OD_RS"

#define VMP_ETHANOL       "ETHNL"
#define VMP_ETHANOL_REF1  "ETHANOL_REF1"
#define VMP_ETHANOL_REF2  "ETHANOL_REF2"
#define VMP_ETHANOL_RS    "ETHANOL_RS"

#define VMP_NH4         "NH4"
#define VMP_H2S         "H2S"
#define VMP_ALCO        "ALCO"
#define VMP_SO2         "SO2"
#define VOC_COUNT     "VOC_COUNT"

#define VMP_TIME        "TIME"
#define VMP_DATE        "DATE"
#define VMP_TIMESTAMP   "TIMESTAMP"

#define NEWLINE_CHAR    '\n'

#define SPIFF_ALERT_SETTINGS_FILE "/AlertSettings.inf"
#define SPIFF_DEVICE_INFO_FILE "/DeviceInfo.inf"
#define SPIFF_DEVICE_CALIBRATION_FILE "/CalibrationInfo.inf"
#define SPIFF_DEVICE_CONNECTION_INFO_FILE "/ConnectionInfo.inf"
#define SPIFF_DEVICE_MODEL_SENSOR_INFO_FILE "/DeviceModelSensorInfo.inf"
#define SPIFF_DEVICE_CALIB_VALUE_FILE "/CalibValuesInfo.inf"

#define SPIFF_DEVICE_FIRST_BACKUP_SENSORDATA_FILE  "/FirstBackupSensorDataInfo.inf"
#define SPIFF_DEVICE_SECOND_BACKUP_SENSORDATA_FILE "/SecondBackupSensorDataInfo.inf"
#define SPIFF_DEVICE_TEMPORARY_BACKUPDATA_FILE     "/TemporaryBackupDataInfo.inf"

#define REQUIRED_KEYS_MISSING_IN_MAP "RequiredKeysMissingInMap"

#define V_DEVICE_ID "DeviceID"
#define V_DEVICE_AUTH "DeviceAuth"
#define V_DEVICE_NAME "DeviceName"
#define V_USER_EMAIL_ID "UserEmailID"
#define V_DEVICE_ROOMTYPE "RoomType"
#define V_DEVICE_POSTALCODE "PostalCode"

#define DEVC_NW_SSID "DevcNwSsid"
#define DEVC_NW_SSID_PASSWORD "DevcNwSsidPassword"
#define DEVC_API_URL "DevcApiUrl"
#define DEVC_SHARE_FW_INFO "ShareFwInfo"

#define STR_TRUE     "True"
#define STR_FALSE    "False"


#define FILE_LIMIT_ONE     240
#define FILE_LIMIT_TWO     400

#define CHECK_BACKUP_DATA "CheckForBackedUpData"
#define INDEXCOUNTAPPENDED   "IndexValueAppended"
#define INDEX_COUNT_OF_SENT_BACKUP_DATA "IndexCountOfSentBackupData"
#define INDEX_LIMIT_FILE_ONE   "IndexLimitForFirstFile"
#define INDEX_LIMIT_FILE_TWO   "IndexLimitForSecondFile"
#define SQL_ERROR      "SQL_ERROR"

// Alert Settings Json Key
#define ALERT_SETTING_LOW "Low"
#define ALERT_SETTING_HIGH "High"
#define ALERT_SETTING_NOTIFY "Notify"

// Model info Json key
#define MODEL_SENSOR_INFO_STEP "Step"
#define MODEL_SENSOR_INFO_DEF_MAX "DefMax"
#define MODEL_SENSOR_INFO_DEF_MIN "DefMin"
#define MODEL_SENSOR_INFO_RANGE_MAX "RangeMax"
#define MODEL_SENSOR_INFO_RANGE_MIN "RangeMin"
#define MODEL_SENSOR_INFO_ALERTBASED "AlertBased"

#define ATLEAST_ONE_ALERT_LOG_SENT "ONE_OR_MORE_ALERT_LOGS_SENT_FOR_CURRENT_DEVC_DATA"

#define DEVICE_API_RESPONSE_CODE "code"
#define DEVICE_API_RESPONSE_SUCCESS_CODE "SUCCESS"
#define DEVICE_API_RESPONSE_SUCCESS_AFTER_FW_UPGRADE "REG_DONE_OTA_UPD_STATUS_ACCEPTED"
#define DEVICE_API_RESPONSE_FLAG_AFTER_FW_UPGRADE    "IsOtaUpdStatusAccepted"
#define DEVICE_API_RESPONSE_NEW_DEVC_UPDATES_AVAILABLE   "DEVICE_UPDATES_AVAILABLE"
#define DEVICE_API_RESPONSE_FAILURE_MESSAGE "failuremessage"
#define DEVICE_API_RESPONSE_HTTP_NOT_OK "HTTP_NOT_OK"
#define DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED "HTTP_REQUEST_FAILED"
#define DEVICE_API_RESPONSE_UNKNOWN_STATUS "API_RESPONSE_UNKNOWN_STATUS"
#define ERR_DESERIALIZE_JSON_FAILED "DESERIALIZE_JSON_FAILED"

#define RES_KEY_OTA_INFO        "OTAInfo"
#define RES_KEY_OTA_FW_ID       "FrmwrID"
#define RES_KEY_OTA_UPD_KEY     "UpdateKey"

#define RES_KEY_DEVICE_INFO      "DeviceInfo"
#define RES_KEY_DEVICE_NAME      "DeviceName"
#define RES_KEY_DEVICE_PINCODE   "DevicePinCode"
#define RES_KEY_DEVICE_VICINITY_TYPE        "DeviceVicinityType"
#define RES_KEY_DEVICE_DATA_SENT_INTERVAL   "DataSendIntervalTime"
#define RES_KEY_DEVICE_COUNTRY_CODE         "CountryCode"

#define RES_KEY_SETTINGS_INFO      "SettingsInfo"

#define DATA_FILTRATION_DELAY      "DataSendingInterval"

#define COUNTRY_CODE      "CountryCode"

#define RES_KEY_RTC_INFO        "RTCInfo"
#define RES_KEY_RTC_ERROR       "isRTCErr"
#define RES_KEY_DEVC_DT_TM      "DevcSentDtTm"

#define RES_KEY_CODE            "code"
#define RES_VAL_CODE_SUCCESS    "SUCCESS"
#define RES_VAL_CODE_FAILED     "DEVICE_AUTH_FAILED"
#define RES_VAL_CODE_INVALID    "INVALID_DEVICEID"   
#define RES_VAL_CODE_SETTING    "NEW_ALERT_SETTINGS"
#define RES_VAL_CODE_FWID_OR_UPKEY  "INVALID_FRMWRID_OR_UPDTKEY"   
#define RES_VAL_CODE_SERVER_ISSUE   "SERVER_EXPERIENCING_ISSUES"

#define RES_KEY_DEV_ID    "DeviceID"
#define RES_KEY_MSG       "message"


#define RES__VALUE_DEV_ID    
#define RES_VALUE_MSG       

#define DEVICE_API_RESPONSE_FAILURE_MESSAGE "failuremessage"
#define DEVICE_API_RESPONSE_HTTP_NOT_OK "HTTP_NOT_OK"

#define NO_ALERT        "NO_ALERT"

#define STATUS_SUCCESS  "SUCCESS"
#define STATUS_FAILED   "FAILED"
#define HDC_FAILED      "HDC_FAILED"
#define FILE_OPEN_IN_WRITE_MODE_FAILED "CANNOT_OPEN_FILE_IN_WRITE_MODE"
#define FILE_OPEN_IN_READ_MODE_FAILED "CANNOT_OPEN_FILE_IN_READ_MODE"
#define FAILED_TO_WRITE "FAILED_TO_WRITE_IN_FILE"
#define FILE_DOES_NOT_EXIT "FILE_PATH_DOES_NOT_EXIST"


#define ALERT_SETTINGS_MAP_EMPTY "ALERT_SETTINGS_MAP_IS_EMPTY"
#define CONNECTION_INFO_IS_EMPTY "CONNECTION_INFO_MAP_IS_EMPTY"
#define INFORMATION_MAP_EMPTY "INFORMATION_MAP_IS_EMPTY"
#define DEVICE_MODEL_SENSOR_INFO_MAP_EMPTY "DEVICE_MODEL_SENSOR_INFO_MAP_IS_EMPTY"
#define DEVICE_DATA_INFO_MAP_EMPTY "DEVICE_DATA_INFO_MAP_IS_EMPTY"
#define CALIB_VALUE_MAP_EMPTY  "CALIB_VALUE_MAP_EMPTY"

#define FAILED_TO_CONNECT "FAILED_TO_CONNECT"
#define INFORMATION_MAP_EMPTY "INFORMATION_MAP_IS_EMPTY"

#define AP_STATUS_SUCCESS       "SUCCESS"
#define FAILED_MISSING_INFO     "FAILED_MISSING_INFO"
#define FAILED_TO_SET_AP        "FAILED_SET_MODE_AP"
#define FAILED_TO_SET_SOFTAP    "FAILED_SET_SOFTAP"
#define FAILED_TO_INIT_AP_SRVR  "FAILED_INIT_AP_SRVR"

#define FAILED_TO_SET_STATION_MODE "FAILED_SET_MODE_STA"

#define RTC_DATE_TIME_VALID   "RTC_DATE_TIME_VALID"
#define RTC_DATE_TIME_INVALID "RTC_DATE_TIME_INVALID"

#define STATUS_OK      "OK"
#define STATUS_NOT_OK  "NOT_OK"

#define DEVC_MARKED_FOR_CALIBRATION    "DEVC_MARKED_FOR_CALIBRATION"
#define DVC_PARAM_NOT_FOUND_FOR_CALIB  "PARAM_NOT_FOUND_FOR_CALIB"
#define DEVC_GET_OUT_OF_CALIB_MODE     "GET_OUT_OF_CALIBRATION_MODE"
#define DEVICE_API_MEASURED_PARAM   "MeasuredParam"
#define DEVICE_API_CALIB_VALUES     "CalibValues"
#define DEVICE_API_CALIB_STATUS     "DevcCalibStatus"
#define DEVICE_API_CALIB_VAL_STEP1INP    "Step1Inp"
#define DEVICE_API_CALIB_VAL_STEP2INP    "Step2Inp"
#define DEVICE_API_CALIB_VAL_STEP3INP    "Step3Inp"
#define DEVICE_API_CALIB_VAL_RTCINP      "RTCInput"
#define DEVICE_API_CALIB_STATUS_STEP1     "Step1"
#define DEVICE_API_CALIB_STATUS_STEP2     "Step2"
#define DEVICE_API_CALIB_STATUS_STEP3     "Step3"
#define DEVICE_API_SERVER_UTC_DT          "SrvrCurrentUTCDtTm"
#define DEVICE_API_DEVICE_UTC_DT          "DevcCurrentUTCDtTm"
#define DEVICE_API_DEVICE_CURRUTC_TM      "CurrentUNIXTimestamp"
#define DEVICE_API_CALIB_POSTCALOUTPUT    "PostCalOutput"

#define CALIB_STATUS_OK     "OK"
#define CALIB_STATUS_FAILED  "CALIBRATION_FAILED"
#define CALIBVAL_NOT_WITHIN_TOLERANCE  "VAL_NOT_WITHIN_TOLERANCE"
#define CALIB_SENSOR_NOT_AVAILABLE     "SENSOR_NOT_AVAILABLE"
#define CALIB_SENSOR_NEGATIVE_VOLTAGE_VAL  "NEGATIVE_VOLTAGE_VALUE"
#define CALIB_STATUS_SPIFFS_ERROR          "SPIFFS_ERROR"
#define CALIB_STATUS_ERROR_SETTING_RTC     "ERROR_SETTING_RTC"
#define CALIB_TIMEOUT_WAITING_STEP_INPUT  "TIMEOUT_WAITING_STEP_INPUT"


#endif // __V_DEFINES_H__
