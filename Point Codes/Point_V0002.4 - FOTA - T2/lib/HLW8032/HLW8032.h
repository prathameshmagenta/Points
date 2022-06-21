#ifndef HLW8032_h
#define HLW8032_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <SoftwareSerial.h>

class HLW8032
{
	public:
		HLW8032();
		void begin(HardwareSerial& SerialData,byte IO);
		void setVF(float Data); 
		void setCF(float Data) ;  
		void SerialReadLoop(); 
		float GetVol();  
		float GetVolAnalog();  
		float GetCurrent(); 
		float GetCurrentAnalog(); 
		float GetActivePower(); 
		float GetInspectingPower();
		float GetPowerFactor(); 
		uint16_t GetPF();
		uint32_t GetPFAll(); 
		float GetKWh();
		
		float CurrentPar; 		
		float CurrentData; 		
		byte SerialTemps[24] ;  
		byte SeriaDataLen =0; 
		bool SerialRead = 0;  
		
	private:
	
		bool Checksum();  
		
		byte _IO;
		HardwareSerial *SerialID;
		uint8_t SysStatus;  
		float VolPar;   
		float VolData;  
		float VF;   
		
		float CF;            
		float PowerPar;         
		float PowerData;        
		uint16_t PF;               
		uint32_t PFData = 1;         
		uint32_t VolR1 = 1880000;    //Voltage divider Upstream resistors 470K*4  1880K
		uint32_t VolR2 = 1000;       //Voltage divider downstream resistors  1K
		float CurrentRF = 1.0*1E-3;     
};
#endif