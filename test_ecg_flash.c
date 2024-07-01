#include "sapphire_pub.h"

#include "crc.h"

#ifdef ECG_EXTERNAL_FLASH
#include "ecg_flash.h"
#else
#include "ecg_internal_flash.h"
#endif

#include "ecg_test.h"

#define TEST_DATA_LARGE_SIZE                131000
#define TEST_DATA_LARGE_READ_CHUNK_SIZE     1024

static uint8_t test_data[20] = { 'T', 'h', 'i', 's', ' ', 's', 'o', 'm', 'e', ' ', 't', 'e', 's', 't', ' ', 'd', 'a', 't', 'a', '.' };
static uint8_t test_data_large[TEST_DATA_LARGE_READ_CHUNK_SIZE];
static uint8_t read_data_large[TEST_DATA_LARGE_READ_CHUNK_SIZE];

void Test_ECG_Flash()
{
    uint32_t i;
    uint32_t bytes_written;
    uint32_t test_size = 0;
    uint32_t test_crc = 0;
    uint8_t read_data[20];
    uint32_t current_program_size;
    uint32_t current_program_crc;
    uint32_t program_crc;
    uint32_t program_size;
    uint32_t read_size;
    
    uint8_t fail = 0;
    
#ifdef ECG_EXTERNAL_FLASH
    ECG_Flash_Init_Access();
    
    ECG_Flash_Get_Program_Info(&current_program_size, &current_program_crc);
#else
	ECG_Internal_Flash_Read_Program_Info(&current_program_size, &current_program_crc);
#endif

    Display(("Running ECG Flash Test...\r\n"));
    
    program_size = 20;
    program_crc = 500;
    
#ifdef ECG_EXTERNAL_FLASH
    ECG_Flash_Set_Program_Info(program_size, program_crc);
    ECG_Flash_Get_Program_Info(&test_size, &test_crc);
#else
	ECG_Internal_Flash_Erase();
	ECG_Internal_Flash_Write_Program_Info(program_size, program_crc);
	ECG_Internal_Flash_Read_Program_Info(&test_size, &test_crc);
#endif

    if(test_size != 20)
    {
        Display(("Writing of program size failed!\r\n"));
        Display(("Expected: %d\r\n", program_size));
        Display(("Actual: %d\r\n", test_size));
        Display(("Exiting Test!\r\n"));
        fail = 1;
        goto test_exit;
    }

    if(test_crc != program_crc)
    {
        Display(("Writing of program data crc failed!\r\n"));
        Display(("Expected: %d\r\n", program_crc));
        Display(("Actual: %d\r\n", test_crc));
        Display(("Exiting Test!\r\n"));
        fail = 1;
        goto test_exit;
    }

#ifdef ECG_EXTERNAL_FLASH 
    ECG_Flash_Start_Write_Program_Data();
    ECG_Flash_Write_Program_Data(test_data, program_size);
    program_crc = ECG_Flash_Write_Get_Program_Data_CRC();
    
    ECG_Flash_Start_Read_Program_Data();
    ECG_Flash_Read_Program_Data(read_data, program_size);
    test_crc = crc(read_data, program_size, ~0);
    ECG_Flash_Stop_Read_Program_Data();
#else
	ECG_Internal_Flash_Start_Write_Program_Data();
	ECG_Internal_Flash_Write_Program_Data((uint32_t*)test_data, program_size);
	program_crc = ECG_Internal_Flash_Write_Get_Program_Data_CRC();
	ECG_Internal_Flash_Start_Read_Program_Data();
	ECG_Internal_Flash_Read_Program_Data((uint32_t*)read_data, program_size);
	test_crc = crc(read_data, program_size, ~0);
	ECG_Internal_Flash_Stop_Read_Program_Data();
#endif 

    /* Verify Data read from device matches data written */
    for(i = 0; i < 20; i++)
    {
        if(read_data[i] != test_data[i])
        {
            Display(("Program Data Write failed at %d\r\n", i));
            Display(("Expected: %c\r\n", (char)test_data[i]));
            Display(("Actual: %c\r\n", (char)read_data[i]));
            fail = 1;
            goto test_exit;
        }
    }

	Display(("Test data good\r\n"));
    
    /* Verify program data crc */
    if(test_crc != program_crc)
    {
        Display(("CRC of data read does not match crc of data written!\r\n"));
        Display(("Expected: %d\r\n", program_crc));
        Display(("Actual: %d\r\n", test_crc));
        Display(("Exiting Test!\r\n"));
        fail = 1;
        goto test_exit;
    }
    
    /* Verify program data CRC */
    if(test_crc != program_crc)
    {
        Display(("Writing of program data crc failed!\r\n"));
        Display(("Expected: %d\r\n", program_crc));
        Display(("Actual: %d\r\n", test_crc));
        Display(("Exiting Test!\r\n"));
        fail = 1;
        goto test_exit;
    }

	Display(("Test data crc good\r\n"));
    
    /* Initialize Large Test Data Chunk */
    for(i = 0; i < TEST_DATA_LARGE_READ_CHUNK_SIZE; i++)
    {
        test_data_large[i] = (uint8_t)i;
    }
    
    Display(("Test Large Write\r\n"));
    
    program_size = TEST_DATA_LARGE_SIZE;
    program_crc = 2908;
   
#ifdef ECG_EXTERNAL_FLASH 
    ECG_Flash_Set_Program_Info(program_size, program_crc);
    ECG_Flash_Get_Program_Info(&test_size, &test_crc);
#else
	ECG_Internal_Flash_Erase();
	ECG_Internal_Flash_Write_Program_Info(program_size, program_crc);
	ECG_Internal_Flash_Read_Program_Info(&test_size, &test_crc);
#endif

    if(test_size != program_size)
    {
        Display(("Writing of program size failed!\r\n"));
        Display(("Expected: %d\r\n", program_size));
        Display(("Actual: %d\r\n", test_size));
        Display(("Exiting Test!\r\n"));
        fail = 1;
        goto test_exit;
    }
    
    
    bytes_written = 0;

#ifdef ECG_EXTERNAL_FLASH
    ECG_Flash_Start_Write_Program_Data();
#else
	ECG_Internal_Flash_Start_Write_Program_Data();
#endif

    do
    {
        if((TEST_DATA_LARGE_SIZE - bytes_written) > 256)
        {

#ifdef ECG_EXTERNAL_FLASH
            ECG_Flash_Write_Program_Data(test_data_large, 256);
#else
			ECG_Internal_Flash_Write_Program_Data((uint32_t*)test_data_large, 256);
#endif

            bytes_written += 256;
        }
        else
        {

#ifdef ECG_EXTERNAL_FLASH
            ECG_Flash_Write_Program_Data(test_data_large, TEST_DATA_LARGE_SIZE - bytes_written);
#else
			ECG_Internal_Flash_Write_Program_Data((uint32_t*)test_data_large, (TEST_DATA_LARGE_SIZE - bytes_written));
#endif

            bytes_written += (TEST_DATA_LARGE_SIZE - bytes_written);
        }
    } while(bytes_written != TEST_DATA_LARGE_SIZE);
    
#ifdef ECG_EXTERNAL_FLASH
    /* Get the CRC for the data written */
    program_crc = ECG_Flash_Write_Get_Program_Data_CRC();
    
    /* Verify large data block */
    ECG_Flash_Start_Read_Program_Data();
#else
    /* Get the CRC for the data written */
    program_crc = ECG_Internal_Flash_Write_Get_Program_Data_CRC();
    
    /* Verify large data block */
    ECG_Internal_Flash_Start_Read_Program_Data();
#endif

    read_size = TEST_DATA_LARGE_SIZE;
    test_crc = ~0;
    test_size = TEST_DATA_LARGE_READ_CHUNK_SIZE;
    do
    {
        if (read_size < TEST_DATA_LARGE_READ_CHUNK_SIZE)
            test_size = read_size;

#ifdef ECG_EXTERNAL_FLASH 
        ECG_Flash_Read_Program_Data(read_data_large, test_size);
#else
        ECG_Internal_Flash_Read_Program_Data((uint32_t*)read_data_large, test_size);
#endif
        
        test_crc = crc(read_data_large, test_size, test_crc);
        
        read_size -= test_size;
    } while(read_size != 0);

#ifdef ECG_EXTERNAL_FLASH
    ECG_Flash_Stop_Read_Program_Data();
#else
    ECG_Internal_Flash_Stop_Read_Program_Data();
#endif
    
    /*for(i = 0; i < 256; i++)
    {
        if(read_data_large[i] != test_data_large[i])
        {
            Display(("Program Data Write Large failed at %d\r\n", i));
            Display(("Expected: %c\r\n", (char)test_data_large[i]));
            Display(("Actual: %c\r\n", (char)read_data_large[i]));
            fail = 1;
            goto test_exit;
        }
    }*/
    
    /* Verify the crc of the program data */
    //test_crc = crc(read_data_large, TEST_DATA_LARGE_SIZE, ~0);
    if(test_crc != program_crc)
    {
        Display(("Writing of program data crc failed!\r\n"));
        Display(("Expected: %d\r\n", program_crc));
        Display(("Actual: %d\r\n", test_crc));
        Display(("Exiting Test!\r\n"));
        fail = 1;
        goto test_exit;
    }
    
test_exit:
    if(!fail)
        Display(("ECG Flash Test Passed!\r\n"));
        
#ifdef ECG_EXTERNAL_FLASH
    ECG_Flash_Set_Program_Info(0, 0);
#else
	ECG_Internal_Flash_Erase();
    //ECG_Internal_Flash_Write_Program_Info(0xFFFFFFFF, 0xFFFFFFFF);
#endif
} /* Test_ECG_Flash() */
