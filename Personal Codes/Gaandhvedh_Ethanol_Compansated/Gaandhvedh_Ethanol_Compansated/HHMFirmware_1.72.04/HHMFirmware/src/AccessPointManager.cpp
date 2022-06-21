#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <map>
#include <WebServer.h>
#include "AccessPointManager.h"
#include "VDefines.h"
#include "VUtils.h"
#include "VInformationManager.h"
#include "WifiSetupPage.h"


bool isWebPageAccessed = false;

WebServer apWebSrvr(80);

File fsUploadFile;

String switchDeviceToAPServerMode(std::map<String, String>& inMapDeviceInfo) {

    String lstrRetVal = AP_STATUS_SUCCESS;
/*
    String DeviceName = inMapDeviceInfo[V_DEVICE_NAME];
    Serial.println("Saved Device Name: [" + DeviceName + "]");
  
    // If the Device Name is still empty then use default device name
    // to be shown in 'AP' mode.
    if(DeviceName.length() <= 0) {
      DeviceName = DEVICE_DEFAULT_NAME_IN_SERVER_MODE;
    } else {
      DeviceName = String(DEVICE_NAME_PREFIX_IN_SERVER_MODE) + " ( " + DeviceName + " )";
    }
*/
    String deviceID = inMapDeviceInfo[V_DEVICE_ID];

    if(deviceID.length() <= 0) {
      return FAILED_MISSING_INFO;
    }

    String DeviceName = String("HHM_" + deviceID);

    // Set the device to 'AP' Mode  
    if( WiFi.mode(WIFI_AP) == false ) {
        Serial.print("Could not set AP mode");
        lstrRetVal = FAILED_TO_SET_AP;
        return lstrRetVal;
    }

    // Configure the AP mode with SSID name and pwd, so that user can connect and configure 
    // if( WiFi.softAP(DeviceName.c_str(), DEVICE_DEFAULT_PASSWORD_IN_SERVER_MODE) == false ) {
    //     Serial.print("Could not set softAP mode");
    //     lstrRetVal = FAILED_TO_SET_SOFTAP;
    //     return lstrRetVal;
    // }

    if( WiFi.softAP(DeviceName.c_str(), NULL) == false ) {
        Serial.print("Could not set softAP mode");
        lstrRetVal = FAILED_TO_SET_SOFTAP;
        return lstrRetVal;
    }

    Serial.println("Server Mode Device Name: " + DeviceName);
    Serial.print("Server Mode IP: ");
    Serial.println(WiFi.softAPIP());

    String strInitAPServerResult = InitializeDeviceAPModeServer();

    if( strInitAPServerResult != AP_STATUS_SUCCESS ) {
      Serial.print("Could not Initialize AP Mode Server");
      lstrRetVal = FAILED_TO_INIT_AP_SRVR;
      return lstrRetVal;
    }

    return AP_STATUS_SUCCESS;
}


String InitializeDeviceAPModeServer() {

    String lstrRetVal = AP_STATUS_SUCCESS;
    // If the user invokes a route which is not supported,
    // then return 404
    apWebSrvr.onNotFound([]() {
        Serial.println("Handling all other routes other than the ones explicitly handled by the code. Anything after the / will be streamed back as a file.");

        // When the Web Page has been opened ie. 192.168.4.1, then esp will stop reading sensor data, otherwise the Wifi scanning doesn't work.
        isWebPageAccessed = true;

        // Handle all routes other than the ones explicitly handled by the code. 
        // Anything after the '/' will be streamed back as a file.
        if ( handleFileRead(apWebSrvr.uri()) == false ) {
            apWebSrvr.send(404, "text/plain", "FileNotFound");
        }
    });

    // first callback is called after the request has ended with all parsed arguments
    // second callback handles file uploads at that location
    // TODO: Check how for the very first time, the ".cfg" uploaded from curl file gets converted to ".inf"
    // apWebSrvr.on("/edit", HTTP_POST, []() { apWebSrvr.send(200, "text/plain", ""); }, handleFileUpload);

    // Route used by "index.html" to show the list of available SSIDs to the user
    apWebSrvr.on("/getSSIDList", HTTP_GET, getSSIDList);

    // Route used by "index.html" to save the inputs given by the user
    apWebSrvr.on("/saveSSIDAndDeviceDetails", HTTP_PUT, saveSSIDAndDeviceDetails);

    apWebSrvr.begin(); // Returns void so no check needed
    Serial.println("HTTP apWebSrvr started");

    return AP_STATUS_SUCCESS;

}

String getContentType(String filename)
{
  if (apWebSrvr.hasArg("download"))
    return "application/octet-stream";
  else if (filename.endsWith(".htm"))
    return "text/html";
  else if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".jpg"))
    return "image/jpeg";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".xml"))
    return "text/xml";
  else if (filename.endsWith(".pdf"))
    return "application/x-pdf";
  else if (filename.endsWith(".zip"))
    return "application/x-zip";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  else if (filename.endsWith(".CFS"))
    return "text/json";
  return "text/plain";
}

void handleRoot() {
  String s = WifiSetupPage;
  apWebSrvr.send(200, "text/html", s);
}

bool handleFileRead(String path)
{
  // DBG_OUTPUT_PORT.println("handleFileRead: " + path);

  Serial.println("Handle File Read called. Path: ");

  if (path.endsWith("/")) {
    // path += "index.html";
    handleRoot();
    return true;
  }
  String contentType = getContentType(path);

  if (SPIFFS.exists(path))
  {
    File file = SPIFFS.open(path, "r");
    apWebSrvr.streamFile(file, contentType);

    // size_t sent = apWebSrvr.streamFile(file, contentType);
    // Serial.println(sent);

    file.close();
    return true;
  }

  return false;
}

// TODO: Check how for the very first time, the ".cfg" uploaded from curl file gets converted to ".inf"
void handleFileUpload() {
    
    Serial.print("handleFileUpload Name: ");
    if (apWebSrvr.uri() != "/edit")
    return;
    HTTPUpload &upload = apWebSrvr.upload();
    if (upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        if (!filename.startsWith("/")) {
            filename = "/" + filename;
        }
        Serial.print("handleFileUpload Name: ");
        Serial.println(filename);

        fsUploadFile = SPIFFS.open(filename, "w");
        filename = String();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
        if (fsUploadFile)
            fsUploadFile.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (fsUploadFile) {
            fsUploadFile.close();
        }
        //DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
    }
}

// Gets the list of SSIDs available around the device
void getSSIDList() {

    Serial.println("getSSIDList called.");

    delay(500);
    int numberOfNetworks = WiFi.scanNetworks();
    String Networks = "";

    Serial.println("Number of Networks:" + String(numberOfNetworks));

    for (int i = 0; i < numberOfNetworks; i++)
    {
        // Serial.print("Network name: ");
        // Serial.println(WiFi.SSID(i));
        // Serial.print("Signal strength: ");
        // Serial.println(WiFi.RSSI(i));
        // Serial.println("-----------------------");
        
        Networks += WiFi.SSID(i) + ","; // TODO: There should not be a trailing comma
    }

    apWebSrvr.send(200, "plain/text", Networks);
}

// Saves the details that have been entered by the user in the Access Point page
// on the browser when the device is in the Server Mode
void saveSSIDAndDeviceDetails() {

  String lstrRetValFromConnectionInfo = "";
  String lstrRetValFromDeviceInfo = "";

  Serial.println("saveSSIDAndDeviceDetails called.");

  String SSID = apWebSrvr.arg("SSID");
  String Password = apWebSrvr.arg("Password");
  String DeviceName = apWebSrvr.arg("DeviceName");
  String EmailID = apWebSrvr.arg("EmailID");
  String Pincode = apWebSrvr.arg("Pincode");
  String RoomType = apWebSrvr.arg("RoomType");

  mapConnectionInfo[DEVC_NW_SSID] = SSID;
  mapConnectionInfo[DEVC_NW_SSID_PASSWORD] = Password;

  mapDeviceInfo[V_DEVICE_NAME] = DeviceName;
  mapDeviceInfo[V_USER_EMAIL_ID] = EmailID;
  mapDeviceInfo[V_DEVICE_ROOMTYPE] = RoomType;
  mapDeviceInfo[V_DEVICE_POSTALCODE] = Pincode;

  // initSPIFFS(); // TODO: Delete if not required

  // Save this information in SPIFF so that it can be used after device is restarted
  lstrRetValFromConnectionInfo = writeConnectionInfoToFile(mapConnectionInfo);
  Serial.println("Connection Info Ret Val:");
  Serial.println(lstrRetValFromConnectionInfo);
  lstrRetValFromDeviceInfo = writeDeviceInfoToFile(mapDeviceInfo);
  Serial.println("DeviceInfo Info Ret Val:");
  Serial.println(lstrRetValFromDeviceInfo);
 
  if(lstrRetValFromConnectionInfo != STATUS_SUCCESS || lstrRetValFromDeviceInfo != STATUS_SUCCESS) {
    apWebSrvr.send(200, "plain/text", "SPIFFS_ISSUE");
  } else {
    String strSuccess = "SUCCESS";
    String browserServerUrl = BROWSER_SERVER_URL;
    String strResponse = strSuccess + "|" + browserServerUrl;
    // apWebSrvr.send(200, "plain/text", "SUCCESS");
    Serial.println(strResponse);
    apWebSrvr.send(200, "plain/text", strResponse.c_str());
  }

  delay(5000);
  // Restart the device so that it can use the new values entered by the user
  ESP.restart();
}

void handleAPModeClient() {
    apWebSrvr.handleClient();
}

