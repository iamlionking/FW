#ifndef __SERIAL_NUM_H__
#define __SERIAL_NUM_H__

#define SERIAL_NUM_LEN              16
#define SECRET_CODE_LEN             16

#define SERIAL_NUMBER_ADDRESS 0xF8000

typedef struct Serial_Code_struct
{
    uint8_t serial_num[SERIAL_NUM_LEN];
    uint8_t secret_code[SECRET_CODE_LEN];
    
    /* This needs to stay at the end of the structure as the crc calculation occurs on everything but this field
     * See serial_num.c for code */
    uint32_t crc;
} Serial_Code_t;

uint8_t Write_Serial_Code(const Serial_Code_t * serial_code);
uint8_t Read_Serial_Code(Serial_Code_t * serial_code);

#if defined (__ECG_DEBUG__)
void Erase_Serial_Code();
#endif

#endif
