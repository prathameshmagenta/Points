#ifndef __V_NH3_H__
#define __V_NH3_H__

#include <Arduino.h>
#include "stdint.h"


#define TGS_REF_TEMP        20.0f
#define TGS_REF_HUM         65.0f

// Select Two Reference PPM points, do calibration and no need to change in code
#define NH3_PPM_REF1            "PPM_NH3_REF1"           //Ref1 PPM(We have to set this..according to the gas availability)
#define NH3_PPM_REF2            "PPM_NH3_REF2"           //Ref2 PPM(We have to set this..according to the gas availability)

#define ETHANOL_PPM_REF1        "PPM_ETHANOL_REF1"       //Ref1 PPM(We have to set this..according to the gas availability)
#define ETHANOL_PPM_REF2        "PPM_ETHANOL_REF2"       //Ref2 PPM(We have to set this..according to the gas availability)

float nh3_getRSAt20degC(float currentTemp);
float nh3_getRSAt65percentRH(float currentRH);
float CalculateCompensatedRS(float RawRSVal, float RSTempComp, float RSHumComp);
float ethanol_getRSAt20degC(float currentTempEth);
float ethanol_getRSAt65percentRH(float currentRHEth);
float nh3_calculatePPM(float currentRSNH3, float currTemp, float currHum);
float Ethanol_calculatePPM(float currentRSEthanol, float currTempe, float currHume);

#endif