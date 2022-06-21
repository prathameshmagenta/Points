#include <Arduino.h>
#include "VCmdLineInterface.h"
#include "VInformationManager.h"
#include "VDefines.h"
#include "VSensors.h"
#include "VNH3.h"
#include "VUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_SIZE    100

#define stricmp     strcasecmp
#define strnicmp    strncasecmp


int32_t input;

CLI cli;

CLI::CLI(){
    cmdValidity = true;
    cmdLineState = 0; 
    currState = START_CLI;      
}

inline void CLI::resetCLI(){
    cmdLineState = 0;
}
    
inline void CLI::setCLI(){
    cmdLineState = 1;
}

inline bool CLI::isCliState(){
    return cmdLineState;
}

inline uint32_t CLI::getCurrentState(){
    return currState;
}

inline void CLI::setCurrentState(uint32_t state){
    currState = state; 
}

uint8_t CLI::getKeyState(){
    return keyState;
}

void CLI::setKeyState(uint8_t state){
    keyState = state;
}

void CLI::setParameter(uint8_t param){
    parameter = param;
}

uint8_t CLI::getParameter(){
    return parameter;
}

void CLI::setInvalidInput(){
    cmdValidity = true;
}

void CLI::resetInvalidInput(){
    cmdValidity = false;
}
    
bool CLI::isInvalidInput(){
    return cmdValidity;
}

uint64_t CLI::getCliActiveTime(){
    return cliEnableTimeTick; //Return in MS
}

void CLI::setCliActiveTime(){
    cliEnableTimeTick = millis();
}

void CLI::resetCliActiveTime(){
    cliEnableTimeTick = 0;
}

void CLI::setLastInputTime(){
    lastInputTimeTick = millis();
}
    
uint64_t CLI::getLastInputTime(){
    return lastInputTimeTick;
}


bool isValidUser(const char *buffer){

    bool retValue = false;
    if(strlen(buffer) >= strlen(CLI_USER_NAME)){
        if(!strncmp(buffer, CLI_USER_NAME, strlen(CLI_USER_NAME))){
            retValue = true;
        }
    }
    return retValue;
}

bool isValidPwd(const char *buffer){

    bool retValue = false;
    if(strlen(buffer) >= strlen(CLI_PWD)){ 
        if(!strncmp(buffer, CLI_PWD, strlen(CLI_PWD))){
            retValue = true;
        }
    }
    return retValue;
}

bool isValidAuthKey(const char *buffer){

    bool retValue = false;
    if(strlen(buffer) > 5){
        retValue = true;
    }
    return retValue;
}

// String getDeviceID(){

// 	uint64_t chipid;
// 	char ChipID[32];

// 	chipid = ESP.getEfuseMac();

// //	Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print Higher 2 bytes  
// //	Serial.printf("%08X\r\n",(uint32_t)chipid);//print Lower 4bytes.

// 	sprintf(ChipID, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
// 	String ChipReturn = String(ChipID);

// 	return ChipReturn;
// }

#define KEY_SIZE    80   
void cli_processCMD(std::map<String, String>& inMapDeviceInfo,
                        std::map<String, uint32_t>& inMapCalibrationInfo,
                        std::map<String, double>& inMapCalibrationSensorInfo){

    float value1=0, value2=0;
    float tempVal, humval;
    uint8_t TempCount = 0; 
    uint8_t HumCount = 0; 

//    char cmd_buffer[CMD_SIZE]; 
    char *cmd_buffer;
    int32_t cmd_index = 0;

    String devID;
    String authKey;

    uint16_t baseLine =0;
    float referenceR0 = 0.0f; 
    float newRefR0 = 0.0f;
    float oldRefR0=0;

    uint8_t tempCali = 0;
    char key[KEY_SIZE];

    // int32_t input = Serial.read();

    if(input > 0){

        cmd_buffer = (char*)malloc(CMD_SIZE*sizeof(char));

        cmd_buffer[cmd_index] = char(input);
        cmd_index = ((cmd_index + 1) % CMD_SIZE);

        Serial.print(char(input));
        input = 0;

        if((cmd_buffer[cmd_index-1] == '\r') ||
            (cmd_buffer[cmd_index-1] == '\n')){

            //Enter into CMD line mode if Enter Key is pressed
            cli.setCLI();
            cli.setCliActiveTime(); 
            cli.setLastInputTime();  
            
            //Entering into CLI mode and remains in CLI, once
            //user type EXIT or timeout
            while(cli.isCliState()){               
                //Start processing if Enter is pressed
                if((cmd_buffer[cmd_index-1] == '\r') ||
                    (cmd_buffer[cmd_index-1] == '\n')){

                    cmd_buffer[cmd_index] = '\0';
                    Serial.println();

                    //If "BACK" is entered, don't clear
                    if(((cli.getCurrentState() == PROCESS_OPTION) ||
                            (cli.getCurrentState() == ASK_SET_RTC) ||
                            (cli.getCurrentState() == ASK_REF_NH3OD)) &&
                            (!strnicmp(cmd_buffer, BACK, strlen(BACK))) ){
                        cli.setCurrentState(SHOW_OPTION);           
                    }
                    //On Save confirmation page, if wrong input (except Y/N) by user
                    if((cli.getCurrentState() == PROCESS_OPTION) &&
                        (cli.getParameter() == STORE) ){
                        
                        if(strnicmp(cmd_buffer, YES, strlen(YES)) &&
                            strnicmp(cmd_buffer, NO, strlen(NO))){
                            Serial.printf("Invalid option\r\n");
                            strcpy(cmd_buffer, SAVE);
                            cli.setCurrentState(ASK_REF);
                        } else {
                            if(!strnicmp(cmd_buffer, YES, strlen(YES))){
                                writeCalibrationInfoToFile(inMapCalibrationInfo);
                                writeCalibDoubleValuesToFile(inMapCalibrationSensorInfo);
                                Serial.printf("Saved Calibration info into Memory\r\n");
                            }
                            cli.setParameter(BLANK);
                            cli.setCurrentState(SHOW_OPTION); 
                        }          
                    }

                    switch(cli.getCurrentState()){
                        case START_CLI:
                            Serial.print("User ID: ");
                            cli.setCurrentState(VALIDATE_USER);
                        break;
                        case VALIDATE_USER:
                            if(isValidUser(cmd_buffer)){
                                Serial.print("Password: ");
                                cli.setCurrentState(VALIDATE_PWD);
                            } else {
                                cli.setCurrentState(START_CLI);
                                cli.resetCLI();
                                //Serial.printf("CLI DBG: invalid user\r\n");
                            }                          
                        break;
                        case VALIDATE_PWD:
                                if(isValidPwd(cmd_buffer)){    
                                    //Read Device Id & Auth Key from SPIFF
                                    devID = inMapDeviceInfo[V_DEVICE_ID];
                                    authKey = inMapDeviceInfo[V_DEVICE_AUTH];

                                    if((devID.length() > 0) &&
                                        (authKey.length() > 0)){

                                        Serial.print("Device ID: ");
                                        Serial.println(devID);
                                        Serial.print("Auth Key: ");
                                        Serial.println(authKey);

                                        Serial.print("Press Enter Again");
                                        cli.setCurrentState(SHOW_OPTION);
                                    } else {
                                        //If Device Id is not present
                                        if(devID.length() == 0){
                                            //Generate the ID using MAC and store it   
                                            Serial.println("Generating Device ID, as it was not found in SPIFF.");  
                                            inMapDeviceInfo[V_DEVICE_ID] = getDeviceID();                                       
                                        }
                                        //Print the Device ID on console
                                        Serial.print("Generated Device ID: ");
                                        Serial.println(inMapDeviceInfo[V_DEVICE_ID]);

                                        //If Auth Key is not present
                                        if(authKey.length() == 0){
                                            Serial.print("Enter Auth Key: ");
                                            cli.setKeyState(ASK_KEY);
                                        }
                                        cli.setCurrentState(GET_AUTH_KEY);
                                    }                      
                                    
                                } else {
                                    //Wrong pwd, reset CLI
                                    cli.setCurrentState(START_CLI);
                                    cli.resetCLI();
                                    //Serial.printf("CLI DBG: invalid pwd\r\n");
                                }
                        break;
                        case GET_AUTH_KEY:
                            switch(cli.getKeyState()){
                                case ASK_KEY:
                                    //If is seems a valid key
                                    if(isValidAuthKey(cmd_buffer)){    
                                        memset(key, '-', KEY_SIZE);
                                        strncpy(key, cmd_buffer, strlen(cmd_buffer)-1); 
                                        key[strlen(cmd_buffer)-1] ='\0';
                                        Serial.printf("Key->%s\r\n", key);                                
                                        Serial.printf("Key Length %d\r\n", strlen(cmd_buffer)-1);
                                        Serial.print("Recommended to Save it into Memory. Save? (Y/N) ");
                                        cli.setKeyState(VALID_KEY);                                       
                                    } else {
                                        Serial.print("Enter Auth Key: ");
                                    }
                                break;
                                case VALID_KEY:
                                    if(!strnicmp(cmd_buffer, YES, strlen(YES))){
                                        inMapDeviceInfo[V_DEVICE_AUTH] = String(key);
                                        writeDeviceInfoToFile(inMapDeviceInfo);
                                        Serial.printf("Saved Device info into Memory.\r\n");
                                        Serial.printf("Press Enter, then Exit and Reboot again.\r\n");
                                        cli.setCurrentState(SHOW_OPTION);                                       

                                    } else if(!strnicmp(cmd_buffer, NO, strlen(NO))){
                                        Serial.printf("Can't move further without saving device information.\r\n");
                                        Serial.print("Enter Auth Key: ");
                                        cli.setKeyState(ASK_KEY);
                                       
                                    } else {
                                        Serial.printf("Not a valid option.\r\n");
                                        Serial.print("Enter Auth Key: ");
                                        cli.setKeyState(ASK_KEY);
                                    }                                     
                                break;
                                default:
                                break;
                            }
                        break;
                        case SHOW_OPTION:                           
                            Serial.println("....Please select one option....");
                            Serial.println("Option 1: To Calibrate TEMP Sensor.");
                            Serial.println("Option 2: To Calibrate RH Sensor.");
                            Serial.println("Option 3: To Calibrate VOC Sensor.");
                            Serial.println("Option 4: To Calibrate NH3 Sensor.");
                            Serial.println("Option 5: To set the RTC.");
                            Serial.println("Type Save: To Save new calibration info.");
                            Serial.println("Type Exit: To Quit from CLI.");
                            Serial.print("vl->");
                            cli.setParameter(BLANK);
                            cli.setCurrentState(ASK_REF);                        
                        break;
                        case ASK_REF:
                            if(!strncmp(cmd_buffer, CAL_TEMP, strlen(CAL_TEMP))){                                                               
                                sensor_getParamFromHDC2080(value1, value2);
                                Serial.printf("Current Temp: %0.2fdegC\r\n", value1); 
                                sensor_readTempCali(tempCali);
                                Serial.printf("Curr Temp Cali: %x\r\n", tempCali);   tempCali = 0;
                                Serial.println("Enter Ref Temp or type \"back\"");                                
                                Serial.print("vl->");
                                cli.setParameter(TEMP);
                                cli.setCurrentState(PROCESS_OPTION);
                            } else if(!strncmp(cmd_buffer, CAL_RH, strlen(CAL_RH))){                               
                                sensor_getParamFromHDC2080(value1, value2);
                                Serial.printf("Current Humidity: %0.2f%%\r\n", value2);
                                sensor_readHumCali(tempCali);
                                Serial.printf("Curr Temp Cali: %x\r\n", tempCali);   tempCali = 0; 
                                Serial.println("Enter Ref Humidity or type \"back\"");
                                Serial.print("vl->");
                                cli.setParameter(RH);
                                cli.setCurrentState(PROCESS_OPTION);
                            } else if(!strncmp(cmd_buffer, CAL_VOC, strlen(CAL_VOC))){                               
                                if(sensor_readBaselineOfCCS(baseLine)){
                                    Serial.printf("Old VOC Base-Line: %x\r\n", inMapCalibrationInfo[VMP_VOC]); 
                                    Serial.printf("New VOC Base-Line: %x\r\n", baseLine);                                
                                    Serial.println("Enter again to copy or type \"back\"");
                                    Serial.print("vl->");
                                    cli.setParameter(VOC); 
                                    cli.setCurrentState(PROCESS_OPTION);
                                } else {
                                    Serial.println("Sorry CCS811 not ready. Try after some time.");
                                    cli.setCurrentState(SHOW_OPTION);
                                }
                            } else if(!strncmp(cmd_buffer, CAL_NH3OD, strlen(CAL_NH3OD))){
                                Serial.println("..Please Select from the following for NH3OD..");
                                Serial.printf("Option 1: To enter the PPM values for 2 Point Calibration.\r\n");
                                Serial.printf("Option 2: To Calibrate %0.2f PPM point.\r\n", inMapCalibrationSensorInfo[NH3_PPM_REF1]);
                                Serial.printf("Option 3: To Calibrate %0.2f PPM point.\r\n", inMapCalibrationSensorInfo[NH3_PPM_REF2]);
                                Serial.println("Option Back: To go back.");
                                Serial.print("vl->");
                                cli.setCurrentState(ASK_REF_NH3OD);
                            } else if(!strncmp(cmd_buffer, SET_RTC, strlen(SET_RTC))){
                                Serial.println("..Please Select the Action to be undertaken..");
                                Serial.println("Option 1: To set the RTC.");
                                Serial.println("Option 2: To view the clock error.");
                                Serial.println("Option Back: To go back.");
                                Serial.print("vl->");
                                cli.setCurrentState(ASK_SET_RTC);
                            } else if(!strnicmp(cmd_buffer, SAVE, strlen(SAVE))){                              
                                Serial.print("Are you sure? (Y/N) ");
                                cli.setParameter(STORE);
                                cli.setCurrentState(PROCESS_OPTION);
                            } else if(!strnicmp(cmd_buffer, QUIT, strlen(QUIT))){
                                cli.resetCLI();
                                //Serial.printf("CLI DBG: normal exit\r\n");
                            } else if(!strnicmp(cmd_buffer, "\r", strlen("\r"))){
                                Serial.print("vl->");
                            } else {
                                Serial.println("Invalid Option, Enter Again.");
                                Serial.print("vl->");
                            }
                            
                        break;
                        case ASK_REF_NH3OD:
                            if(!strncmp(cmd_buffer, NH3_PPM_VAL, strlen(NH3_PPM_VAL))){
                                Serial.println("..Please Select 2 PPM points for NH3..");
                                Serial.printf("Option 1: To Enter First PPM value for 2 point calibration.\r\n");
                                Serial.printf("Option 2: To Enter Second PPM value for 2 point calibration.\r\n");
                                Serial.println("Option Back: To go back.");                               
                                Serial.print("vl->");
                                cli.setParameter(NH3_PPM);
                                cli.setCurrentState(PROCESS_OPTION);
                            } else if(!strncmp(cmd_buffer, CAL_NH3OD_REF1, strlen(CAL_NH3OD_REF1))){
                                referenceR0 = sensor_readR0ofTGS(VMP_NH3OD);

                                if(referenceR0 > 0){
                                    
                                    newRefR0 = referenceR0;
                                    oldRefR0 = inMapCalibrationSensorInfo[VMP_NH3OD_REF1];

                                    Serial.printf("Old RS(%0.2fPPM) @ 20dC & 65%%: %0.3f\r\n", inMapCalibrationSensorInfo[NH3_PPM_REF1], oldRefR0); 
                                    Serial.printf("New RS(%0.2fPPM) @ 20dC & 65%%: %0.3f\r\n", inMapCalibrationSensorInfo[NH3_PPM_REF1], newRefR0);                                
                                    Serial.println("Enter again to copy or type \"back\"");
                                    Serial.print("vl->");
                                    cli.setParameter(NH3OD_REF1);
                                    cli.setCurrentState(PROCESS_OPTION);
                                } else {
                                    Serial.println("Sorry TGS2602 not ready. Try after some time.");
                                    cli.setCurrentState(SHOW_OPTION);
                                }
                            } else if(!strncmp(cmd_buffer, CAL_NH3OD_REF2, strlen(CAL_NH3OD_REF2))){
                                referenceR0 = sensor_readR0ofTGS(VMP_NH3OD);

                                if(referenceR0 > 0){
                                    
                                    newRefR0 = referenceR0;
                                    oldRefR0 = inMapCalibrationSensorInfo[VMP_NH3OD_REF2];

                                    Serial.printf("Old RS(%0.2fPPM) @ 20dC & 65%%: %0.3f\r\n", inMapCalibrationSensorInfo[NH3_PPM_REF2], oldRefR0); 
                                    Serial.printf("New RS(%0.2fPPM) @ 20dC & 65%%: %0.3f\r\n", inMapCalibrationSensorInfo[NH3_PPM_REF2], newRefR0);                                
                                    Serial.println("Enter again to copy or type \"back\"");
                                    Serial.print("vl->");
                                    cli.setParameter(NH3OD_REF2);
                                    cli.setCurrentState(PROCESS_OPTION);
                                } else {
                                    Serial.println("Sorry TGS2602 not ready. Try after some time.");
                                    cli.setCurrentState(SHOW_OPTION);
                                }
                            } else {
                                Serial.println("Invalid Option, Enter Again.");
                                Serial.print("vl->");
                            }   
                        break;
                        case ASK_SET_RTC:
                            if(!strncmp(cmd_buffer, SET_RTC_TIME, strlen(SET_RTC_TIME))){
                                Serial.println("Enter the current unixtime to set the time or to go back type \"back\"");
                                Serial.print("vl->");
                                cli.setParameter(UNIX_TIME_T);
                                cli.setCurrentState(PROCESS_OPTION);
                            } else if(!strncmp(cmd_buffer, VIEW_CLOCK_ERROR, strlen(VIEW_CLOCK_ERROR))){
                                Serial.println("Enter the current unixtime to view clock error or to go back type \"back\"");
                                Serial.print("vl->");
                                cli.setParameter(UNIX_TIME_C);
                                cli.setCurrentState(PROCESS_OPTION);    
                            } else {
                                Serial.println("Invalid Option, Enter Again.");
                                Serial.print("vl->");
                            }
                        break;
                        case PROCESS_OPTION:     
                            switch(cli.getParameter()){
                                case TEMP:    
                                    if(atof(cmd_buffer) > 0.0f){                      
                                        inMapCalibrationInfo[VMP_TEMP] = sensor_calibrateSensor(VMP_TEMP, value1, atof(cmd_buffer));
                                        if(sensor_validateCaliHDC2080(VMP_TEMP)){
                                            Serial.printf("Temp Calibration Done, Press Enter again %x.\r\n", inMapCalibrationInfo[VMP_TEMP]);
                                            TempCount ++;
                                            if(TempCount == 3) {
                                                sensor_getParamFromHDC2080(tempVal, humval);
                                                if(abs(tempVal - atof(cmd_buffer)) <= 2) {
                                                    Serial.println("Also Sensor Temp Val is within tolerance");
                                                } else {
                                                    Serial.println("But Sensor Temp Val is not within tolerance");
                                                }
                                                TempCount = 0;
                                            }                                            
                                        } else {
                                            Serial.printf("Temp Calibration Failed, Press Enter again.\r\n");
                                        }                                                                                                                   
                                        cli.setCurrentState(SHOW_OPTION);
                                        value1 = 0;
                                    } else {                               
                                        Serial.print("vl->");
                                    }
                                break;
                                case RH:       
                                    if(atof(cmd_buffer) > 0.0f){                     
                                        inMapCalibrationInfo[VMP_HUM] = sensor_calibrateSensor(VMP_HUM, value2, atof(cmd_buffer));
                                        if(sensor_validateCaliHDC2080(VMP_HUM)){
                                            Serial.printf("RH Calibration Done, Press Enter again %x.\r\n", inMapCalibrationInfo[VMP_HUM]);
                                            HumCount ++;
                                            if(HumCount == 3) {
                                                sensor_getParamFromHDC2080(tempVal, humval);
                                                if(abs(humval - atof(cmd_buffer)) <= 2) {
                                                    Serial.println("Also Sensor RH Val is within tolerance");
                                                } else {
                                                    Serial.println("But Sensor RH Val is not within tolerance");
                                                }
                                                HumCount = 0;
                                            }
                                        } else {
                                            Serial.printf("RH Calibration Failed, Press Enter again.\r\n");
                                        }                                        
                                        cli.setCurrentState(SHOW_OPTION);
                                        value2 = 0;
                                    } else {                            
                                        Serial.print("vl->");
                                    }
                                break;
                                case VOC:                            
                                    inMapCalibrationInfo[VMP_VOC] = baseLine;
                                    Serial.printf("VOC: Baseline copied. Press Enter again.\r\n");
                                    cli.setCurrentState(SHOW_OPTION);
                                break;
                                case NH3_PPM:
                                    if(!strncmp(cmd_buffer, NH3_PPM_1, strlen(NH3_PPM_1))){
                                        Serial.printf("Please Enter First PPM value for 2 point calibration.\r\n");                               
                                        Serial.print("vl->");
                                        cli.setParameter(NH3_PPM_VAL_1);
                                        cli.setCurrentState(PROCESS_OPTION);
                                    } else if(!strncmp(cmd_buffer, NH3_PPM_2, strlen(NH3_PPM_2))){
                                        Serial.printf("Please Enter Second PPM value for 2 point calibration.\r\n");                              
                                        Serial.print("vl->");
                                        cli.setParameter(NH3_PPM_VAL_2);
                                        cli.setCurrentState(PROCESS_OPTION);
                                    
                                    } else {
                                        Serial.println("Invalid Option, Enter Again.");
                                        Serial.print("vl->");
                                    }
                                break;
                                case NH3_PPM_VAL_1:
                                    if(atof(cmd_buffer) > 0.0f){                      
                                        inMapCalibrationSensorInfo[NH3_PPM_REF1] = atof(cmd_buffer);
                                        Serial.printf("First PPM Value (%0.2fPPM) Entered.Press Enter again and SAVE it.\r\n", inMapCalibrationSensorInfo[NH3_PPM_REF1]);
                                        cli.setCurrentState(SHOW_OPTION);
                                    } else {                               
                                        Serial.print("vl->");
                                    }
                                break;
                                case NH3_PPM_VAL_2:
                                    if(atof(cmd_buffer) > 0.0f){                     
                                        inMapCalibrationSensorInfo[NH3_PPM_REF2] = atof(cmd_buffer);
                                        Serial.printf("Second PPM Value (%0.2fPPM) Entered.Press Enter again and SAVE it.\r\n", inMapCalibrationSensorInfo[NH3_PPM_REF2]);
                                        cli.setCurrentState(SHOW_OPTION);
                                    } else {                               
                                        Serial.print("vl->");
                                    }
                                break;
                                case NH3OD_REF1:
                                    inMapCalibrationSensorInfo[VMP_NH3OD_REF1] = newRefR0;
                                    Serial.printf("NH3OD: RS(%0.2fPPM) %0.3f copied. Press Enter again.\r\n", inMapCalibrationSensorInfo[NH3_PPM_REF1], newRefR0);
                                    newRefR0 = 0;
                                    cli.setCurrentState(SHOW_OPTION);
                                break;
                                case NH3OD_REF2:
                                    inMapCalibrationSensorInfo[VMP_NH3OD_REF2] = newRefR0;
                                    Serial.printf("NH3OD: RS(%0.2fPPM) %0.3f copied. Press Enter again.\r\n", inMapCalibrationSensorInfo[NH3_PPM_REF2], newRefR0);
                                    newRefR0 = 0;
                                    cli.setCurrentState(SHOW_OPTION);
                                break;
                                case UNIX_TIME_T:
                                    setRTCTimeFromUNIX(cmd_buffer);
                                    Serial.println("Type back to go \"back\"");
                                    Serial.print("vl->");
                                break;
                                case UNIX_TIME_C:
                                    ViewClockError(cmd_buffer);
                                    Serial.println("Type back to go \"back\"");
                                    Serial.print("vl->");
                                break;
                                default:
                                break;
                            }
                            
                        break;
                        default:
                            cli.setCurrentState(START_CLI);
                            cli.resetCLI();
                            //Serial.printf("CLI DBG: default state\r\n");
                        break;

                    }
                 
                    memset(cmd_buffer, '-', CMD_SIZE);
                    cmd_index = 0;                                       
                }
                
                // Read from Keyboard again
                // int32_t input = Serial.read();
                if(input > 0){
                    cmd_buffer[cmd_index] = char(input);
                    if(cli.getCurrentState() != VALIDATE_PWD){
                        Serial.print(cmd_buffer[cmd_index]);
                    }
                    cmd_index = ((cmd_index + 1) % CMD_SIZE);
                    //Note down last input time
                    cli.setLastInputTime();
                } else {
                    //If no input from Keyboard for last 60(in MS) mints
                    if((millis() - cli.getLastInputTime()) > (60*1000)){
                        //Serial.printf("CLI DBG: Timeout, no i/p\r\n");
                        cli.resetCLI();
                    }
                }           
                //If User-ID & Pwd are not entered within specified time (4 sec)
                if(((cli.getCurrentState() == VALIDATE_USER) ||
                    (cli.getCurrentState() == VALIDATE_PWD)) &&
                    ((millis() - cli.getCliActiveTime()) > (4*1000))){   //Wait for 4 Sec only  
                    //Serial.printf("CLI DBG: Timeout, wrong credetial\r\n");                      
                    cli.resetCLI();
                }                 
            }
            //Exit from CLI may be due to Timeout
            //Better if all items r initialized for next time
            memset(cmd_buffer, '-', CMD_SIZE);
            cmd_index = 0;

            cli.resetCliActiveTime();
            cli.setCurrentState(START_CLI);

            Serial.printf("\r\n");
            Serial.println("Exiting CLI ");
        }

        //Release the Memory before leaving CLI.
        free(cmd_buffer);
    }
}