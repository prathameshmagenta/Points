//********************************************************************************************
// BLE Uitility
//********************************************************************************************
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "ble_utility.h"

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

//********************************************************************************************
// BLE Uitility
//********************************************************************************************
// uint8_t dev_to_app_frame[20];							//Device to App Communication

uint8_t app_to_dev_frame[20]; // App to Device Communication
uint8_t ble_grid_status[3] = {0x02, 0x09, 0x00}; // Update Device Status Code in ble_grid_status[2] only, Keep rest bytes as it is.
char blue_name[] = "CG_Primus_123_A"; // Change last character of array as per avalability

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
		std::string rxValue = pCharacteristic->getValue();

		if (rxValue.length() > 0)
		{
			Serial.println("*********");
			Serial.print("Received Value: ");

			for (int i = 0; i < rxValue.length(); i++)
			{
				Serial.print(rxValue[i]);
				app_to_dev_frame[i] = rxValue[i];
			}

			Serial.println();
			Serial.println("*********");
		}
	}
};

//********************************************************************************************
// BLE Initialization
//********************************************************************************************
void ble_init(void)
{
	Serial.begin(115200);

	// Create the BLE Device
	BLEDevice::init(blue_name);

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
	Serial.println("Waiting a client connection to notify...");

	esp_ble_gap_config_adv_data_raw(ble_grid_status, 3); // Upload Charger Status
	delay(500);
}

//********************************************************************************************
// Send Data to App
//********************************************************************************************
void send_data_app(uint8_t *dev_to_app_frame, uint8_t length)
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
void data_communication(void)
{
	if (!deviceConnected && oldDeviceConnected)
	{
		delay(500);								// Warm-up period
		esp_ble_gap_set_device_name(blue_name); // Bluetooth Name Change
		delay(500);

		pServer->startAdvertising(); // Preparing for New Connection

		esp_ble_gap_config_adv_data_raw(ble_grid_status, 3); // Upload Charger Status
		delay(500);

		Serial.println("start advertising");
		oldDeviceConnected = deviceConnected;
	}

	if (deviceConnected && !oldDeviceConnected)
	{
		oldDeviceConnected = deviceConnected;
	}
}

//********************************************************************************************
// End of File
//********************************************************************************************
