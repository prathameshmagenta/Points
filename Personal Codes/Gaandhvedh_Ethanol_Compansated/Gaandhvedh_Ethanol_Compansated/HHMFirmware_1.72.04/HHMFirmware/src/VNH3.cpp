#include <stdlib.h>
#include <map>
#include <math.h>

#include "VNH3.h"
#include "VDefines.h"

extern std::map<String, double> mapCalibrationSensorValueInfo;

// Compensate the Rs at 20 degC
float nh3_getRSAt20degC(float currentTemp)
{
    float factor = (0.00022 * currentTemp * currentTemp) - (0.0385 * currentTemp) + 1.688;
    Serial.printf("NH3: factor = %0.4f for T = %0.2f°C\r\n", factor, currentTemp);
    return (factor);
}

// Compensate the Rs at 65% RH
float nh3_getRSAt65percentRH(float currentRH)
{
    float factor = (0.000033 * currentRH * currentRH) - (0.0015 * currentRH) + 0.9567;
    Serial.printf("NH3: factor = %0.4f for RH = %0.2f%%\r\n", factor, currentRH);
    return (factor);
}

// Compensate the Rs at 20 degC
float ethanol_getRSAt20degC(float currentTempEth)
{

    float efactor = (0.00003 * currentTempEth * currentTempEth * currentTempEth) - (0.00275 * currentTempEth * currentTempEth) +
                    (0.0526 * currentTempEth) + 0.802;
    Serial.printf("Ethanol: %0.4f for T = %0.2f°C\r\n", efactor, currentTempEth);
    return (efactor);
}

// Compensate the Rs at 65% RH
float ethanol_getRSAt65percentRH(float currentRHEth)
{
    float efactor = -(0.0001 * currentRHEth * currentRHEth) + (0.0102 * currentRHEth) + 0.7804;
    Serial.printf("Ethanol: factor = %0.4f for RH = %0.2f%%\r\n", efactor, currentRHEth);
    return (efactor);
}

float CalculateCompensatedRS(float RawRSVal, float RSTempComp, float RSHumComp)
{

    float RSTempDiff, RSHumDiff, RSCompensated = 0.0f;
    RSTempDiff = (RSTempComp - RawRSVal);
    Serial.printf("The difference is Raw RS and Temp Comp RS is %0.2f\r\n", RSTempDiff);
    RSHumDiff = (RSHumComp - RawRSVal);
    Serial.printf("The difference is Raw RS and Hum Comp RS is %0.2f\r\n", RSHumDiff);
    RSCompensated = (RawRSVal + RSTempDiff + RSHumDiff);
    Serial.printf("The compensated RS is %0.2f\r\n", RSCompensated);

    return RSCompensated;
}

// Calculate AMMONIA PPM after Temp & RH compensation.
float nh3_calculatePPM(float currentRSNH3, float currTemp, float currHum)
{

    float rs_REF1 = 0.0f, rs_REF2 = 0.0f, ppm = 0.0f;
    if (currentRSNH3 == 0.0f)
        return -1.0f;

    // If calibration not done. This check is very important so as to getting invalid output due to division by 'Zero'.
    if ((mapCalibrationSensorValueInfo[VMP_NH3OD_REF1] == 0) ||
        (mapCalibrationSensorValueInfo[VMP_NH3OD_REF2] == 0) ||
        (mapCalibrationSensorValueInfo[NH3_PPM_REF1] == 0) ||
        (mapCalibrationSensorValueInfo[NH3_PPM_REF2] == 0))
        return -1.0f;

    // For First Step we have choosen the Min RS value and stored its precision by using another Map.Its already at 20 degrees, so no need to compensate again.
    rs_REF1 = mapCalibrationSensorValueInfo[VMP_NH3OD_REF1];

    // For Second Step we have choosen the Min RS value and stored its precision by using another Map.Its already at 20 degrees, so no need to compensate again.
    rs_REF2 = mapCalibrationSensorValueInfo[VMP_NH3OD_REF2];

    // Since we are not using pointers, we are simply saving the decimal precision by using another Map.
    float NH3FirstPPMVal = mapCalibrationSensorValueInfo[NH3_PPM_REF1];

    // Since we are not using pointers, we are simply saving the decimal precision by using another Map.
    float NH3SecondPPMVal = mapCalibrationSensorValueInfo[NH3_PPM_REF2];

    ppm = ((log10(NH3SecondPPMVal / NH3FirstPPMVal) * log10(currentRSNH3 / rs_REF1)) + (log10(rs_REF2 / rs_REF1) * log10(NH3FirstPPMVal))) / log10(rs_REF2 / rs_REF1);
    ppm = pow(10, ppm);

    if (ppm > 30.0f)
        ppm = 30.0f;
    Serial.printf("NH3: RS(%0.2fPPM) = %0.2f, RS(%0.2fPPM) = %0.2f, RS(Compensated) = %0.2f, (NH3)PPM = %0.2f\r\n",
                  NH3FirstPPMVal, rs_REF1, NH3SecondPPMVal, rs_REF2, currentRSNH3, ppm);
    Serial.println();

    return ppm;
}

// Calculate ETHANOL PPM after Temp & RH compensation.

float Ethanol_calculatePPM(float currentRSEthanol, float currTempe, float currHume)
{
    float rs_REF1_Eth = 0.0f, rs_REF2_Eth = 0.0f, Ethanolppm = 0.0f;
    if (currentRSEthanol == 0.0f)
        return -1.0f;

    // If calibration not done. This check is very important so as to getting invalid output due to division by 'Zero'.
    if ((mapCalibrationSensorValueInfo[VMP_ETHANOL_REF1] == 0) ||
        (mapCalibrationSensorValueInfo[VMP_ETHANOL_REF2] == 0) ||
        (mapCalibrationSensorValueInfo[ETHANOL_PPM_REF1] == 0) ||
        (mapCalibrationSensorValueInfo[ETHANOL_PPM_REF2] == 0))
    {
        return -1.0f;
    }

    // For First Step we have choosen the Min RS value and stored its precision by using another Map.Its already at 20 degrees, so no need to compensate again.
    rs_REF1_Eth = mapCalibrationSensorValueInfo[VMP_ETHANOL_REF1];

    // For Second Step we have choosen the Min RS value and stored its precision by using another Map.Its already at 20 degrees, so no need to compensate again.
    rs_REF2_Eth = mapCalibrationSensorValueInfo[VMP_ETHANOL_REF2];

    // Since we are not using pointers, we are simply saving the decimal precision by using another Map.
    float EthFirstPPMVal = mapCalibrationSensorValueInfo[ETHANOL_PPM_REF1];

    // Since we are not using pointers, we are simply saving the decimal precision by using another Map.
    float EthSecondPPMVal = mapCalibrationSensorValueInfo[ETHANOL_PPM_REF2];

    Ethanolppm = ((log10(EthSecondPPMVal / EthFirstPPMVal) * log10(currentRSEthanol / rs_REF1_Eth)) + (log10(rs_REF2_Eth / rs_REF1_Eth) * log10(EthFirstPPMVal))) / log10(rs_REF2_Eth / rs_REF1_Eth);
    Ethanolppm = pow(10, Ethanolppm);

    if (Ethanolppm > 10.0f)
    {
        Ethanolppm = 10.0f;
    }
    Serial.printf("Ethanol: EthRS(%0.2fPPM) = %0.2f, EthRS(%0.2fPPM) = %0.2f, EthRS(Compensated) = %0.2f, (Ethanol)PPM = %0.4f\r\n",
                  EthFirstPPMVal, rs_REF1_Eth, EthSecondPPMVal, rs_REF2_Eth, currentRSEthanol, Ethanolppm);
    Serial.println();
    return Ethanolppm;
}
