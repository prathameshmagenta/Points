#ifndef __V_IMAGEUPGRADE_H__
#define __V_IMAGEUPGRADE_H__


#include "stdint.h"


enum imageUpgradeStatus{
    IMG_UPGRADE_FAILED_TO_INIT_HTTP = -10,
    IMG_UPGRADE_FAILED_TO_OPEN_HTTP = -11,
    IMG_UPGRADE_FAILED_TO_INIT_OTA  = -12,
    IMG_UPGRADE_FAILED_TO_WRITE_OTA = -13,
    IMG_UPGRADE_FAILED_TO_UPD_PARTITION = -15,

    IMG_UPGRADE_SUCCESSFULL = 0,
    IMG_UPGRADE_SAME_VERSION = 1,
};

class FW_Image{
    bool isNewFwAvailable;
    uint32_t fw_id;
    String updateKey;

public:
    void setNewFwAvailability();
    bool isFWAvailable();
    void resetNewFwAvailability();

};

void image_setNewFWAvailability();
void image_resetNewFWAvailability();
bool image_isNewFWAvailable();

void image_showRunningImage();
int32_t image_upgradeFirmware();

String image_getClearanceToDownloadBin(std::map<String, String>& inMapConnectionInfo,
                                        std::map<String, String>& inMapDeviceInfo);


#endif