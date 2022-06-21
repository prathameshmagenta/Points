#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <map>

#include <string>

#include "VDefines.h"
#include "VCommunication.h"


#define MAX_ATTEMPT     5

void esp32::setCurrentState(uint16_t state){
    currentState = state;
}

uint16_t esp32::getCurrentState(){
    return currentState;
}

void esp32::increaseAttemptNo(){
    if(noOfAttempt < MAX_ATTEMPT){
        noOfAttempt++;
    }
}

void esp32::resetAttemptNo(){
    noOfAttempt=0;
}

bool esp32::isMaxAttempt(){
    if(noOfAttempt >= MAX_ATTEMPT){
        return true;
    } else {
        return false;
    }
}


bool comm_communicateToServer(std::map<String, String> &inMapConnectionInfo,
                              std::map<String, String> &inMapDeviceInfo,
                              const String &uri,
                              const String &postBody,
                              String &response) {

    bool retValue = false;

    //Copy URL & Post Body in a local String
    String relURI = String(uri);
    String compURL = inMapConnectionInfo[DEVC_API_URL] + relURI;

    String strPostBody = String(postBody);

    HTTPClient http;
/*
    Serial.print("Received URI:");
    Serial.println(relURI);

    Serial.print("URL:");
    Serial.println(compURL);
*/
    // To Hide Device Auth.
    Serial.print("POST-Body:");
    // Serial.println(strPostBody);

    //Pass URL and start an HTTP request
    http.begin(compURL);
    http.addHeader("Content-Type", "application/json");

    //Send the HTTP Post Body
    int httpResponseCode = http.POST(strPostBody);

    if (httpResponseCode > 0) {
        //If if was a successfull HTTP Send
        if (httpResponseCode == HTTP_CODE_OK) {

            retValue = true;

            //Return the HTTP Response 
            response = http.getString();
            Serial.println("HTTP Post success");
        } else {
            Serial.print("HTTP Post failed while Registering Device. Result: ");
            Serial.println(httpResponseCode);
        }
    } else {
        
        Serial.print("HTTP Post failed while sending Post-Body. Error Code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
    return retValue;
}


bool comm_communicateToServerForOTA(const String &url,
                                    const String &postBody,
                                    String &response) {

    bool retValue = false;

    //Copy URL & Post Body in a local String
    String compURL = String(url);

    String strPostBody = String(postBody);

    HTTPClient http;

    Serial.print("URL: ");
    Serial.println(compURL);
    
    // To Hide Device Auth.
    Serial.print("POST-Body:");
    // Serial.println(strPostBody);

    //Pass URL and start an HTTP request
    http.begin(compURL);
    http.addHeader("Content-Type", "application/json");

    //Send the HTTP Post Body
    int httpResponseCode = http.POST(strPostBody);

    if (httpResponseCode > 0) {
        //If if was a successfull HTTP Send
        if (httpResponseCode == HTTP_CODE_OK) {

            retValue = true;
            //Return the HTTP Response 
            response = http.getString();
            Serial.println("HTTP Post for OTA success");
        } else {
            Serial.print("HTTP Post for OTA failed. Result: ");
            Serial.println(httpResponseCode);
        }
    } else {
        
        Serial.print("HTTP Post for OTA failed while sending Post-Body. Error Code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
    return retValue;
}



