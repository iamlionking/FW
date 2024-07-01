#ifndef __ECG_INTERNAL_FLASH_H__
#define __ECG_INTERNAL_FLASH_H__

#include <stdbool.h>
#include <stdint.h>

#define ECG_INTERNAL_FLASH_SIZE					0x100000
#define ECG_INTERNAL_FLASH_UPDATE_OFFSET		0x7C000
#define ECG_INTERNAL_FLASH_BLOCK_SIZE			0x4000
#define ECG_INTERNAL_FLASH_UPDATE_BLOCK_COUNT	31
#define ECG_INTERNAL_FLASH_UPDATE_MAXIMUM_SIZE	0x7C000

void ECG_Internal_Flash_Erase();
void ECG_Internal_Flash_Read_Program_Info(uint32_t *size, uint32_t *data_crc);
void ECG_Internal_Flash_Write_Program_Info(uint32_t size, uint32_t crc);
void ECG_Internal_Flash_Read_Program_Data(uint32_t *data, uint32_t len);
void ECG_Internal_Flash_Start_Read_Program_Data(void);
void ECG_Internal_Flash_Stop_Read_Program_Data(void);
void ECG_Internal_Flash_Start_Write_Program_Data(void);
void ECG_Internal_Flash_Write_Program_Data(uint32_t *data, uint32_t len);
uint32_t ECG_Internal_Flash_Write_Get_Program_Data_CRC();

#endif //__ECG_INTERNAL_FLASH_H__
