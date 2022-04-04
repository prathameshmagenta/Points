//********************************************************************************************
// BLE Header File
//********************************************************************************************
#include <Arduino.h>

//********************************************************************************************
// Extern Global Varibale
//********************************************************************************************
extern uint8_t adv_raw_data[];

//********************************************************************************************
// Extern Function
//********************************************************************************************
void ble_init(void);
void data_communication(void);
void send_data_app(uint8_t *dev_to_app_frame, uint16_t length);
bool device_connect_status();
String Scan_JSON_On_BLE(void);

//********************************************************************************************
// End of File
//********************************************************************************************