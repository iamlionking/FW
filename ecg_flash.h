#ifndef __ECG_FLASH_H__
#define __ECG_FLASH_H__

/* Flash Commands */
#define ECG_FLASH_WRITE_ENABLE          0x06
#define ECG_FLASH_WRITE_DISABLE         0x04
#define ECG_FLASH_READ_IDENTIFICATION   0x9F
#define ECG_FLASH_READ_STATUS_REG       0x05
#define ECG_FLASH_WRITE_TO_LOCK_REG     0xE5
#define ECG_FLASH_WRITE_STATUS_REG      0x01
#define ECG_FLASH_READ_LOCK_REG         0xE8
#define ECG_FLASH_READ_DATA_BYTES       0x03
#define ECG_FLASH_FAST_READ_BYTES       0x0B
#define ECG_FLASH_PAGE_WRITE            0x0A
#define ECG_FLASH_PAGE_PROGRAM          0x02
#define ECG_FLASH_PAGE_ERASE            0xDB
#define ECG_FLASH_SUBSECTOR_ERASE       0x20
#define ECG_FLASH_SECTOR_ERASE          0xD8
#define ECG_FLASH_BULK_ERASE            0xC7
#define ECG_FLASH_DEEP_POWER_DOWN       0xB9
#define ECG_FLASH_RELEASE_POWER_DOWN    0xAB

/* Status Register Bit Masks */
#define ECG_FLASH_SR_WIP_MASK           0x01
#define ECG_FLASH_SR_WEL_MASK           0x02
#define ECG_FLASH_SR_BP0_MASK           0x03
#define ECG_FLASH_SR_BP1_MASK           0x04
#define ECG_FLASH_SR_RESERVED_MASK      0x70
#define ECG_FLASH_SR_SRWD_MASK          0x80

/* SSI Bus */
#define ECG_FLASH_SSI_BUS               SSI0_BASE

/* Flash Memory Map */
#define ECG_FLASH_PROGRAM_SIZE_ADDRESS      0x000000
#define ECG_FLASH_PROGRAM_DATA_ADDRESS      0x000100

void ECG_Flash_Init_Access(void);
void ECG_Flash_Get_Program_Info(uint32_t *size, uint32_t *data_crc);
void ECG_Flash_Read_Program_Data(uint8_t *data, uint32_t len);
void ECG_Flash_Set_Program_Info(uint32_t size, uint32_t data_crc);
void ECG_Flash_Start_Read_Program_Data(void);
void ECG_Flash_Stop_Read_Program_Data(void);
void ECG_Flash_Start_Write_Program_Data(void);
void ECG_Flash_Write_Program_Data(uint8_t *data, uint16_t len);
uint32_t ECG_Flash_Write_Get_Program_Data_CRC();

#endif