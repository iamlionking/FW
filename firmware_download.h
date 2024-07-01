#ifndef __FIRMWARE_DOWNLOAD_H__
#define __FIRMWARE_DOWNLOAD_H__
 
/* Process:
 * State of firmware download is idle
 * Start download command arrives
 * State -> Firmware_Download_Start
 * Wait x time for size
 * Receive Size
 * State -> Firmware_Download_Received_Size
 * Wait x time for first data chunk
 * State -> Firmware_Download_Receive_Data
 * As each chunk arrives:
 *    Perform CRC check
 *    If good, write data to flash at current address
 *    Increase current address by size of bytes written; update total bytes received
 * After done writing, State -> Firmware_Download_Finished
 * Send data to alert APP software Device is restarting
 * Jump to Updater in Booterloader */

#include "device_api.h"

#define FLASH_APP_SIZE_ADDRESS              0x000000
#define FLASH_APP_DATA_START_ADDRESS        0x000100
#define FLASH_APP_DATA_CHUNK_SIZE           0x000100  //256 byte data chunks

#define FIRMWARE_DOWNLOAD_CMD_OK            0x00
#define FIRMWARE_DOWNLOAD_CMD_ERROR         0x01
#define FIRMWARE_DOWNLOAD_CMD_NO_EXT        0x02

typedef enum ECG_Firmware_Doanload_State_enum_t8
{
    Firmware_Download_State_Idle,
    Firmware_Download_State_Received_Size,
    Firmware_Download_State_Receive_Data,
    Firmware_Download_State_Finish
} ECG_Firmware_Download_State_t8;

void ECG_Firmware_Download_Init(void);
void ECG_Firmware_Download_Set_Size(const uint32_t size);
void ECG_Firmware_Download_Receive_Data(const CardeaFirmwareUpdateBlock_t *data_block);
void ECG_Firmware_Download_Finalize(void);
ECG_Firmware_Download_State_t8 ECG_Firmware_Download_GetState(void);
void ECG_Firmware_Download_Proc(void* data);

#endif