/**************************************************************************************************************************************************
*       ADS1x9x_Device_Status.c  -  Status handling for ADS1x9x devices.  All initialization calls are initiated from main.c                                      *
*                                                                                                                                                 *
*       Author:             Luke Duncan                                                                                                         *
*                                                                                                                                                 *
*       Revision Date:      June 2009                                                                                                             *
*                                                                                                                                                 *
*       Revision Level:     1st pass                                                                                                              *
*                                                                                                                                                 *
*       For Support:        https://e2e.ti.com/support/development_tools/mavrk/default.aspx                                                       *
*                                                                                                                                                 *
***************************************************************************************************************************************************
*       Copyright © 2009-2010 Texas Instruments Incorporated - http://www.ti.com/                                                                 *
***************************************************************************************************************************************************
*  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met: *
*                                                                                                                                                 *
*    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.                 *
*                                                                                                                                                 *
*    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the        *
*    documentation and/or other materials provided with the distribution.                                                                         *
*                                                                                                                                                 *
*    Neither the name of Texas Instruments Incorporated nor the names of its contributors may be used to endorse or promote products derived      *
*    from this software without specific prior written permission.                                                                                *
*                                                                                                                                                 *
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          *
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT     *
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT         *
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    *
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE      *
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                                           *
***************************************************************************************************************************************************
*                                 MODULE CHANGE LOG                                                                                               *
*                                                                                                                                                 *
*       Date Changed:                             Developer:                                                            *
*       Change Description:                                                            *
*                                                                                                                                                 *
**************************************************************************************************************************************************/
/**************************************************************************************************************************************************
*                                 Included Headers                                                                                                *
**************************************************************************************************************************************************/
#include "sapphire_pub.h"

#include "ADS1x9x.h"
//#include "BKRNLAPI.h"

#include "ecg_ads.h"
#include "ssi.h"

/**************************************************************************************************************************************************
*                                 Prototypes                                                                                                      *
**************************************************************************************************************************************************/

/**************************************************************************************************************************************************
*                                 Global Variables                                                                                                *
**************************************************************************************************************************************************/


/**************************************************************************************************************************************************
*                                 External Variables                                                                                              *
**************************************************************************************************************************************************/
extern unsigned char ADS1x9x_SPI_data;
static const uint8_t reg_bit_mask[] =
{
    0xff,  //0x00
    0xff,  //0x01
    0xff,  //0x02
    0xff,  //0x03
    0xff,  //0x04
    0xff,  //0x05
    0xff,  //0x06
    0xff,  //0x07
    0xff,  //0x08
    0xff,  //0x09
    0xff,  //0x0a
    0xff,  //0x0b
    0xff,  //0x0c
    0xff,  //0x0d
    0xff,  //0x0e
    0xff,  //0x0f
    0xff,  //0x10
    0xff,  //0x11
    0xff,  //0x12
    0x0f,  //0x13
    0xff,  //0x14
    0xff,  //0x15
    0xff,  //0x16
    0xff,  //0x17
    0xff   //0x18
};


/**********************************************************************************************************
*               ADS1x9x Status Registers                                                                  *
**********************************************************************************************************/
unsigned char ADS1x9x_Read_Version (void)  
{
    ADS1x9x_SPI_Address_Byte_Count(READ_DEVICE_ID, SINGLE_BYTE_READ_WRITE);
    return (uint8_t)ADS1x9x_SPI_Data(SPI_TEST_DATA);
}  

/**********************************************************************************************************
* Verify the ADS1x9x using a Constants Table of known good settings                        *
**********************************************************************************************************/
unsigned char verify_ADS1x9x_Registers (unsigned char* constant_pointer)
{   
    uint8_t i;
    volatile uint8_t j;
    uint8_t read_data[ADS1x9x_TOP_REGISTER_SIZE + ADS1x9x_BOTTOM_REGISTER_SIZE + 2];
    uint8_t tx_data[ADS1x9x_TOP_REGISTER_SIZE + ADS1x9x_BOTTOM_REGISTER_SIZE + 2];
    unsigned char error = ADS_1x9x_INIT_SUCCESS;                                

    BTPS_MemInitialize(read_data, 0x00, ADS1x9x_TOP_REGISTER_SIZE + ADS1x9x_BOTTOM_REGISTER_SIZE + 2);
    BTPS_MemInitialize(tx_data, 0x00, ADS1x9x_TOP_REGISTER_SIZE + ADS1x9x_BOTTOM_REGISTER_SIZE + 2);
  
    ADS1x9x_SPI_Address_Byte_Count (DEFAULT_READ_NUMBER_OF_REGISTERS, ADS1x9x_TOP_REGISTER_SIZE);
//    ADS1x9x_SPI_Address_Byte_Count (DEFAULT_READ_NUMBER_OF_REGISTERS, ADS1x9x_TOP_REGISTER_SIZE + ADS1x9x_BOTTOM_REGISTER_SIZE + 2);

    for (i = 0; i < ADS1x9x_TOP_REGISTER_SIZE; i++)                            
    {   
        read_data[i] = ADS1x9x_SPI_Burst (SPI_TEST_DATA);
    }
    for(j = 0; j < 17; j++);
    
    Clear_ADS1x9x_Chip_Enable();

    ADS1x9x_SPI_Address_Byte_Count ((DEFAULT_READ_NUMBER_OF_REGISTERS + ADS1x9x_TOP_REGISTER_SIZE + 2), ADS1x9x_BOTTOM_REGISTER_SIZE);
    
    for (i = ADS1x9x_TOP_REGISTER_SIZE + 2; i < ADS1x9x_BOTTOM_REGISTER_SIZE + ADS1x9x_TOP_REGISTER_SIZE + 2; i++)
    {                              
        read_data[i] = ADS1x9x_SPI_Burst (SPI_TEST_DATA);
    }
    for(j = 0; j < 17; j++);
    Clear_ADS1x9x_Chip_Enable();

    /* Verify data */
    for(i = 0; i < ADS1x9x_TOP_REGISTER_SIZE + ADS1x9x_BOTTOM_REGISTER_SIZE + 2; i++)
    {
        if ((read_data[i] & reg_bit_mask[i]) != constant_pointer [i])
        {
            Display(("Register Verification Error! %d %x\r\n", i, read_data[i]));
            error = ADS_1x9x_VERIFY_ERROR;                                      
        }
    }
    return error;
}
