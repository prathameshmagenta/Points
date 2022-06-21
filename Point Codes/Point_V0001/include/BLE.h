#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class BLE
{
private:
    /* data */
public:
    BLE();
    void BLE_INIT();
    bool BLE_Connect_Status();
    void BLE_Tx();
    char *RxData();
};
