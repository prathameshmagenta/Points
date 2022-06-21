#include "Arduino.h"
#include "Define.h"
#include "MType2.h"

void CP_init(void)
{
    pinMode(CP_Pin, OUTPUT);
    pinMode(CP_Ready_Pin, INPUT_PULLUP);
    pinMode(CP_Charge_Pin, INPUT_PULLUP);
    ledcSetup(0, 1000.0, 8);
    ledcAttachPin(CP_Pin, 0);
    ledcWrite(0, 255);
}

bool Check_Type_2_Connector(bool _Type2_On)
{
    if (_Type2_On == true)
    {
        while (CP_Ready_Pin == HIGH)
            ;
        Serial.println("EV Detected on connector!");
        ledcWrite(0, uint8_t(0.2 * 255));
        while ((CP_Ready_Pin == LOW) && (pulseIn(CP_Charge_Pin, LOW) >= 0.0))
            ;
        Serial.println("EV is charge requesting!");
        ledcWrite(0, uint8_t(0.2 * 255));
        return _Type2_On;
    }
    else
    {
        return _Type2_On;
    }
}
