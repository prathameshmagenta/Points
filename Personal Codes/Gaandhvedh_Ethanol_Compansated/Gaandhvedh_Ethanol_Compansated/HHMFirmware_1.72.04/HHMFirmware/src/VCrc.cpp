
#include <esp_spi_flash.h>
#include "VCrc.h"

#define SIZE	256

unsigned int crc32_table[SIZE];

//*----------------------------------------------------------------------------
//* \fn      command_Reflect
//* \brief   Reflection is a requirement for the official CRC-32 standard.
//*          You can create CRCs without it, but they won't conform to the standard.
//*          Used only by command_Init_CRC32_Table()
//* \inputs  unsigned long variable and size
//* \output  reflect value
//*----------------------------------------------------------------------------
unsigned long command_Reflect(unsigned long ref, char ch) {

	unsigned long value = 0;
	int i;

	for (i = 1; i < (ch + 1); i++) // Swap bit 0 for bit 7
	{ // bit 1 for bit 6, etc.
		if (ref & 1)
			value |= (1 << (ch - i));
		ref >>= 1;
	}
	return value;
}

//*----------------------------------------------------------------------------
//* \fn      command_Get_CRC
//* \brief   calculate CRC
//* \inputs  buffer and buffer size
//* \output  CRC
//*----------------------------------------------------------------------------
int command_Get_CRC(uint32_t ota_addr, int dwSize) {

	uint64_t crc = 0xffffffff; 
	uint32_t len, i;
	uint8_t flash_data;

	//Actual length is excluding last 4 bytes
	len = dwSize - 4;

	if (len > 0) {
		
		Serial.printf("Calculating CRC, please wait");
		for(i=0; i<len; i++){
			spi_flash_read((ota_addr + i), (uint8_t*)&flash_data, 1);
			crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ flash_data]; // Perform the algorithm on each character

			if(i%10000 == 0){
				Serial.printf(".");
			}
		}
		Serial.printf("\r\n");
		
		crc = crc ^ 0xffffffff; // Exclusive OR the result with the beginning value.		
	}
	return crc;
}

//*----------------------------------------------------------------------------
//* \fn      command_Init_CRC32_Table
//* \brief   Call this function only once to initialize the CRC table.
//* \inputs  none
//* \output  none
//*----------------------------------------------------------------------------

void command_Init_CRC32_Table(void) {
	unsigned long ulPolynomial = 0x04c11db7;
	int i, j;

	for (i = 0; i < SIZE; i++) // 256 values representing ASCII character codes.
	{
		crc32_table[i] = (command_Reflect(i, 8) << 24);
		for (j = 0; j < 8; j++)
			crc32_table[i] = (crc32_table[i] << 1)
			                    ^ (crc32_table[i] & (((unsigned long) 1) << 31) ?
					            ulPolynomial : 0);
		crc32_table[i] = command_Reflect(crc32_table[i], 32);
	}
}
