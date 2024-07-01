#include "sapphire_pub.h"

#include "crc.h"
#include "flash.h"
#include "serial_num.h"

//#define SERIAL_NUMBER_ADDRESS 0x3e000

static void read_flash(int iLength, uint32_t *pucDest);
static void write_flash(int iLength, uint32_t *pucSrc);

/* Public - Writes the serial number and secret code to the internal flash.
 * 
 * Examples
 * 
 *      Serial_Code_t serial_code;
 *      
 *      Write_Serial_Code(&serial_code);
 * 
 * Returns 0 if the serial number already exists, 1 otherwise
 */
uint8_t Write_Serial_Code(const Serial_Code_t * serial_code)
{
    Serial_Code_t current;
    if(serial_code == 0)
        return 0;
    
    /* First read the current serial number */
    if(Read_Serial_Code(&current) == 1)
        /* Serial Number and Code already exist; return that the write failed*/
        return 0;

    write_flash(sizeof(Serial_Code_t), (uint32_t*)serial_code);
    
    Display(("Wrote new Serial Number: %s\r\n", serial_code->serial_num));
    Display(("With Secret Code: %s\r\n", serial_code->secret_code));
    Display(("With CRC: %x\r\n", serial_code->crc));
    return 1;
} /* Write_Serial_Num() */


/* Public - Reads the serial number and secret code from flash.
 * 
 * Examples
 * 
 *      Serial_Code_t serial_code;
 *      uint8_t result;
 * 
 *      result = Read_Serial_Code(&serial_code);
 *      
 * Returns 1 if successful and the serial number and secret code are valid, 0 otherwise
 */
uint8_t Read_Serial_Code(Serial_Code_t * serial_code)
{
    if(serial_code == 0)
        return 0;
        
    read_flash(sizeof(Serial_Code_t), (uint32_t*)serial_code);
    
    if(crc(serial_code, sizeof(Serial_Code_t), ~0) != 0)
    {
        /* CRC is not valid; return all 0's */
        BTPS_MemInitialize(serial_code->serial_num, 0x00, SERIAL_NUM_LEN);
        BTPS_SprintF((char*)serial_code->serial_num, "00000000");
        BTPS_MemInitialize(serial_code->secret_code, '0', SECRET_CODE_LEN);
        serial_code->crc = 0;
        Display(("CRC is not valid.  Using %s.\r\n", serial_code->serial_num));
        return 0;
    }
    
    return 1;
} /* Read_Serial_Num() */


#if defined (__ECG_DEBUG__)
void Erase_Serial_Code()
{
    Serial_Code_t serial_code;
    
    BTPS_MemInitialize(serial_code.secret_code, 0x00, SECRET_CODE_LEN);
    BTPS_MemInitialize(serial_code.serial_num, 0x00, SERIAL_NUM_LEN);
    serial_code.crc = 0xffff;
    
    write_flash(sizeof(serial_code), (uint32_t*)&serial_code);
} /* Erase_Serial_Code() */
#endif


/* Internal - Reads data from the internal flash.
 * 
 * Returns nothing
 */
static void read_flash(int iLength, uint32_t *pucDest)
{
BTPS_MemCopy(pucDest, (void *)SERIAL_NUMBER_ADDRESS, iLength);
} /* read_flash() */


/* Internal - Writes data to the internal flash.
 * 
 * Returns nothing.
 */
static void write_flash(int iLength, uint32_t *pucSrc)
{
    FlashErase(SERIAL_NUMBER_ADDRESS);
    
    //
    // Make sure iLength is multiple of 4
    //
    iLength = (iLength + 3) & ~0x3;

    //
    // Program the data into the flash
    //
    FlashProgram(pucSrc, SERIAL_NUMBER_ADDRESS, iLength);

} /* write_flash() */
