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
void send_data_app(uint8_t *dev_to_app_frame, uint8_t length);
bool Start_message_identification(void);
bool device_connect_status();
bool Stop_message_identification(void);
bool KWh_Set_identification(void);

//********************************************************************************************
// End of File
//********************************************************************************************