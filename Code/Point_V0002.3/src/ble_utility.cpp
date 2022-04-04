//********************************************************************************************
// BLE Uitility
//********************************************************************************************
#include <Arduino.h>
#include <Define.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "ble_utility.h"
#include "M_EEPROM.h"

//********************************************************************************************
// UUID Services and Characteristic
//********************************************************************************************
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//********************************************************************************************
// GLobal Variables
//********************************************************************************************
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;
std::string rxValue;

//********************************************************************************************
// BLE Uitility
//********************************************************************************************
// uint8_t dev_to_app_frame[20]; //Device to App Communication
char app_to_dev_frame[1024]; // App to Device Communication

//********************************************************************************************
// Class to Class Poly
//********************************************************************************************
class MyServerCallbacks : public BLEServerCallbacks
{
	void onConnect(BLEServer *pServer)
	{
		deviceConnected = true;
	};

	void onDisconnect(BLEServer *pServer)
	{
		deviceConnected = false;
	}
};

//********************************************************************************************
// Class to Class Poly
//********************************************************************************************
class MyCallbacks : public BLECharacteristicCallbacks
{
	void onWrite(BLECharacteristic *pCharacteristic)
	{
		rxValue = pCharacteristic->getValue();
		if (rxValue.length() > 0)
		{
			for (int i = 1; i <= rxValue.length(); i++)
				app_to_dev_frame[i] = rxValue[i];
		}
	}
};

//********************************************************************************************
// BLE Initialization
//********************************************************************************************
void ble_init(void)
{
	String blue_name = Read_EEPROM_String(EEPROM_CHARGER_ID_ADD).c_str();
	// Create the BLE Device
	BLEDevice::init(blue_name.c_str());
	// Create the BLE Server
	pServer = BLEDevice::createServer();
	pServer->setCallbacks(new MyServerCallbacks());

	// Create the BLE Service
	BLEService *pService = pServer->createService(SERVICE_UUID);

	// Create a BLE Characteristic
	pTxCharacteristic = pService->createCharacteristic(
		CHARACTERISTIC_UUID_TX,
		BLECharacteristic::PROPERTY_NOTIFY);

	pTxCharacteristic->addDescriptor(new BLE2902());

	BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
		CHARACTERISTIC_UUID_RX,
		BLECharacteristic::PROPERTY_WRITE);

	pRxCharacteristic->setCallbacks(new MyCallbacks());
	// Start the service
	pService->start();

	// Start advertising
	pServer->getAdvertising()->start();
	// Serial.println("Waiting a client connection to notify...");
	delay(500);
}

//********************************************************************************************
// Send Data to App
//********************************************************************************************
void send_data_app(uint8_t *dev_to_app_frame, uint16_t length)
{
	if (deviceConnected)
	{
		pTxCharacteristic->setValue(dev_to_app_frame, length);
		pTxCharacteristic->notify();
		delay(10);
	}
}

//********************************************************************************************
// Continous Work (Keep this function in Loop / While(1) )
//********************************************************************************************
// void data_communication(void)
// {
// 	if (!deviceConnected && oldDeviceConnected)
// 	{
// 		esp_ble_gap_set_device_name(blue_name.c_str()); // Bluetooth Name Change

// 		pServer->startAdvertising(); // Preparing for New Connection

// 		Serial.println("start advertising");
// 		oldDeviceConnected = deviceConnected;
// 	}

// 	if (deviceConnected && !oldDeviceConnected)
// 	{
// 		oldDeviceConnected = deviceConnected;
// 	}
// }

//********************************************************************************************
// Startus from App
//********************************************************************************************

bool device_connect_status()
{
	return deviceConnected;
}

String Scan_JSON_On_BLE(void)
{
	return String(rxValue.c_str());
}
//********************************************************************************************
// End of File
//********************************************************************************************
