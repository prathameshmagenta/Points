#ifndef __V_CRC_H__
#define __V_CRC_H__

#include <Arduino.h>
#include <stdint.h>


void command_Init_CRC32_Table(void);
int command_Get_CRC(uint32_t ota_addr, int dwSize);


#endif
