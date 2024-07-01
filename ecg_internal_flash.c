#include "ecg_internal_flash.h"

#include "crc.h"

#include "driverlib/flash.h"

static uint32_t* read_cursor = 0;
static uint32_t write_cursor = 0;
static uint32_t prev_crc;

void ECG_Internal_Flash_Erase()
{
	for(int index = 0;index < ECG_INTERNAL_FLASH_UPDATE_BLOCK_COUNT; index++) {
		FlashErase(ECG_INTERNAL_FLASH_UPDATE_OFFSET + ECG_INTERNAL_FLASH_BLOCK_SIZE * index);
	}
}

void ECG_Internal_Flash_Read_Program_Info(uint32_t *size, uint32_t *data_crc)
{
	*size = *((uint32_t*)ECG_INTERNAL_FLASH_UPDATE_OFFSET);
	*data_crc = *((uint32_t*)(ECG_INTERNAL_FLASH_UPDATE_OFFSET + sizeof(uint32_t)));
}

void ECG_Internal_Flash_Write_Program_Info(uint32_t size, uint32_t data_crc)
{
	uint32_t size_data[3];
	uint32_t info_crc;

	size_data[0] = size;
	size_data[1] = data_crc;

    info_crc = crc(size_data, 8, ~0);

	size_data[2] = info_crc;

	FlashProgram(size_data, ECG_INTERNAL_FLASH_UPDATE_OFFSET, sizeof(size_data));
}

void ECG_Internal_Flash_Read_Program_Data(uint32_t *data, uint32_t len)
{
	uint32_t words = len / 4;

	if(len % 4 != 0) {
		words++;
	}

	for(uint32_t index = 0;index < words;index++) {
		*data = *read_cursor;
		data++;
		read_cursor++;
	}
}

void ECG_Internal_Flash_Start_Read_Program_Data(void)
{
	read_cursor = (uint32_t*)(ECG_INTERNAL_FLASH_UPDATE_OFFSET + sizeof(uint32_t) * 3);
}

void ECG_Internal_Flash_Stop_Read_Program_Data(void)
{
	read_cursor = 0;
}

void ECG_Internal_Flash_Start_Write_Program_Data(void)
{
	write_cursor = ECG_INTERNAL_FLASH_UPDATE_OFFSET + sizeof(uint32_t) * 3;
    prev_crc = ~0;
}

void ECG_Internal_Flash_Write_Program_Data(uint32_t *data, uint32_t len)
{
	uint32_t words = len / 4;

	if(len % 4 != 0) {
		words++;
	}

	if(write_cursor + words * sizeof(uint32_t) > ECG_INTERNAL_FLASH_UPDATE_OFFSET + ECG_INTERNAL_FLASH_UPDATE_MAXIMUM_SIZE)
	{
		// We are about to write outside of the firmware update range in flash. Abort.
		return;
	}

	FlashProgram(data, write_cursor, words * 4);
    prev_crc = crc(data, len, prev_crc);
	write_cursor += words * sizeof(uint32_t);
}

uint32_t ECG_Internal_Flash_Write_Get_Program_Data_CRC()
{
    return prev_crc;
}
