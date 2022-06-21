
#include <stdint.h>
#include "VCalculation.h"

#include <Arduino.h>

uint32_t noOfSamples(uint32_t size, uint32_t wPointer,
                            uint32_t rPointer){
    if(rPointer > wPointer){
        return (size + wPointer - rPointer);
    } else
    return (wPointer - rPointer);
}    


//template <class T>
float calculation_calculateAvg (float *buffer, uint32_t bufferSize,
                        uint32_t writeIndex, uint32_t &readPointer) {
    float avg_value = 0, temp=0;
    int no_samples, i=0;

//    Serial.println("Avg function called");

    if(buffer==0 || bufferSize==0 ||
            (readPointer == writeIndex)){
        return 0;
    } 

    no_samples = noOfSamples(bufferSize,writeIndex,readPointer);

    if(no_samples == 1){
        avg_value = *(buffer + readPointer);
        readPointer = (readPointer+1) % bufferSize;
        return avg_value;
    }

    while(i<no_samples){
        temp = *(buffer + ((readPointer + i)%bufferSize));
        avg_value += temp; 
//        Serial.print(temp); Serial.print(" "); 
        i++;
    }
//    Serial.print("Sum: ");Serial.println(avg_value);

    avg_value = (avg_value/no_samples); 
    
    readPointer = ((readPointer+no_samples) % bufferSize);

    return avg_value;
}


float calculation_calAvgAfterFilter (float *buffer, uint32_t bufferSize,
                                uint32_t writeIndex, uint32_t &readPointer) {

    float tem_buffer[200], avg_value;
    uint32_t no_samples, i=0;
    uint32_t no_samples_to_reject;
    uint32_t temp_writeIndex;

    if(buffer==0 || bufferSize==0 ||
            (readPointer == writeIndex)){
        return 0;
    } 

    no_samples = noOfSamples(bufferSize,writeIndex,readPointer);

    if(no_samples == 1){
        avg_value = *(buffer + readPointer);
        readPointer = (readPointer+1) % bufferSize;
        return avg_value;
    }

    while(i<no_samples){
        tem_buffer[i] = *(buffer + ((readPointer + i)%bufferSize)); 
        i++;
    }

    Serial.print("No of Samples: "); Serial.println(no_samples);
     
    // Serial.println("Before Sort ");
    // for(i=0; i<no_samples; i++){
    //     Serial.print(tem_buffer[i]); Serial.print(" ");
    // }
    // Serial.println("");

    std::sort(tem_buffer, tem_buffer+no_samples);
/*
    Serial.println("After Sort ");
    for(i=0; i<no_samples; i++){
        Serial.print(tem_buffer[i]); Serial.print(" ");
    }
    Serial.println("");
*/
    no_samples_to_reject = (no_samples * REJECTION_PERCENT /200);
    temp_writeIndex = (no_samples - no_samples_to_reject);
    
    avg_value = calculation_calculateAvg(tem_buffer, 200, 
                                        temp_writeIndex, no_samples_to_reject);

    Serial.print("Avg: ");
    Serial.println(avg_value);

    readPointer = ((readPointer + no_samples) % bufferSize);
    return avg_value;
}

#define SAMPLE_SIZE_FOR_VALIDITY    3
//template<typename T>
int32_t calculation_detectValidTriggerPoint (float *buffer, uint32_t bufferSize,
                                        uint32_t writeIndex, uint32_t* readPointer,
                                        struct alarm_settings *a_setting, 
                                        float &validTriggerPoint) {

    uint32_t readIndex = *readPointer;
    int32_t retValue = NO_TRIGGER;
    
    uint32_t no_samples, i=0;
    uint32_t valid_sample = 0;
 
    if(buffer==nullptr || bufferSize==0 ||
            (readIndex == writeIndex) ||
            ((a_setting->isHighNull == 1) && 
                (a_setting->isLowNull == 1))){

        Serial.printf("Buffer Size->%d\r\n", bufferSize); 
        Serial.printf("Wptr : Rptr->%d:%d\r\n", writeIndex, readIndex); 
        Serial.printf("H-Null : L-Null->%d:%d\r\n", a_setting->isHighNull, a_setting->isLowNull); 
        retValue =  ALARM_DISABLED;
        return retValue;
    } 

    Serial.print("Read Index "); Serial.print(readIndex);
    Serial.print(", Write Index "); Serial.println(writeIndex);

    no_samples = noOfSamples(bufferSize, writeIndex, readIndex);

    if(no_samples < 3){
        return INVALID;
    }

    //Evaluate for Positive Slope, if it is not disable
    if(!a_setting->isHighNull){

        retValue = NO_VALID_TRIGGER;
        for(i=0; i<no_samples; i++){            
            //Check if there are 3 consequtive points which are greater than higher-cutoff
            if(*(buffer + ((readIndex + i) % bufferSize)) > a_setting->highCutoff){

                valid_sample++; 
                Serial.print("Found +valid sample at "); Serial.println(i);                                                                
            } else {
                valid_sample = 0;                
            }

            if(valid_sample >= SAMPLE_SIZE_FOR_VALIDITY){
                //Copy last valid sample to arg buffer
                validTriggerPoint = *(buffer+((readIndex + i) % bufferSize));
                retValue = VALID_POSITIVE_TRIGGER;
                break;
            } 
        }
    }

    //If no positive edge, check for negative edge
    if((retValue != VALID_POSITIVE_TRIGGER) &&
                        (!a_setting->isLowNull)){

        retValue = NO_VALID_TRIGGER;
        //Evaluate for Negative Slope
        for(i=0; i<no_samples; i++){
            //Check if there are 3 consequtive points which are less than lower-cutoff
            if(*(buffer+((readIndex + i) % bufferSize)) < a_setting->lowCutoff){

                valid_sample++; 
                Serial.print("Found -valid sample at "); Serial.println(i);                                                               
            } else {
                valid_sample = 0;                
            }

            if(valid_sample >= SAMPLE_SIZE_FOR_VALIDITY){
                //Copy last valid sample to arg buffer
                validTriggerPoint = *(buffer+((readIndex + i) % bufferSize));
                retValue = VALID_NEGATIVE_TRIGGER;
                break;
            }
        }
    }

    Serial.println("Evaluation done");
    Serial.print("Alert Engine-Actual Samples: ");
    for(i=0; i<no_samples; i++){
        Serial.print(*(buffer+((readIndex + i) % bufferSize))); Serial.print(" ");   
    }
    Serial.println("");

    //Update the Read Index
    *readPointer = ((readIndex + no_samples) % bufferSize);

    Serial.print("Alert final Read Index: ");
    Serial.print(*readPointer);
    Serial.print(" Write Index: ");
    Serial.println(writeIndex);

    return retValue;
}

