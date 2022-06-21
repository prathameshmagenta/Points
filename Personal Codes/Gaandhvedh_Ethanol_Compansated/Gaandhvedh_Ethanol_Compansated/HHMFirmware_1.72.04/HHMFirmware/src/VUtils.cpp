#include <Arduino.h>
#include <SPIFFS.h>
#include <FS.h>
#include "VUtils.h"
#include "VDefines.h"
#include "VSensors.h"
#include "VInformationManager.h"
#include <WiFi.h>
#include <RTCx.h>           // DS1307 RTC

unsigned long last = 0;
char chararrTimeDate[20];
String strRtcTimeDate;
char chararrOnlyTime[7];
String strRtcTime;
char chararrOnlyDate[9];
String strRtcDate;
char chararrOnlyMonth[5];
String strRtcMonth;
uint8_t RtcMonth = 0;
char chararrOnlyDay[5];
String strRtcDay;
uint8_t RtcDay = 0;
char chararrOnlyHour[5];
String strRtcHour;
uint8_t RtcHour = 0;
char chararrOnlyMin[5];
String strRtcMin;
uint8_t RtcMin = 0;
char chararrOnlySec[5];
String strRtcSec;
uint8_t RtcSec = 0;
String RtcValStat = "";
uint32_t RTCUnixT;


String TempStat = "";
String HumStat = "";
String CCSStat = "";
String NH3Stat = "";
String EthStat = "";
String RTCHStat = "";

struct RTCx::tm tm;

extern std::map<String, String> mapDeviceInfo;


void initSPIFFS() {
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    Serial.println("Please restart");
    return;
  }
  Serial.println("Format OK...");
}

String encrypt(String instrContentToEncrypt) {
  xxtea.setKey(FILE_CONTENT_ENC_KEY);
  String result = xxtea.encrypt(instrContentToEncrypt);
  
  if(result == "-FAIL-") {
    Serial.println("Encryption Failed");
    return "";
  } else {
    return result;
  }
}

String decrypt(String instrEncryptedContent) {
  xxtea.setKey(FILE_CONTENT_ENC_KEY);
  String result = xxtea.decrypt(instrEncryptedContent);

  if(result == "-FAIL-") {
    Serial.println("Decryption Failed for Encrypted content: " + instrEncryptedContent);
    return "";
  } else {
    return result;
  }
}

// Common Anode type
void setSpecifiedIndicatorColorForLED(int inLEDColor) {

  switch(inLEDColor) {
    case LED_COLOR_RED:
      setLedRgbColor(0, 1, 1); 
      break;
    case LED_COLOR_GREEN:
      setLedRgbColor(1, 0, 1); 
      break;
    case LED_COLOR_BLUE:
      setLedRgbColor(1, 1, 0); 
      break;
    case LED_COLOR_YELLOW:
      setLedRgbColor(0, 0, 1); 
      break;
    case LED_COLOR_RASPBERRY:
      setLedRgbColor(0, 1, 0); 
      break;
    case LED_COLOR_WHITE:
      setLedRgbColor(0, 0, 0);  
      break;
    case LED_COLOR_BLACK:
    default:
      // Turn off the LED
      setLedRgbColor(1, 1, 1); 
      break;
  } 
}



void setLedRgbColor(int red_light_value, int green_light_value, int blue_light_value) {
  digitalWrite(RED_LIGHT_PIN, red_light_value);
  digitalWrite(GREEN_LIGHT_PIN, green_light_value);
  digitalWrite(BLUE_LIGHT_PIN, blue_light_value);
}

// Connects to the WiFi based on the SSID and Password provided.
String connectToWifi(String instrSSID, String instrPass)
{
  String lstrRetVal = STATUS_SUCCESS;
  
  // If the SSID or the Password passed is blank then do not proceed
  if( instrSSID.length() <= 0 || instrPass.length() <= 0 ) {
    Serial.println("Cannot connect to WiFi. SSID or Password is empty/null.");
    // TODO: Set RGB color to LED to indicate WiFi Connection Failed
    digitalWrite(COMM_LED, HIGH); // Keep LED permanently ON to indicate WiFi Connection failed
    lstrRetVal = CONNECTION_INFO_IS_EMPTY;
    return lstrRetVal;
  }

  // Connect to Wifi using SSID and SSID-Password

  // First set the device to STATION mode
  if( WiFi.mode(WIFI_STA) == false ) {
      Serial.println("Could not set STATION mode");
      lstrRetVal = FAILED_TO_SET_AP;
      return lstrRetVal;
  }

  // Start connecting to WiFi. Check the status of connection in a loop for 30 seconds.
  Serial.println("Connecting to WiFi ");
  WiFi.begin(instrSSID.c_str(), instrPass.c_str());

  // Keep checking status for 60 tries with delay of 500 millseconds
  // (Overall half a minute)
  int retryCount = 0;
  bool bLEDStatus = false;
  while (WiFi.status() != WL_CONNECTED) {
    bLEDStatus = !bLEDStatus;
    // digitalWrite(COMM_LED, (bLEDStatus ? HIGH : LOW)); // Keep blinking LED during WiFi check
    
    delay(500); // Wait half a second
    Serial.print(".");
    retryCount++;
    if (retryCount > 60)
    {
      Serial.println("Could not connect to SSID:" + instrSSID);

      lstrRetVal = FAILED_TO_CONNECT;
      return lstrRetVal;
    }
  }

  Serial.print("Wifi connected successfully. Local IP: ");
  Serial.println(WiFi.localIP());

  return lstrRetVal;
}

String ReadRTCTime(String &strRtcTimeDate, String &strRtcDate, String &strRtcTime) {

  if (millis() - last > 1000) {
    last = millis();
    rtc.readClock(tm);

    // struct tm {
		// int tm_sec; // Seconds [0..59]
		// int tm_min; // Minutes [0..59].
		// int tm_hour; // Hour [0..23].
		// int tm_mday; // Day of month [1..31].
		// int tm_mon; // Month of year [0..11].
		// int tm_year; // Years since 1900.
		// int tm_wday; // Day of week [0..6] (Sunday=0).
		// int tm_yday; // Day of year [0..365]. (-1=unset).
		// int tm_isdst; // Daylight Savings flag (ignored).
	  // };

    // The Object is tm here.The Structure members are accessed using this Object tm.

    snprintf(chararrTimeDate, sizeof(chararrTimeDate), "%04d-%02d-%02d %02d:%02d:%02d",
					  tm.tm_year + 1900,
					  tm.tm_mon + 1,
					  tm.tm_mday,
					  tm.tm_hour,
					  tm.tm_min,
					  tm.tm_sec);
    
    strRtcTimeDate = chararrTimeDate;
    // Serial.println("The current TimeStamp is: ");
    // Serial.println(strRtcTimeDate);

    snprintf(chararrOnlyDate, sizeof(chararrOnlyDate), "%04d%02d%02d",
					  tm.tm_year + 1900,
					  tm.tm_mon + 1,
					  tm.tm_mday);
    
    strRtcDate = chararrOnlyDate;
    // Serial.println("The current Date is: ");
    // Serial.println(strRtcDate);

    snprintf(chararrOnlyTime, sizeof(chararrOnlyTime), "%02d%02d%02d",
					  tm.tm_hour,
					  tm.tm_min,
					  tm.tm_sec);
    
    strRtcTime = chararrOnlyTime;
    // Serial.println("The current Time is: ");
    // Serial.println(strRtcTime);

    // Check if you get garbage data for Month.
    snprintf(chararrOnlyMonth, sizeof(chararrOnlyMonth), "%02d",
                                                tm.tm_mon + 1);

    strRtcMonth = chararrOnlyMonth;
    // Serial.println("The current Month is: ");
    // Serial.println(strRtcMonth);
    RtcMonth = strRtcMonth.toInt();
    if((RtcMonth == 0 ) || (RtcMonth > 12)) {

      Serial.println("Month is above 12 means garbage data received,returning from fun.");
      RtcValStat = RTC_DATE_TIME_INVALID;
      return RtcValStat;
    } else {
      
      RtcValStat = RTC_DATE_TIME_VALID;
    }

    // Check if you get garbage data for Day.
    snprintf(chararrOnlyDay, sizeof(chararrOnlyDay), "%02d",
                                                tm.tm_mday);

    strRtcDay = chararrOnlyDay;
    // Serial.println("The current Day is: ");
    // Serial.println(strRtcDay);
    RtcDay = strRtcDay.toInt();
    if((RtcDay == 0 ) || (RtcDay > 31)) {

      Serial.println("Day is above 31 means garbage data received,returning from fun.");
      RtcValStat = RTC_DATE_TIME_INVALID;
      return RtcValStat;
    } else {

      RtcValStat = RTC_DATE_TIME_VALID;
    }

    // Check if you get garbage data for Hour.
    snprintf(chararrOnlyHour, sizeof(chararrOnlyHour), "%02d",
                                                tm.tm_hour);
    
    // strRtcHour = chararrOnlyHour;
    // Serial.println("The current Hour is: ");
    Serial.println(strRtcHour);
    RtcHour = strRtcHour.toInt();
    if(RtcHour > 24) {

      Serial.println("Hour is above 24 means garbage data received,returning from fun.");
      RtcValStat = RTC_DATE_TIME_INVALID;
      return RtcValStat;
    } else {

      RtcValStat = RTC_DATE_TIME_VALID;
    }

    // Check if you get garbage data for Min.
    snprintf(chararrOnlyMin, sizeof(chararrOnlyMin), "%02d",
                                                tm.tm_min);
    
    strRtcMin = chararrOnlyMin;
    // Serial.println("The current Min is: ");
    // Serial.println(strRtcMin);
    RtcMin = strRtcMin.toInt();
    if(RtcMin > 60) {

      Serial.println("Min is above 60 means garbage data received,returning from fun.");
      RtcValStat = RTC_DATE_TIME_INVALID;
      return RtcValStat;
    } else {

      RtcValStat = RTC_DATE_TIME_VALID;
    }

    // Check if you get garbage data for Sec.
    snprintf(chararrOnlySec, sizeof(chararrOnlySec), "%02d",
                                                    tm.tm_sec);
    
    strRtcSec = chararrOnlySec;
    // Serial.println("The current Sec is: ");
    // Serial.println(strRtcSec);
    RtcSec = strRtcSec.toInt();
    if(RtcSec > 60) {

      Serial.println("Sec is above 60 means garbage data received,returning from fun.");
      RtcValStat = RTC_DATE_TIME_INVALID;
      return RtcValStat;
    } else {

      RtcValStat = RTC_DATE_TIME_VALID;
    }
  
  } 

  return RtcValStat;

}

void PrintRtcTime(String RtcDateTime, uint32_t &RTCUnixT) {

  Serial.println("The RTC time is:");
  Serial.println(RtcDateTime);

  // RTCx::printIsotime(Serial, tm).println();
  RTCx::time_t t = RTCx::mktime(&tm);
  RTCUnixT = t;
  Serial.print("unixtime = ");
  Serial.println(t);
  Serial.println("-----");

}

void setRTCTimeFromUNIX(char *TimeVal) {

  RTCx::time_t t = atol(TimeVal);
  RTCx::gmtime_r(&t, &tm);
  rtc.setClock(&tm);
  Serial.println("Clock set");
  Serial.println(TimeVal);
  RTCx::printIsotime(Serial, tm);
  Serial.println("~~~~~");

}

void ViewClockError(char *ErrorVal) {

  RTCx::time_t pcTime = atol(ErrorVal);
  rtc.readClock(&tm);
  RTCx::time_t mcuTime = RTCx::mktime(&tm);
  Serial.print("MCU clock error: ");
  Serial.print(mcuTime - pcTime);
  Serial.println(" s");
  Serial.println("~~~~~");

}

void ReadDeviceIDAuthKey() {

  String CMDReceivedString = "";

  // Initialise the Array as an empty Array, so that it can save only new data. Old data stored will be flushed.
  String StringSplits[2] = {};

  if(Serial.available() > 0) {

    // "_SerialCommunicationEnd" is the delimiter from which we will get know serial communicaton has been ended.

    CMDReceivedString = (Serial.readStringUntil('\n'));

    if (CMDReceivedString.length() > 0) {

      // Passing DeviceID from the Serial Communication.
      if (CMDReceivedString == "getDeviceID") {

        // Here we are wrapping a DeviceID between two keys which will help us to retrive required information at 
        // Flashing station and if there are any other Serial prints then it will ignore them.
        String responseToSend = "RetrievedDeviceIDStart_" + String(getDeviceID()) + "_RetrievedDeviceIDEnd_SerialCommunicationEnd";

        Serial.println(responseToSend);

      } else if(CMDReceivedString == "testHardware") {
        getSensorInfo(TempStat, HumStat, CCSStat, NH3Stat, EthStat, RTCHStat);
        String responseToSend = "RetrievedHWTestInfoStart_" + String("{\"HUM\": \"" + HumStat + "\", \"TEMP\": \"" + TempStat + "\", \"VOC\": \"" + CCSStat + "\", \"NH3OD\": \"" + NH3Stat + "\", \"ETHNL\": \"" + EthStat + "\", \"RTC\": \"" + RTCHStat + "\"}") + "_RetrievedHWTestInfoEnd_SerialCommunicationEnd";
        //String responseToSend = "RetrievedHWTestInfoStart_" + String("{\"HUM\": \"OK\", \"TEMP\": \"OK\", \"VOC\": \"OK\", \"NH3OD\": \"OK\"}") + "_RetrievedHWTestInfoEnd_SerialCommunicationEnd";

        Serial.println(responseToSend);

      } else {

        bool bHasRequiredKey = false;
        String strDeviceAuth = "";
        String retValue = "";
        
        // The sent Authkey will be in below format:
        // "setAuthKey: $gdbjdnn13vjdnnch.ehndhhd79ndkkdmn :"
        // ":" is string separator.
        for (int i = 0; i < 2; i++)	{
          StringSplits[i] = GetStringPartAtSpecificIndex(CMDReceivedString, ':', i);	//give this function your string to split, your char to split and the index of the StringPart you want to get

          if(StringSplits[i].length() > 0) {
            // Extracting AuthKey from serial communiaction and storing the same in the spiff file.
            if(i==0 && StringSplits[i] == "setAuthKey") {
              bHasRequiredKey = true;
            }

            if(i==1 && bHasRequiredKey == true) {
              strDeviceAuth = StringSplits[i];
            }
          }
        }

        if(bHasRequiredKey && strDeviceAuth.length() > 0) {
          mapDeviceInfo[V_DEVICE_ID] = getDeviceID();
          mapDeviceInfo[V_DEVICE_AUTH] = String(strDeviceAuth);

          writeDeviceInfoToFile(mapDeviceInfo);
          
          // Get the Device Information if it is present in SPIFFS file. While writing back from the file, the map is cleared. 
          // So if the .inf has nothing saved, then nothing will be written back to the map and we will get error as 'CANNOT_OPEN_FILE_IN_READ_MODE'. 
          // Also since map is cleared initially before writing we will get map size as 'ZERO'.
          retValue = readDeviceInfoFromFile(mapDeviceInfo);

          // Due to any reason, the mapDeviceInfo is empty meaning the ID and Auth are not stored in the map then 
          // send message as 'ERROR' only and not 'SUCCESS'. Keep Yellow LED on to indicate this situation.
          if(mapDeviceInfo.size() <= 0) {

            Serial.println("SavedAuthStatusStart_ERROR_SavedAuthStatusEnd_SerialCommunicationEnd");
            // Set the color of LED to Yellow.
            setSpecifiedIndicatorColorForLED(LED_COLOR_YELLOW);

          } else if( (mapDeviceInfo[V_DEVICE_ID].length() > 0) && (mapDeviceInfo[V_DEVICE_AUTH].length() > 0) ) {

            // Here we are wrapping a Status between two keys which will help us to retrive required information at 
            // Flashing station and if there are any other Serial prints then it will ignore them.
            Serial.println("SavedAuthStatusStart_" + String(retValue) + "_SavedAuthStatusEnd" + "_SerialCommunicationEnd");
          }

        } else {

          // Here we are wrapping a Status between two keys which will help us to retrive required information at 
          // Flashing station and if there are any other Serial prints then it will ignore them.
          Serial.println("SavedAuthStatusStart_ERROR_SavedAuthStatusEnd_SerialCommunicationEnd");
        }
      }
    }
  }
}


String GetStringPartAtSpecificIndex(String StringToSplit, char SplitChar, int StringPartIndex) {
  
	String originallyString = StringToSplit;

	String outString = "";

	for (int i1 = 0; i1 <= StringPartIndex; i1++)
	{
		outString = "";										//if the for loop starts again reset the outString (in this case other part of the String is needed to take out)
		int SplitIndex = StringToSplit.indexOf(SplitChar);	//set the SplitIndex with the position of the SplitChar in StringToSplit

		if (SplitIndex == -1)								//is true, if no Char is found at the given Index
		{
			//outString += "Error in GetStringPartAtSpecificIndex: No SplitChar found at String '" + originallyString + "' since StringPart '" + (i1-1) + "'";		//just to find Errors
			return outString;
		}
		for (int i2 = 0; i2 < SplitIndex; i2++)
		{
			outString += StringToSplit.charAt(i2);			//write the char at Position 0 of StringToSplit to outString
		}
		StringToSplit = StringToSplit.substring(StringToSplit.indexOf(SplitChar) + 1);	//change the String to the Substring starting at the position+1 where last SplitChar found
	}
	return outString;
}


String getDeviceID(){

	uint64_t chipid;
	char ChipID[32];

	chipid = ESP.getEfuseMac();

//	Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print Higher 2 bytes  
//	Serial.printf("%08X\r\n",(uint32_t)chipid);//print Lower 4bytes.

	sprintf(ChipID, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
	String ChipReturn = String(ChipID);

	return ChipReturn;
}