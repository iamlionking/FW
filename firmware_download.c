#include "sapphire_pub.h"

#include "device_api.h"
#include "bt_transport.h"
#include "crc.h"
#include "ecg_adc.h"

#ifdef ECG_EXTERNAL_FLASH
#include "ecg_flash.h"
#else
#include "ecg_internal_flash.h"
#endif

#include "ecg_power_ctl.h"
#include "ecg_usb.h"
#include "firmware_download.h"

static ECG_Firmware_Download_State_t8 state;
static uint32_t app_size;
static uint32_t bytes_received;
static uint32_t blocks_expected;
static uint32_t blocks_received;
static uint32_t prev_crc;


/* Public - Initializes the firmware download state machine
 * 
 * Examples
 * 
 *      ECG_Firmware_Download_Init();
 * 
 * Returns nothing
 */
void ECG_Firmware_Download_Init(void)
{
    state = Firmware_Download_State_Idle;
    prev_crc = ~0;
} /* ECG_Firmware_Download_Init() */


/* Public - Stores the size of the firmware being downloaded and
 * send an appropriate response to the PC Application.
 * 
 * Examples
 * 
 *      uint32_t size = 131000;
 * 
 *      ECG_Fimrware_Download_Set_Size(size);
 * 
 * Returns nothing
 */
void ECG_Firmware_Download_Set_Size(const uint32_t size)
{
    CardeaCmdUpdateFirmwareResponse_t response;
    uint8_t external_voltage_present = 0;
    
    response.cmd = CARDEA_CMD_UPDATE_FIRMWARE;

    external_voltage_present = ECG_Power_Get_External_Connection_State();
    if(external_voltage_present != 1)
    {
        response.status = FIRMWARE_DOWNLOAD_CMD_NO_EXT;
#ifdef ECG_HARDWARE_BLUETOOTH
        BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
        USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
        ECG_Firmware_Download_Init();
        return;
    }
    
    if(state != Firmware_Download_State_Idle)
    {
        response.status = FIRMWARE_DOWNLOAD_CMD_ERROR;
#ifdef ECG_HARDWARE_BLUETOOTH
        BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
        USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
        ECG_Firmware_Download_Init();
        return;
    }

	if(size > ECG_INTERNAL_FLASH_UPDATE_MAXIMUM_SIZE)
	{
        response.status = FIRMWARE_DOWNLOAD_CMD_ERROR;
#ifdef ECG_HARDWARE_BLUETOOTH
        BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
        USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
        ECG_Firmware_Download_Init();
        return;
	}
    
    app_size = size;
    bytes_received = 0;
    blocks_received = 0;
    blocks_expected = app_size / FLASH_APP_DATA_CHUNK_SIZE;
    if(app_size % FLASH_APP_DATA_CHUNK_SIZE != 0)
        blocks_expected++;
    
#ifdef ECG_EXTERNAL_FLASH
    /* Make sure SSI is initialized to talk to the flash chip */
    ECG_Flash_Init_Access();
#else
	ECG_Internal_Flash_Erase();
#endif
 
    response.status = FIRMWARE_DOWNLOAD_CMD_OK;

#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
    USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
    state = Firmware_Download_State_Received_Size;

#ifdef ECG_EXTERNAL_FLASH
    ECG_Flash_Start_Write_Program_Data();
#else
	ECG_Internal_Flash_Start_Write_Program_Data();
#endif
} /* ECG_Firmware_Download_Set_Size() */


/* Public - Receives a block of data and writes it to the external flash
 * and sends the appropriate response to the PC Application.
 * 
 * Examples
 * 
 *      CardeaFirmwareUpdateBlock_t data_block;
 * 
 *      ECG_Firmware_Download_Receive_Data(&data_block);
 * 
 * Returns nothing
 */
void ECG_Firmware_Download_Receive_Data(const CardeaFirmwareUpdateBlock_t *data_block)
{
    CardeaCmdUpdateFirmwareResponse_t response;
    response.cmd = CARDEA_CMD_UPDATE_FIRMWARE;
    uint32_t calculated_crc;
    uint16_t write_size;
    uint8_t external_voltage_present = 0;
    
//    Display(("Block Number: %d\r\n", data_block->block_number));
    
    external_voltage_present = ECG_Power_Get_External_Connection_State();
    if(external_voltage_present != 1)
    {
        response.status = FIRMWARE_DOWNLOAD_CMD_NO_EXT;
#ifdef ECG_HARDWARE_BLUETOOTH
        BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
        USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
        ECG_Firmware_Download_Init();
        return;
    }
    
    if(state != Firmware_Download_State_Received_Size && state != Firmware_Download_State_Receive_Data)
    {
        response.status = FIRMWARE_DOWNLOAD_CMD_ERROR;
#ifdef ECG_HARDWARE_BLUETOOTH
        BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
        USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
        ECG_Firmware_Download_Init();
        return;
    }
    
    state = Firmware_Download_State_Receive_Data;
    
    /* Verify Data */
    calculated_crc = crc(data_block, sizeof(CardeaFirmwareUpdateBlock_t), ~0);
    if(calculated_crc != 0)
    {
        Display(("Caculated CRC: %x\r\n", calculated_crc));
        Display(("Actual CRC: %x\r\n", data_block->crc));
        response.status = FIRMWARE_DOWNLOAD_CMD_ERROR + 1;
#ifdef ECG_HARDWARE_BLUETOOTH
        BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
        USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
        ECG_Firmware_Download_Init();
        return;
    }
    
    if((app_size - bytes_received) < FLASH_APP_DATA_CHUNK_SIZE)
        write_size = app_size - bytes_received;
    else
        write_size = FLASH_APP_DATA_CHUNK_SIZE;
    
    prev_crc = crc(data_block->data, write_size, prev_crc);
    
    /* Write received data to flash */

#ifdef ECG_EXTERNAL_FLASH
    ECG_Flash_Write_Program_Data((uint8_t *)data_block->data, write_size);
#else
	ECG_Internal_Flash_Write_Program_Data((uint32_t*)data_block->data, write_size);
#endif

    blocks_received++;
        
    response.status = FIRMWARE_DOWNLOAD_CMD_OK;
#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
    USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
    bytes_received += write_size;

} /* ECG_Firmware_Download_Receive_Data() */


/* Public - Performs the final actions necessary for storing the new
 * firmware on the external flash.  Sends appropriate response to the
 * PC Application.
 * 
 * Examples
 * 
 *      ECG_Firmware_Download_Finalize();
 * 
 * Returns nothing
 */
void ECG_Firmware_Download_Finalize(void)
{
    CardeaCmdUpdateFirmwareResponse_t response;
    uint32_t program_data_crc;
    uint8_t external_voltage_present = 0;
    
    response.cmd = CARDEA_CMD_UPDATE_FIRMWARE;

    external_voltage_present = ECG_Power_Get_External_Connection_State();
    if(external_voltage_present != 1)
    {
        response.status = FIRMWARE_DOWNLOAD_CMD_NO_EXT;
#ifdef ECG_HARDWARE_BLUETOOTH
        BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
        USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
        ECG_Firmware_Download_Init();
        return;
    }

    if(state != Firmware_Download_State_Receive_Data)
    {
        response.status = FIRMWARE_DOWNLOAD_CMD_ERROR;
#ifdef ECG_HARDWARE_BLUETOOTH
        BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
        USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
        ECG_Firmware_Download_Init();
        Display(("Wrong State!\r\n"));
        return;
    }
    
    if(blocks_received != blocks_expected)
    {
        response.status = FIRMWARE_DOWNLOAD_CMD_ERROR;
#ifdef ECG_HARDWARE_BLUETOOTH
        BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
        USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
        ECG_Firmware_Download_Init();
        Display(("Not enough blocks received.\r\n"));
        return;
    }
    
    /* Now that we have all of the data, write the size of the application to the flash
     * so the boot loader will know that we need to update the flash on a reset */
#ifdef ECG_EXTERNAL_FLASH
    program_data_crc = ECG_Flash_Write_Get_Program_Data_CRC();
    ECG_Flash_Set_Program_Info(app_size, program_data_crc);
#else
	program_data_crc = ECG_Internal_Flash_Write_Get_Program_Data_CRC();
	ECG_Internal_Flash_Write_Program_Info(app_size, program_data_crc);
#endif

    response.status = FIRMWARE_DOWNLOAD_CMD_OK;
#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#elif ECG_HARDWARE_USB
    USB_Send_Command_Response(&response, sizeof(CardeaCmdUpdateFirmwareResponse_t));
#endif
    
    state = Firmware_Download_State_Finish;
    
    g_iResetAfterUpdate = 1;
} /* ECG_Firmware_Download_Finalize() */


/* Public - Processes firmware download messages based on the current
 * state of the download.
 * 
 * Examples
 * 
 *      uint32_t size;
 *      ECG_Firmware_Download_Proc(&size);
 * 
 * Returns nothing
 */
void ECG_Firmware_Download_Proc(void *data)
{
    /* Call appropriate function for each state */
    switch(state)
    {
        case Firmware_Download_State_Idle:
            ECG_Firmware_Download_Set_Size(*((uint32_t*)data));
            break;
        case Firmware_Download_State_Received_Size:
        case Firmware_Download_State_Receive_Data:
            ECG_Firmware_Download_Receive_Data((CardeaFirmwareUpdateBlock_t*)data);
            break;
        case Firmware_Download_State_Finish:
            break;
    }
} /* ECG_Firmware_Download_Proc() */


/* Public - Gets the current state of the firmware download.
 * 
 * Examples
 * 
 *      ECG_Firmware_Download_State_t8 dl_state;
 * 
 *      dl_state = ECG_Firmware_Download_GetState();
 * 
 * Returns nothing
 */
ECG_Firmware_Download_State_t8 ECG_Firmware_Download_GetState(void)
{
    return state;
} /* ECG_Firmware_Download_GetState() */
