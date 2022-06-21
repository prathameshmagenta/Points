#ifndef __V_CMDLINEINTERFACE_H__
#define __V_CMDLINEINTERFACE_H__

#include "stdint.h"
#include <map>



#define CLI_USER_NAME				(char *)"SUDO\r"
#define CLI_USER_NAME_LENGTH		5

#define CLI_PWD				        (char *)"VILISO\r"
#define CLI_PWD_LENGTH		        7

#define CAL_TEMP                    (char *)"1\r"
#define CAL_RH                      (char *)"2\r"
#define CAL_VOC                     (char *)"3\r"
#define CAL_NH3OD                   (char *)"4\r"
#define SET_RTC                     (char *)"5\r"

#define NH3_PPM_VAL                 (char *)"1\r"
#define NH3_PPM_1                   (char *)"1\r"
#define NH3_PPM_2                   (char *)"2\r"
#define CAL_NH3OD_REF1              (char *)"2\r"
#define CAL_NH3OD_REF2              (char *)"3\r"

#define SET_RTC_TIME                (char *)"1\r"
#define VIEW_CLOCK_ERROR            (char *)"2\r"

#define BACK                        (char *)"BACK\r"

#define SAVE                        (char *)"SAVE\r"
#define QUIT                        (char *)"EXIT\r"

#define YES                         (char *)"Y\r"
#define NO                          (char *)"N\r"



enum state{
    START_CLI = 0,

    VALIDATE_USER,
    VALIDATE_PWD,

    GET_AUTH_KEY,       //3
    SHOW_OPTION,

    ASK_REF,            //5
    ASK_REF_NH3OD,

    ASK_SET_RTC,

    PROCESS_OPTION,

    QUIT_CLI,
};

enum param{
    BLANK = 0,
    TEMP = 11,
    RH,
    VOC,
    NH3_PPM,
    NH3_PPM_VAL_1,
    NH3_PPM_VAL_2,
    NH3OD_REF1,
    NH3OD_REF2,
    UNIX_TIME_T,
    UNIX_TIME_C,

    STORE,
};

enum key_stat{
    ASK_KEY = 1,
    VALID_KEY,
};


class CLI{

    bool cmdValidity;
    bool cmdLineState;
 
    uint32_t currState;
    uint8_t keyState;
    uint8_t parameter;
    uint64_t cliEnableTimeTick;     //Time in MS
    uint64_t lastInputTimeTick;
 
public:
    CLI();
    void resetCLI();
    void setCLI();
    bool isCliState();

    uint32_t getCurrentState();
    void setCurrentState(uint32_t state);

    uint8_t getKeyState();
    void setKeyState(uint8_t state);

    void setParameter(uint8_t param);
    uint8_t getParameter();

    void setInvalidInput();
    void resetInvalidInput();
    bool isInvalidInput();

    void setCliActiveTime();
    void resetCliActiveTime();
    uint64_t getCliActiveTime();

    void setLastInputTime();
    uint64_t getLastInputTime();
};


void cli_processCMD(std::map<String, String>& inMapDeviceInfo,
                        std::map<String, uint32_t>& inMapCalibrationInfo,
                        std::map<String, double>& inMapCalibrationSensorInfo);



#endif