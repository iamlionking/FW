#include "sapphire_pub.h"

#include "crc.h"
#include "ecg_gpio.h"
#include "ecg_flash.h"
#include "ecg_ssi.h"
#include "gpio.h"
#include "ssi.h"

#define ECG_FLASH_ADDR_SIZE         3
#define ECG_PROGRAM_INFO_SIZE       12

#define CLEAR_CHIP_SELECT() GPIOPinWrite(GPIO_FLASH_CS_PORT, GPIO_FLASH_CS_PIN, GPIO_FLASH_CS_PIN);
#define SET_CHIP_SELECT() GPIOPinWrite(GPIO_FLASH_CS_PORT, GPIO_FLASH_CS_PIN, ~GPIO_FLASH_CS_PIN);

static uint32_t current_page;
static uint32_t prev_crc;

static void do_page_write(uint8_t *address, uint8_t *data, uint16_t data_len);
static uint8_t is_write_in_progress();
static void read_data(uint8_t *data, uint32_t len);
static void send_command(uint8_t command);
static void send_data(uint8_t *data, uint16_t data_len);
static void send_write_enable();


/* Public - Initializes the SSI bus to be able to access the external flash part.
 * 
 * Examples
 * 
 *      ECG_Flash_Init_Access();
 * 
 * Returns nothing
 */
void ECG_Flash_Init_Access(void)
{
    if(ECG_SSI_Get_Mode() != SSI_Flash_Mode)
        ECG_SSI_Set_Mode(SSI_Flash_Mode);
} /* ECG_Flash_Init_Access() */


/* Public - Retrieves the program information stored on the external flash.
 * 
 * size - A pointer to hold the size of the program.
 * data_crc - A pointer to hold the crc of the stored program data.
 * 
 * Examples
 * 
 *      uint32_t program_size = 0;
 *      uint32_t program_data_crc = 0;
 * 
 *      ECG_Flash_Get_Program_Info(&program_size, &program_data_crc);
 * 
 * Returns nothing
 */
void ECG_Flash_Get_Program_Info(uint32_t *size, uint32_t *data_crc)
{
    uint8_t address[ECG_FLASH_ADDR_SIZE] = { 0x00, 0x00, 0x00 };
    uint8_t data[ECG_PROGRAM_INFO_SIZE];
    uint32_t temp_size;
    uint32_t temp_data_crc;
    
    send_command(ECG_FLASH_READ_DATA_BYTES);
    send_data(address, ECG_FLASH_ADDR_SIZE);
    read_data(data, ECG_PROGRAM_INFO_SIZE);
    
    temp_size = (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
    temp_data_crc = (data[7] << 24) + (data[6] << 16) + (data[5] << 8) + data[4];
    
    CLEAR_CHIP_SELECT();
    
    *size = 0;
    *data_crc = 0;
    
    /* Verify CRC */
    if(crc(data, ECG_PROGRAM_INFO_SIZE, ~0) == 0)
    {
        /* CRC was valid, so update with true size and data_crc */
        *size = temp_size;
        *data_crc = temp_data_crc;
    }
} /* ECG_Flash_Get_Program_Size() */


/* Public - Reads a chunk of program data using the specified size.
 * 
 * data - A pointer to an array to hold the data being read.
 * len - The number of bytes to read.
 * 
 * Examples
 * 
 *      uint8_t data[1024];
 *       
 *      BTPS_MemInitialize(data, 0x00, 1024);
 *      ECG_Flash_Read_Program_Data(data, 1024);
 * 
 * Returns nothing
 */
void ECG_Flash_Read_Program_Data(uint8_t *data, uint32_t len)
{
    read_data(data, len);
} /* ECG_Flash_Read_Program_Data() */


/* Public - Writes the program's size of data crc to the external flash. The
 * information stored by this function will also be CRC'd internally to ensure
 * that when read from the external flash part it is valid.
 * 
 * size - The size to write to the flash.
 * data_crc - A CRC of the entierity of the program data to be written to the flash.
 * 
 * Examples
 * 
 *      uint32_t size = 131059
 *      uint32_t data_crc = 0xabf3459c;
 * 
 *      ECG_Flash_Set_Program_Info(size, data_crc);
 * 
 * Returns nothing
 */
void ECG_Flash_Set_Program_Info(uint32_t size, uint32_t data_crc)
{
    uint8_t address[ECG_FLASH_ADDR_SIZE] = { 0x00, 0x00, 0x00 };
    uint8_t size_data[ECG_PROGRAM_INFO_SIZE];
    uint32_t info_crc = 0;
    
    size_data[0] = (size & 0x000000ff);
    size_data[1] = (size & 0x0000ff00) >> 8;
    size_data[2] = (size & 0x00ff0000) >> 16;
    size_data[3] = (size & 0xff000000) >> 24;
    
    size_data[4] = (data_crc & 0x000000ff);
    size_data[5] = (data_crc & 0x0000ff00) >> 8;
    size_data[6] = (data_crc & 0x00ff0000) >> 16;
    size_data[7] = (data_crc & 0xff000000) >> 24;
    
    /* Calculate CRC */
    info_crc = crc(size_data, 8, ~0);
    
    size_data[8] = (info_crc & 0x000000ff);
    size_data[9] = (info_crc & 0x0000ff00) >> 8;
    size_data[10] = (info_crc & 0x00ff0000) >> 16;
    size_data[11] = (info_crc & 0xff000000) >> 24;
    
    do_page_write(address, size_data, ECG_PROGRAM_INFO_SIZE);
} /* ECG_Flash_Set_Program_Size() */


/* Public - Configures state variables to prepare for reading program data from 
 * the external flash part.
 * 
 * Examples
 * 
 *      ECG_Flash_Start_Read_Program_Data();
 * 
 * Returns nothing.
 */
void ECG_Flash_Start_Read_Program_Data(void)
{
    uint8_t address[ECG_FLASH_ADDR_SIZE] = { 0x00, 0x01, 0x00 };

    send_command(ECG_FLASH_READ_DATA_BYTES);
    send_data(address, 3);
} /* ECG_Flash_Start_Read_Program_Data() */


/* Public - Configures state variables to signify program data reading has 
 * finished.
 * 
 * Examples
 * 
 *      ECG_Flash_Start_Read_Program_Data();
 *      //Read data in a for loop
 *      ECG_Flash_Stop_Read_Program_Data();
 * 
 * Returns nothing.
 */
void ECG_Flash_Stop_Read_Program_Data(void)
{
    CLEAR_CHIP_SELECT();
} /* ECG_Flash_Stop_Read_Program_Data() */


/* Public - Configures state variables to signify that writing of program data
 * is about to start.
 * 
 * Examples
 * 
 *      ECG_Flash_Start_Write_Program_Data();
 * 
 * Returns nothing.
 */
void ECG_Flash_Start_Write_Program_Data(void)
{
    current_page = 0x01;
    prev_crc = ~0;
} /* ECG_Flash_Start_Write_Program_Data() */


/* Public - Configures state variables to signify that writing of program data
 * has finished.
 * 
 * Examples
 * 
 *      ECG_Flash_Start_Write_Program_Data();
 *      //Write data in a for loop
 *      ECG_Flash_Stop_Write_Program_Data();
 * 
 * Returns nothing.
 */
void ECG_Flash_Write_Program_Data(uint8_t *data, uint16_t len)
{
    uint8_t address[ECG_FLASH_ADDR_SIZE];
    
    address[0] = current_page / 0x100;
    address[1] = current_page % 0x100;
    address[2] = 0x00;
    
    do_page_write(address, data, len);
    current_page++;
    
    /* Update CRC each time write to the flash */
    prev_crc = crc(data, len, prev_crc);
} /* ECG_Flash_Write_Program_Data() */


/* Public - Retrieves the CRC calculated while the program data was
 * being written to the external flash.
 * 
 * Examples
 * 
 *      uint32_t data_crc;
 * 
 *      data_crc = ECG_Flash_Write_Get_Program_Data_CRC();
 * 
 * Returns the calculated data crc for the program data while it was written
 * to the external flash.  If no writing has occured, then the return
 * value is meaningless.
 */
uint32_t ECG_Flash_Write_Get_Program_Data_CRC()
{
    return prev_crc;
} /* ECG_Flash_Write_Get_Program_Data_CRC() */


/* Internal - Performs the actions necessary to write to a page on the external flash.
 * 
 * address - The starting address of the page write operation.
 * data - The data to write
 * data_len - The lenght of the data being written.
 * 
 * Returns nothing.
 */
static void do_page_write(uint8_t *address, uint8_t *data, uint16_t data_len)
{
    volatile uint8_t j;
    
    send_write_enable();
    
    send_command(ECG_FLASH_PAGE_WRITE);
    send_data(address, ECG_FLASH_ADDR_SIZE);
    send_data(data, data_len);

    while(SSIBusy(ECG_FLASH_SSI_BUS));
    CLEAR_CHIP_SELECT();

    /* Delay a bit */
    for(j = 0; j < 4; j++);
    
    send_command(ECG_FLASH_READ_STATUS_REG);
    while(is_write_in_progress() != 0);
    CLEAR_CHIP_SELECT();
} /* do_page_write() */


/* Internal - Reads the status of the external flash device.
 * 
 * Returns 1 if the device is busy, 0 otherwise.
 */
static uint8_t is_write_in_progress()
{
    uint32_t status;
    
    SSIDataPut(ECG_FLASH_SSI_BUS, 0x00);
    while(SSIBusy(ECG_FLASH_SSI_BUS));
    SSIDataGet(ECG_FLASH_SSI_BUS, &status);
    
    return (status & 0x01);
} /* is_write_in_progress() */


/* Internal - Reads data of the specified lenght from the device.
 * 
 * data - A pointer to an array to hold the data
 * len - The length of the data to read.
 * 
 * Returns nothing
 */
static void read_data(uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint32_t temp = 0x00000000;
    
    for(i = 0; i < len; i++)
    {
        SSIDataPut(ECG_FLASH_SSI_BUS, 0x00);
        while(SSIBusy(ECG_FLASH_SSI_BUS));
        SSIDataGet(ECG_FLASH_SSI_BUS, &temp);
        data[i] = (uint8_t)temp;
    }
} /* read_data() */


/* Internal - Sends a command to the external flash.
 * 
 * command - A valid command that will be exectued by the external flash.
 * 
 * Returns nothing
 */
static void send_command(uint8_t command)
{
    uint32_t temp;
    SET_CHIP_SELECT();
    
    SSIDataPut(ECG_FLASH_SSI_BUS, command);
    while(SSIBusy(ECG_FLASH_SSI_BUS));
    SSIDataGet(ECG_FLASH_SSI_BUS, &temp);
} /* send_command() */


/* Internal - Writes data to the external flash.
 * 
 * data - The data being written.
 * data_len - The length of the data.
 * 
 * Returns nothing.
 */
static void send_data(uint8_t *data, uint16_t data_len)
{
    uint16_t i;
    uint32_t temp;
    
    for(i = 0; i < data_len; i++)
    {
        SSIDataPut(ECG_FLASH_SSI_BUS, *(data+i));
        while(SSIBusy(ECG_FLASH_SSI_BUS));
        SSIDataGet(ECG_FLASH_SSI_BUS, &temp);
    }
} /* send_data() */


/* Internal - Sends the write enable command.  This is necessary before a write
 * operation can be performed.
 * 
 * Returns nothing.
 */
static void send_write_enable()
{
    send_command(ECG_FLASH_WRITE_ENABLE);
    CLEAR_CHIP_SELECT();
} /* send_write_enable() */
