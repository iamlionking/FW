/**************************************************************************************************************************************************
*       ADS1x9x_Functions.c  -  Sets up the Functions for Operating the ADS1x9x.  All initialization calls are initiated from main.c                                      *
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

#include "stdef.h"                                // Common Expression Definitions
#include "ADS1x9x.h"

#include "ecg_gpio.h"
#include "gpio.h"
#include "BKRNLAPI.h"

/**************************************************************************************************************************************************
*                                 Prototypes                                                                                                      *
**************************************************************************************************************************************************/

/**************************************************************************************************************************************************
*                                 Global Variables                                                                                                *
**************************************************************************************************************************************************/

char ADC_Data_Ready;
static uint8_t ads_read = 0;

/**************************************************************************************************************************************************
*                                 External Variables                                                                                              *
**************************************************************************************************************************************************/
extern volatile unsigned char ADS1x9x_Config_2;
extern volatile ADS1x9x_Config_2_Register_type *ADS1x9x_Config_2_register;

extern unsigned char ADS1x9x_SPI_data;
extern char ADC_Data_Ready;

void set_ADS1x9x_Start_Pin(unsigned char state);

/**********************************************************************************************************
* Initialize ADS1x9x GPIO                                                                                 *
**********************************************************************************************************/
void enable_ADS1x9x_Conversion (void)
{    
    Soft_Start_ReStart_ADS1x9x();
    Start_Read_Data_Continuous ();
    
    //Hard_Start_ReStart_ADS1x9x ();
}

void Hard_Reset_ADS1x9x(void)
{
    volatile uint8_t i;
    //BTPS_Delay(50);
    GPIOPinWrite(GPIO_ADS_N_RESET_PORT, GPIO_ADS_N_RESET_PIN, ~GPIO_ADS_N_RESET_PIN);
    for(i = 0; i < 50; i++);
    GPIOPinWrite(GPIO_ADS_N_RESET_PORT, GPIO_ADS_N_RESET_PIN, GPIO_ADS_N_RESET_PIN);
    for(i = 0; i < 200; i++);
}

void Wake_Up_ADS1x9x (void)
{
    Set_ADS1x9x_Chip_Enable (); 
    ADS1x9x_SPI_Data (WAKE_CONVERTER_FROM_SLEEP);
}

void Put_ADS1x9x_In_Sleep (void)
{
    Set_ADS1x9x_Chip_Enable (); 
    ADS1x9x_SPI_Data (PLACE_CONVERTER_IN_SLEEP_MODE);
}

void Soft_Reset_ADS1x9x (void)
{
    volatile uint8_t i;
    
    Set_ADS1x9x_Chip_Enable (); 
    ADS1x9x_SPI_Data (RESET_CONVERTER);
    
    for(i = 0; i < 200; i++);
}

void Soft_Start_ReStart_ADS1x9x (void)
{
    Set_ADS1x9x_Chip_Enable();
    ADS1x9x_SPI_Data (START_RESTART_CONVERSION);
}

void Hard_Start_ReStart_ADS1x9x (void)
{
    set_ADS1x9x_Start_Pin(HIGH);
}

void Soft_Stop_ADS1x9x (void)
{    
    Set_ADS1x9x_Chip_Enable (); 
    ADS1x9x_SPI_Data (STOP_CONVERSION);
}

void Hard_Stop_ADS1x9x (void)
{
    set_ADS1x9x_Start_Pin(LOW);
}

void Stop_Read_Data_Continuous (void)
{
    Set_ADS1x9x_Chip_Enable (); 
    ADS1x9x_SPI_Data (STOP_READ_DATA_CONTINUOUSLY);
}

void Start_Read_Data_Continuous (void)
{
    Set_ADS1x9x_Chip_Enable (); 
    ADS1x9x_SPI_Data (SET_READ_DATA_CONTINUOUSLY);
}

void Set_ADS1x9x_Chip_Enable (void)
{
	GPIOPinWrite(GPIO_ADS_N_CS_PORT, GPIO_ADS_N_CS_PIN, ~GPIO_ADS_N_CS_PIN);
}
void Clear_ADS1x9x_Chip_Enable (void)
{
	GPIOPinWrite(GPIO_ADS_N_CS_PORT, GPIO_ADS_N_CS_PIN, GPIO_ADS_N_CS_PIN);
}

void set_ADS1x9x_Start_Pin(unsigned char state)
{
	if(state == LOW)
		GPIOPinWrite(GPIO_ADS_START_PORT, GPIO_ADS_START_PIN, ~GPIO_ADS_START_PIN);
	else
		GPIOPinWrite(GPIO_ADS_START_PORT, GPIO_ADS_START_PIN, GPIO_ADS_START_PIN);
}

void ADS1x9x_Toggle_Read()
{
    if(ads_read == 0)
        ads_read = 1;
    else
        ads_read = 0;
}

uint8_t ADS1x9x_Should_Read()
{
    return ads_read;
}

uint8_t ADS1x9x_Do_Read_Continuous()
{
    uint8_t data = 0xFE;
    uint8_t read_count = 27;
    
    
    GPIOPinWrite(GPIO_ADS_START_PORT, GPIO_ADS_START_PIN, ~GPIOPinRead(GPIO_ADS_START_PORT, GPIO_ADS_START_PIN));
    
    Set_ADS1x9x_Chip_Enable();
    
    Start_Read_Data_Continuous();
    
    /* Wait for data ready to go low */
    while(GPIOPinRead(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN));
    
    Display(("Started Data Read\r\n"));
    do
    {
        
        
        ADS1x9x_SPI_Burst(0x00);
    } while(--read_count > 0);
    
    Clear_ADS1x9x_Chip_Enable();
    
    Display(("Stopped Data Read\r\n"));
    Stop_Read_Data_Continuous();
    
    //ADS1x9x_Toggle_Read();
    
    return data;
}

/**********************************************************************************************************
*                            Enable the ADS1x9x Test Mode                                                       *
**********************************************************************************************************/
unsigned char Enable_ADS1x9x_Test_Mode (unsigned char Test_Signal_Source, unsigned char Test_Signal_Reference, unsigned char Test_Signal_Type)
{
    unsigned char Verify_status = ADS_1x9x_INIT_SUCCESS;                                      // Error state set Clear

    
// Load Values set in the ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (READ_CONFIG_2_REGISTER, SINGLE_BYTE_READ_WRITE);          // Read Device ID, Single Byte the Part Number
    ADS1x9x_Config_2 = ADS1x9x_SPI_Data (SPI_TEST_DATA);                                      // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)

//  Modify Values for the ADS1x9x
    *ADS1x9x_Config_2_register &= ~(CONFIG_2_TEST_SIGNAL_FREQUENCY_MASK | CONFIG_2_TEST_SIGNAL_AMPLITUDE_MASK | CONFIG_2_TEST_SIGNAL_SOURCE_MASK);
    *ADS1x9x_Config_2_register |= (Test_Signal_Type << CONFIG_2_TEST_SIGNAL_FREQUENCY);        // DEFAULT_PULSED_AT_CLOCK_FREQUENCY_DIVIDED_BY_2_TO_THE_21ST
                                                                                              // PULSED_AT_CLOCK_FREQUENCY_DIVIDED_BY_2_TO_THE_20TH
                                                                                              // AT_DC 
    *ADS1x9x_Config_2_register |= (Test_Signal_Reference << CONFIG_2_TEST_SIGNAL_AMPLITUDE);   // DEFAULT_PLUS_MINUS_1_MV_TIMES_VREF_DIVIDED_BY_2_4
                                                                                              // PLUS_MINUS_2_MV_TIMES_VREF_DIVIDED_BY_2_4
                                                                                              // PLUS_MINUS_10_MV_TIMES_VREF_DIVIDED_BY_2_4
                                                                                              // PLUS_MINUS_1V_TIMES_VREF_DIVIDED_BY_2_4 
    *ADS1x9x_Config_2_register |= (Test_Signal_Source << CONFIG_2_TEST_SIGNAL_SOURCE);          // DEFAULT_TEST_SIGNALS_ARE_DRIVEN_EXTERNALLY, TEST_SIGNALS_ARE_DRIVEN_INTERNALLY

// Program Values into ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (WRITE_CONFIG_2_REGISTER, SINGLE_BYTE_READ_WRITE);         // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (ADS1x9x_Config_2);                                   // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)

    return Verify_status;    
}


