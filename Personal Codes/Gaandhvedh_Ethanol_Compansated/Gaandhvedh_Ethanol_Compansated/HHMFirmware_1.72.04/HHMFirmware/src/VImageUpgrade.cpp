#include <Arduino.h>
#include <esp_spi_flash.h>
#include <map>
#include "esp_ota_ops.h"
#include "esp_http_client.h"

#include "VImageUpgrade.h"
#include "VCrc.h"
#include "VDefines.h"
#include "VCommunication.h"
#include "VResponseParser.h"
#include "VInformationManager.h"
#include "VDelay.h"


extern std::map<String, String> mapDeviceInfo;
extern std::map<String, String> mapConnectionInfo;
extern std::map<String, uint32_t> mapCalibrationInfo;

#define BUFFSIZE    1024

void FW_Image::setNewFwAvailability(){
    isNewFwAvailable = 1;
}


void FW_Image::resetNewFwAvailability(){
    isNewFwAvailable = 0;
}


bool FW_Image::isFWAvailable(){
    return isNewFwAvailable;
}


FW_Image image;

void image_setNewFWAvailability(){
    image.setNewFwAvailability();
}

void image_resetNewFWAvailability(){
    image.resetNewFwAvailability();
}

bool image_isNewFWAvailable(){
    return image.isFWAvailable();
}

//Display the info of current partition, in which image is running
void image_showRunningImage(){

    const esp_partition_t *running = esp_ota_get_running_partition();
    Serial.printf("Current Image running at OTA partition %d, Addr: 0x%x\r\n", 
                                (running->subtype%ESP_PARTITION_SUBTYPE_APP_OTA_MIN), running->address);
}

static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

int32_t image_upgradeFirmware(){

    int32_t retCode = IMG_UPGRADE_SUCCESSFULL;
    int32_t cal_crc = 0, img_crc = 0;

    char compURL[256]; 

    const esp_partition_t *update_partition;
    static char ota_write_data[BUFFSIZE + 1] = { 0 };

    uint32_t binary_file_length = 0;

    bool start_ota_update = false;
    bool collect_bin_from_server = false;

    DELAY ota_timeout; 

    esp_err_t err;
    esp_ota_handle_t update_handle = 0 ;
    
    //Get Info of current partition, where Image is running
    const esp_partition_t *running = esp_ota_get_running_partition();

    //Get Info of configured boot partition
    const esp_partition_t *configured = esp_ota_get_boot_partition();

    Serial.printf("IMG Upg: Rumnning Image Type %d, SubType %d, @Addr 0x%x\r\n", running->type, running->subtype, running->address);
    Serial.printf("IMG Upg: Config. Image Type %d, SubType %d, @Addr 0x%x\r\n", configured->type, configured->subtype, configured->address);

    if(running->address ==
                configured->address){
        //Selected partition points to running image
        //Get Info of next partition 
        update_partition = esp_ota_get_next_update_partition(NULL);
        Serial.printf("IMG Upg: Next Partition Type %d, SubType %d, @Addr 0x%x\r\n", update_partition->type, 
                                                                            update_partition->subtype, update_partition->address);
    } else {
        //Selected partition doesn't points running image
        //High possibility there was a unsuccessfull FW Upgradation Attempt
        //Lets try once again
        update_partition = configured;
    }

    strcpy(compURL, mapConnectionInfo[DEVC_API_URL].c_str()); 
    strcat(compURL, "vdevice/getBinForOTA/");
    strcat(compURL, mapDeviceInfo[V_DEVICE_ID].c_str());
    //strcat(compURL, "/");
    Serial.println(compURL);
    
    esp_http_client_config_t config = {
        
        //.url = "https://developmv.com/vdevice/getBinForOTA/", 
        
        .url =  compURL,      
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    if (client != NULL) {
        err = esp_http_client_open(client, 0);
        if(err == ESP_OK) {
            esp_http_client_fetch_headers(client);
            start_ota_update = true;
            collect_bin_from_server = true;
            ota_timeout.setDelay(OTA_TIMEOUT);
            Serial.printf("IMG Upg: Opened HTTP connection\r\n");

        } else {
            Serial.printf("IMG Upg: Failed to open HTTP connection: %s\r\n", esp_err_to_name(err));
            esp_http_client_cleanup(client);
            retCode = IMG_UPGRADE_FAILED_TO_OPEN_HTTP;
        }

    } else {
        retCode = IMG_UPGRADE_FAILED_TO_INIT_HTTP;
        Serial.printf("IMG Upg: Failed to initialise HTTP connection\r\n");
    }
    
    if(start_ota_update){
        //Erase the new partition area
        // Disable the WatchDog Timer.
        disableCore0WDT();
        err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
        // Enable the WatchDog Timer.
        enableCore0WDT();
        if (err == ESP_OK) {
            //Successfully Erased, now read bin file from http and write into flash
            while(collect_bin_from_server && !ota_timeout.isTimeOut()){
                int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
                if (data_read < 0) {
                    Serial.printf("IMG Upg: SSL data read error\r\n");
                    http_cleanup(client);
                } else if(data_read > 0){
                    //Print to check the contents of new bin  
                /*  for(int i=0; i<data_read; i++){
                        Serial.printf("%2x ", ota_write_data[i]);
                        if(i>0 && ((i+1)%16==0)){
                            Serial.println();
                        }
                    }
                    Serial.println();
                */
                    //Write into new partition area
                    err = esp_ota_write(update_handle, (const void *)ota_write_data, data_read);
                    if (err == ESP_OK) {
                        //Update no of bytes
                        binary_file_length += data_read;
                        if((binary_file_length%4096)==0){
                            Serial.printf("IMG Upg: Written %d bytes.\r\n", binary_file_length);
                        }
                    } else {
                        Serial.printf("IMG Upg: Not able to write into partition\r\n");
                    }                 

                } else if (data_read == 0) {
                    Serial.printf("IMG Upg: No more data received\r\n");
                    Serial.printf("IMG Upg: Total File Size %d Kb.\r\n", binary_file_length/1024);
                    collect_bin_from_server = false;
                }
            }
            //Close OTA functionality
            if (esp_ota_end(update_handle) == ESP_OK) {

                //Calculate CRC from Image
                cal_crc = command_Get_CRC(update_partition->address, binary_file_length);

                //Read CRC from Flash
                spi_flash_read((update_partition->address + binary_file_length - 4), (int32_t*)&img_crc, 4);
                Serial.printf("IMG Upg: CRC(cal) 0x%x CRC(img) 0x%x\r\n", cal_crc, img_crc);
                    
                if(cal_crc == img_crc){
                    //No bit corrupted, upgradation successfull
                    //Set this new partition as boot partition
                    Serial.printf("CRC Matched\r\n");
                    err = esp_ota_set_boot_partition(update_partition);
                    if (err != ESP_OK) {
                        //Not able set the new partition as Boot partition
                        Serial.printf("IMG Upg: Failed to set new partition as Boot partition.\r\n");
                        http_cleanup(client);
                        retCode = IMG_UPGRADE_FAILED_TO_UPD_PARTITION;

                    } else {
                        //Store the new FW-ID and UpdateKey along with flag, so that those info can be share 
                        //during device registration process after device reboot
                        // mapDeviceInfo[DEVC_SHARE_FW_INFO] = String(STR_TRUE);
                        // writeDeviceInfoToFile(mapDeviceInfo);
                        mapCalibrationInfo[DEVC_SHARE_FW_INFO] = 1;
                        writeCalibrationInfoToFile(mapCalibrationInfo);

                        Serial.printf("IMG Upg: Going to restart system\r\n");
                        esp_restart();
                    }
                } else {
                    Serial.printf("IMG Upg: CRC does not match, skipping upgrade\r\n");
                }
            } else {
                //Failed to Close OTA
                Serial.printf("IMG Upg: Failed, no data has been written.\r\n");
                http_cleanup(client);
                retCode = IMG_UPGRADE_FAILED_TO_WRITE_OTA;
            }
        
        } else {
            Serial.printf("IMG Upg: Failed to init OTA.\r\n");
            http_cleanup(client);
            retCode = IMG_UPGRADE_FAILED_TO_INIT_OTA;
        }
    }

    return retCode;
}
    

String image_getClearanceToDownloadBin(std::map<String, String>& inMapConnectionInfo,
                                        std::map<String, String>& inMapDeviceInfo){

    String lstrResult = "";

    String url = inMapConnectionInfo[DEVC_API_URL] + "vdevice/getClearanceToDownloadBinForOTA";
    String httpResposne;

    uint32_t retFromParser=0;

    String strPostBody =  String("{") +
                                "\"DeviceID\": \"" + inMapDeviceInfo[V_DEVICE_ID] + "\", " +
                                "\"DeviceAuth\": \"" + inMapDeviceInfo[V_DEVICE_AUTH] + "\", " +
                                "\"FrmwrID\": " + inMapDeviceInfo[RES_KEY_OTA_FW_ID] + ", " +
                                "\"UpdateKey\": \"" + inMapDeviceInfo[RES_KEY_OTA_UPD_KEY] + "\"" +
                                "}";
                                
    Serial.println("Sending OTA clearance request to Server"); 
    
    if(comm_communicateToServerForOTA(url, strPostBody, httpResposne)){
        
        Serial.println("Server Response");       
        retFromParser = parseOtaResponse(httpResposne);

        if(retFromParser == SUCCESS){
            lstrResult = STATUS_SUCCESS;
        }
        
    } else {
        lstrResult = DEVICE_API_RESPONSE_HTTP_REQUEST_FAILED;
        Serial.println("Http request failed.");
    }

    
  return lstrResult;
}
