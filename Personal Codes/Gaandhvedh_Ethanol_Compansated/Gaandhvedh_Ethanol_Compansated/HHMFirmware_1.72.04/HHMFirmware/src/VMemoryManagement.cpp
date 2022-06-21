#include <SPIFFS.h>
#include "VMemoryManagement.h"
#include "VInformationManager.h"
#include "VDefines.h"
#include "VUtils.h"


/********* New methodology ***********/
uint16_t IndexCountOfDatasetToBeRetreived = 0;
uint16_t IndexCountToBeAppended = 0;
String StringToBeStoredinTempFile = "";

String getRtcDate;
String getRtcTime;
String getRtcDateTime;
String getExtractedTimeStamp;
// Since the Send Device Data has to fulfill a condition, that if Time is Valid, then only send RTC Time and not then take Server time.
// Since the backup data will not have any corrupted time, we have hardcoded the RTC Status as RTC_DATE_TIME_VALID.
String RTCStathardcoded = RTC_DATE_TIME_VALID;

double RtcTime;
double RtcDate;

extern std::map<String, String> mapConnectionInfo; 
extern std::map<String, double> mapDeviceValues;                                 // Sensor Reading Values that have been read by this device.
extern std::map<String, uint32_t> mapCalibrationInfo;
extern std::map<String, String> mapDeviceInfo;                                   // DeviceID, DeviceName, OwnerID, PostalID, VicinityType etc.

// Also, Start checking for Backed Up Data on First HTTP Fail.
extern bool gbCheckForBackedUpData;
// Flag for sending the backed up data from device if any.
extern bool gbSendBackedUpData;

// For the server to know whether it's fresh data or old Backlog data.
extern bool isBackLogData;


void SaveSensorDataInSPIFFS(String getRtcDate, String getRtcTime) {

    // Keep these statuses local as they need to be cleared at every iteration.
    String result = "" ;

    // On every restart Index number won't start from 'ONE'. 
    // Instead it will take the next count from the map(memory). But map will be empty initially, so make the Index deliberately as 'ONE'.
    IndexCountToBeAppended = mapCalibrationInfo[INDEXCOUNTAPPENDED];
    if(IndexCountToBeAppended == 0) {

        Serial.println("Its a fresh dataset for saving in file. Making the Index 'One'");
        IndexCountToBeAppended = 1;
    }

    // Do write only uptill the file limit is not reached. The Index limit(240) once reached, will not permit more data for the First File. 
    // Next writing will be in the Second File. In case of the second file, the File Limit will be (400) which will be the final limit.
    if(IndexCountToBeAppended > mapCalibrationInfo[INDEX_LIMIT_FILE_TWO]) {

        Serial.println("File limit reached. Can't write more to any of the files.");

    } else {

        // write the Device data values(sensor values) to the file along with inserted timestamp.
        // Read the current RTC time and display it on serial for debugging.
        // PrintRtcTime(getRtcDateTime);
        RtcDate = getRtcDate.toDouble();
        Serial.println("The date is:");
        Serial.println(RtcDate);
        
        RtcTime = getRtcTime.toDouble();
        Serial.println("The time is:");
        Serial.println(RtcTime);
        mapDeviceValues.insert({ VMP_DATE, RtcDate });
        mapDeviceValues.insert({ VMP_TIME, RtcTime });

        result = writeDeviceDataToFileForBackup(mapDeviceValues, IndexCountToBeAppended);
        if(result == STATUS_SUCCESS) {

            Serial.println("Successfully saved the sensor data in SPIFFS for backup.");

            // Also increment the Index Value so that the new dataset will have next index value.
            IndexCountToBeAppended ++;
            
            Serial.print("The new count of the Index is: ");
            Serial.println(IndexCountToBeAppended);

            mapCalibrationInfo[INDEXCOUNTAPPENDED] = IndexCountToBeAppended;
            // Write the Index value in memory.
            writeCalibrationInfoToFile(mapCalibrationInfo);

        } else if (result != STATUS_SUCCESS) {

            Serial.println("Failed to write the sensor data into the SPIFFS for backup.");
            // No other actions can be taken.
            // TO DO: In case the file crashes, think of new alternative. Done. Can't think of anything else. lol.

            Serial.println("Deleting both /FirstBackupSensorDataInfo.inf, /SecondBackupSensorDataInfo.inf, /TemporaryBackupDataInfo.inf due to error while writing to file.");

            SPIFFS.remove(SPIFF_DEVICE_FIRST_BACKUP_SENSORDATA_FILE);
            SPIFFS.remove(SPIFF_DEVICE_SECOND_BACKUP_SENSORDATA_FILE);
            SPIFFS.remove(SPIFF_DEVICE_TEMPORARY_BACKUPDATA_FILE);

            Serial.println("Resetting both Index Count and Index Count of set data both to Zero. Starting from scratch again.");
            mapCalibrationInfo[INDEXCOUNTAPPENDED] = 0;
            mapCalibrationInfo[INDEX_COUNT_OF_SENT_BACKUP_DATA] = 0;

            // Don't read also unnecessarily. Since everything is deleted and Index is back to 'Zero', there is no data left to be read.
            // So unless a new file is made the reading won't happen till then.
            gbCheckForBackedUpData = false;
            mapCalibrationInfo[CHECK_BACKUP_DATA] = 0;
            Serial.println("The check for BackUp Data Flag has been made false.");
            writeCalibrationInfoToFile(mapCalibrationInfo);
        }

        // Erase the Time Stamp after writing the data into the file.
        // This is a very important step as the mapDeviceValues will have two unnecessary parameters ie. seperate Date and Time
        // which will be sent to the server every time SaveDeviceCurrentReadings API will be fired then. We need to send only the TimeStamp.
        mapDeviceValues.erase(VMP_DATE);
        mapDeviceValues.erase(VMP_TIME);  

    }

}


void RetrieveAndSendSensorDataFromSPIFFS() {

    // Keep these statuses local as they need to be cleared at every iteration.
    String strDeviceDataStatus = "";

    String FileStat = "";

    String strTempRes = "";

    String strBackupStat = "";

    String strResult = "";

    // Serial.println("Into the function.Retrieve the sensor data");
  
    Serial.println("Sending the most oldest data");

    // But map will be empty initially, so make the Index deliberately as 'ONE'.
    IndexCountOfDatasetToBeRetreived = mapCalibrationInfo[INDEX_COUNT_OF_SENT_BACKUP_DATA];
    if(IndexCountOfDatasetToBeRetreived == 0) {

        Serial.println("Its a fresh dataset for sending to Server. Making the Index 'One'.");
        IndexCountOfDatasetToBeRetreived = 1;
    }

    // Now if the Index of the dataset to be retreived matches that of next Index count of new incoming dataset, that means all the
    // datasets have been sent to server. So delete the /FirstBackupSensorDataInfo.inf, /SecondBackupSensorDataInfo.inf accordingly and /TemporaryBackupDataInfo.inf files.
    // These files will be created again once the wifi goes down again and data is stored again.
    if(IndexCountOfDatasetToBeRetreived == mapCalibrationInfo[INDEXCOUNTAPPENDED]) {

        Serial.println("Datasets saved in file have all been read/the limit has been reached. Deleting /FirstBackupSensorDataInfo.inf, /SecondBackupSensorDataInfo.inf accordingly and /TemporaryBackupDataInfo.inf.");

        // 241 beacuse while writing new data it will point to next Index so we need to add 1. Its actually filled till 240 ie. First File fully filled. 
        if(IndexCountOfDatasetToBeRetreived <= (FILE_LIMIT_ONE + 1)) {

            Serial.println("Removing First file since Index is less then 241.");
            SPIFFS.remove(SPIFF_DEVICE_FIRST_BACKUP_SENSORDATA_FILE);

        } else if(IndexCountOfDatasetToBeRetreived <= (FILE_LIMIT_TWO + 1)) {

            // 401 beacuse while writing new data it will point to next Index so we need to add 1. Its actually filled till 400 ie. Second File fully filled. 
            Serial.println("Removing First and Second file since Index is less then 401.");
            SPIFFS.remove(SPIFF_DEVICE_FIRST_BACKUP_SENSORDATA_FILE);
            SPIFFS.remove(SPIFF_DEVICE_SECOND_BACKUP_SENSORDATA_FILE);
        } else {

            Serial.println("Please check the Index Count for removing file.");
        }
        
        // In above both cases, remove the temporary file.
        SPIFFS.remove(SPIFF_DEVICE_TEMPORARY_BACKUPDATA_FILE);

        Serial.println("Resetting both Index Count and Index Count of set data both to Zero.");
        mapCalibrationInfo[INDEXCOUNTAPPENDED] = 0;
        mapCalibrationInfo[INDEX_COUNT_OF_SENT_BACKUP_DATA] = 0;

        gbCheckForBackedUpData = false;
        mapCalibrationInfo[CHECK_BACKUP_DATA] = 0;
        Serial.println("The check for BackUp Data Flag has been made false.");
        writeCalibrationInfoToFile(mapCalibrationInfo);

    } else {

        strResult = readBackUpStringFromFile(IndexCountOfDatasetToBeRetreived, mapDeviceValues, StringToBeStoredinTempFile, FileStat);
        // Now store the Extracted String of that particular Index in a Temporary File.
        if( (strResult == STATUS_SUCCESS) && (FileStat == STATUS_SUCCESS) ) {

            Serial.println("String received from file successfully. Saving it in a temporary file.");
            strTempRes = writeBackupDataIntoTempFile(StringToBeStoredinTempFile);

        } else {

            Serial.println("Issue in reading the whole backed up data as String or issue in file reading.");
            Serial.println(FileStat);
            Serial.println(strResult);
        }

        delay(10);

        // Next Read the extracted device data String stored in the temporary file and push it in the map.
        if(strTempRes == STATUS_SUCCESS) {

            Serial.println("Stored into temp file successfully. Extracting BackUp String from the temporary file.");
            strBackupStat = readBackedUpDataInfo(mapDeviceValues, getExtractedTimeStamp);

        } else {

            Serial.println("Issue in writing extracted BackUp String into the temporary file.");
            Serial.println(strTempRes);
        }

        // Send the Backup Data only if its successfully retreived from SPIFFS.    
        if(strBackupStat == STATUS_SUCCESS) {

            Serial.println("Successfully retreived the sensor data from the SPIFFS.");
            Serial.println("********************************************************");

            // Now if the Sensor data along with the TimeStamp is successfully retreived from the SPIFFS,
            // Then send the file data to the server,and check the Status too later.
            isBackLogData = true;
            strDeviceDataStatus = sendDeviceDataToServer(mapConnectionInfo, mapDeviceInfo, mapDeviceValues, getExtractedTimeStamp, RTCStathardcoded, isBackLogData);

            if(strDeviceDataStatus == STATUS_SUCCESS || strDeviceDataStatus == SQL_ERROR) {

                // If the RTC has some I2C issue, the data might have corrupted time and will be always pinging to the server.So don't consider that dataset too. Go for the next one.
                Serial.println("Device data response status:");
                Serial.println(strDeviceDataStatus);

                Serial.printf( "Dataset of index %d is sent successfully\r\n", IndexCountOfDatasetToBeRetreived);
                Serial.println("************************************************************************************************");

                // Now the next dataset of incremented index will be fetched if the previous dataset has been sent sucessfully to the server. 
                // So store it in a map safely.
                IndexCountOfDatasetToBeRetreived ++;
                mapCalibrationInfo[INDEX_COUNT_OF_SENT_BACKUP_DATA] = IndexCountOfDatasetToBeRetreived;
                // Save the new Index successfully in a map.
                writeCalibrationInfoToFile(mapCalibrationInfo);
                               
            } else if(strDeviceDataStatus == DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED) {

                // If the API gives HTTP_REQUEST_FAILED that means Internet has gone down again, so keep the Index count same and 
                // Make the gbSendBackedUpData as 'false' again, so Backed Up Data won't be sent to the server again.
                Serial.printf("HTTP Failed. Keeping the Index Count as it is: %d \r\n", IndexCountOfDatasetToBeRetreived);
                gbSendBackedUpData = false;

            } else {

                Serial.println("The Backed Up Data couldn't be sent to the server.");
                Serial.println(strDeviceDataStatus);
                // Make the gbSendBackedUpData as 'false' again, so Backed Up Data won't be sent to the server again.
                gbSendBackedUpData = false;
            }

        } else if(strBackupStat != STATUS_SUCCESS) {

            Serial.println("Failed to retreive the stored BackedUp String from the temporary file.Please Check.");
            Serial.println(strBackupStat);

            Serial.println("Deleting both /FirstBackupSensorDataInfo.inf, /SecondBackupSensorDataInfo.inf, /TemporaryBackupDataInfo.inf due to error while reading from file.");

            SPIFFS.remove(SPIFF_DEVICE_FIRST_BACKUP_SENSORDATA_FILE);
            SPIFFS.remove(SPIFF_DEVICE_SECOND_BACKUP_SENSORDATA_FILE);
            SPIFFS.remove(SPIFF_DEVICE_TEMPORARY_BACKUPDATA_FILE);

            Serial.println("Resetting both Index Count and Index Count of set data both to Zero. Starting and reading from scratch again.");
            mapCalibrationInfo[INDEXCOUNTAPPENDED] = 0;
            mapCalibrationInfo[INDEX_COUNT_OF_SENT_BACKUP_DATA] = 0;

            // Don't read unnecessarily. File crashes then will be handled while writing itself.
            gbCheckForBackedUpData = false;
            mapCalibrationInfo[CHECK_BACKUP_DATA] = 0;
            Serial.println("The check for BackUp Data Flag has been made false.");
            writeCalibrationInfoToFile(mapCalibrationInfo);
            
        } 

    }
}

// TODO: Manage the file size and if sent and saved Indices match then delete both the Backup File and even the Temp File. Done.


