/**************************************************************************************************************************************************
*       ADS_SPI_Functions.c  -  SPI Communication for ADS Devices.  All initialization calls are initiated from main.c                                      *
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

#include "stdef.h"
#include "ADS1x9x.h"

#include "ssi.h"
#include "ecg_ads.h"
#include "BKRNLAPI.h"

/**************************************************************************************************************************************************
*                                 Prototypes                                                                                                      *
**************************************************************************************************************************************************/

/**************************************************************************************************************************************************
*                                 Global Variables                                                                                                *
**************************************************************************************************************************************************/

/**************************************************************************************************************************************************
*                                 External Variables                                                                                              *
**************************************************************************************************************************************************/


/**********************************************************************************************************
* ADS1298 Device SPI Interchip Communication                                                              *
*       This code is used to communicate to the ADS1298                                                   *
**********************************************************************************************************/
void ADS1x9x_SPI_Address_Byte_Count (unsigned char Read_Write_Address, unsigned char Number_Of_Bytes)
{ 
    uint32_t temp;
    
    if(SSIBusy(ADS_SSI_BUS))
        return;
    
    Set_ADS1x9x_Chip_Enable ();                                 // Set the Chip ENABLE to start the SPI transaction
    
    SSIDataPut(ADS_SSI_BUS, Read_Write_Address);
    while(SSIBusy(ADS_SSI_BUS));
    SSIDataGet(ADS_SSI_BUS, &temp);
    
    SSIDataPut(ADS_SSI_BUS, Number_Of_Bytes);
    while(SSIBusy(ADS_SSI_BUS));
    SSIDataGet(ADS_SSI_BUS, &temp);
    
} /* ADS1x9x_SPI_Address_Byte_Count() */


unsigned char ADS1x9x_SPI_Data (unsigned char Data)         // Complements the SPI_Address command 
{
    uint32_t read_data;
    volatile uint16_t i;
    
    SSIDataPut(ADS_SSI_BUS, Data);                          // Send the data sitting at the pointer Data to the TX Buffer

    while(SSIBusy(ADS_SSI_BUS));                           //wait till it transmits
    for(i = 0; i < 17; i++);
    
	Clear_ADS1x9x_Chip_Enable ();                           // Clear the Chip ENABLE to terminate the SPI transaction

    SSIDataGet(ADS_SSI_BUS, &read_data);
    
    return (uint8_t)read_data;                             // Return Status Data or Requested Read Data
} /* ADS1x9x_SPI_Data() */

    
unsigned char ADS1x9x_SPI_Burst (unsigned char Data)        // Complements the SPI_Address command 
{                                                           // But allows multiple transactions (no Clear Chip ENABLE)
    uint32_t read_data = 0;
    
    SSIDataPut(ADS_SSI_BUS, Data);                          // Send the data sitting at the pointer Data to the TX Buffer */

    while (SSIBusy(ADS_SSI_BUS));                           //wait till it transmits
    
	SSIDataGet(ADS_SSI_BUS, &read_data);
    return (uint8_t)read_data;                             // Return Status Data or Requested Read Data */
} /* ADS1x9x_SPI_Burst() */