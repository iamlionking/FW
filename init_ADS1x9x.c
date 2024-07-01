/**************************************************************************************************************************************************
*       init_ADS1x9x.c  -  Set's up an ADS1x9x device.  All initialization calls are initiated from main.c                                      *
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
*       Date Changed:                            Developer:                                                                                       *
*       Change Description:                                                                                                                       *
*                                                                                                                                                 *
**************************************************************************************************************************************************/
/**************************************************************************************************************************************************
*                                 Included Headers                                                                                                *
**************************************************************************************************************************************************/
#include <string.h>
#include "sapphire_pub.h"

#include "stdef.h"
#include "ADS1x9x.h"
#include "gpio.h"
#include "ecg_ads.h"
#include "ecg_gpio.h"
#include "ssi.h"
#include "ecg_ssi.h"

#include "BKRNLAPI.h"

/**************************************************************************************************************************************************
*                                 Global Variables                                                                                                *
**************************************************************************************************************************************************/
unsigned char ADS1x9x_SPI_data;

// volatile register_name_type *register_name = (volatile register_name_type*)0xADD;
volatile unsigned char ADS1x9x_Config_1;
volatile ADS1x9x_Config_1_Register_type *ADS1x9x_Config_1_register = (volatile ADS1x9x_Config_1_Register_type*) &ADS1x9x_Config_1;
volatile unsigned char ADS1x9x_Config_2;
volatile ADS1x9x_Config_2_Register_type *ADS1x9x_Config_2_register = (volatile ADS1x9x_Config_2_Register_type*) &ADS1x9x_Config_2;
volatile unsigned char ADS1x9x_Config_3;
volatile ADS1x9x_Config_3_Register_type *ADS1x9x_Config_3_register = (volatile ADS1x9x_Config_3_Register_type*) &ADS1x9x_Config_3;
volatile unsigned char ADS1x9x_Config_4;
volatile ADS1x9x_Config_4_Register_type *ADS1x9x_Config_4_register = (volatile ADS1x9x_Config_4_Register_type*) &ADS1x9x_Config_4;

unsigned char ADS1x9x_Default_Register_Settings[] = 
{
  0x04, // Config1 (Low power mode and 1K SPS)
  0x03, // Config2
  0xEC, // Config3
  0xE7, // loff
  0x50, // ch1set  
  0x50, // ch2set  
  0x50, // ch3set  
  0x50, // ch4set  
  0x50, // ch5set  
  0x50, // ch6set  
  0x50, // ch7set  
  0x50, // ch8set  
  0x03, // rld_sensp
  0x03, // rld_sensn
  0xFF, // loff_sensp
  0x01, // loff_sensn
  0x00, // loff_flip                0x10
  0x00, // loff_statp (Read Only!)  0x11
  0x00, // loff_statn (Read Only!)  0x12
  0x01, // gpio                     0x13
  0x01, // pace                     0x14
  0x00, // resp                     0x15
  0x02,  // config4                  0x16
  0x09,
  0xC2
};

/*
unsigned char ADS1x9x_Test_Register_Settings[] = 
{
  0x04, // Config1 (Low power mode and 1K SPS)
  0x03, // Config2
  0xEC, // Config3
  0xE7, // loff
  0x65, // ch1set  (configured for test signal)
  0x65, // ch2set  (configured for test signal)
  0x65, // ch3set  (configured for test signal)
  0x65, // ch4set  (configured for test signal)
  0x65, // ch5set  (configured for test signal)
  0x65, // ch6set  (configured for test signal)
  0x65, // ch7set  (configured for test signal)
  0x65, // ch8set  (configured for test signal)
  0x03, // rld_sensp
  0x03, // rld_sensm
  0xFF, // loff_sensp
  0x01, // loff_sensm
  0x00, // loff_flip        //0x10
  0x00, // loff_statp (Read Only!) //0x11
  0x00, // loff_statn (Read Only!) //0x12
  0x01, // gpio 0x13
  0x01, // pace 0x14
  0x00, // resp 0x15
  0x02, // config4 0x16
  0x09,
  0xC2
};
*/

unsigned char ADS1x9x_Test_Register_Settings[] = 
{
  0x04, // Config1 (Low power mode and 1K SPS)
  0x14, // Config2
  0xEC, // Config3
  0xE7, // loff
  0x55, // ch1set  (configured for test signal)
  0x55, // ch2set  (configured for test signal)
  0x55, // ch3set  (configured for test signal)
  0x55, // ch4set  (configured for test signal)
  0x55, // ch5set  (configured for test signal)
  0x55, // ch6set  (configured for test signal)
  0x55, // ch7set  (configured for test signal)
  0x55, // ch8set  (configured for test signal)
  0x03, // rld_sensp
  0x03, // rld_sensm
  0xFF, // loff_sensp
  0x01, // loff_sensm
  0x00, // loff_flip        //0x10
  0x00, // loff_statp (Read Only!) //0x11
  0x00, // loff_statn (Read Only!) //0x12
  0x01, // gpio 0x13
  0x01, // pace 0x14
  0x00, // resp 0x15
  0x02, // config4 0x16
  0x09,
  0xC2
};

const unsigned char *ADS1x9x_download_pointer = ADS1x9x_Default_Register_Settings;

volatile unsigned char ADS1x9x_Lead_Off_Control;
volatile ADS1x9x_Lead_Off_Control_Register_type *ADS1x9x_Lead_Off_Control_Register = (volatile ADS1x9x_Lead_Off_Control_Register_type*)&ADS1x9x_Lead_Off_Control;

volatile unsigned char ADS1x9x_Channel_Settings;
volatile ADS1x9x_Channel_Settings_Register_type *ADS1x9x_Channel_Settings_Register = (volatile ADS1x9x_Channel_Settings_Register_type*)&ADS1x9x_Channel_Settings;

volatile unsigned char ADS_1298_Channel_Stack [8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//unsigned char ADS1x9x_SPI_Settings [6] = {LOW_POLARITY, RETARDED_DATA, _8MHZ_MAX_CLOCK, AFE_ADS1x9x, 0x00, SLOT_NUMBER_1};

volatile unsigned char* ADS1x9x_SPI_TX_BUFFER [4];
volatile unsigned char* ADS1x9x_SPI_RX_BUFFER [4];
//volatile USCI_Interrupt_Flag_Status_type* ADS1x9x_SPI_Interrupt_Flags[4];

volatile uint8_t ADS1x9x_Version_ID_Number;
uint8_t ECG_Num_Channels = 8;                                 // Number of ECG channels to use
static uint8_t ads_init = 0;
static uint8_t ads_test_mode_state = 0;

/**************************************************************************************************************************************************
*                                 External Variables                                                                                              *
**************************************************************************************************************************************************/
extern volatile unsigned char Print_UART_Message_Busy_Flag;

/**************************************************************************************************************************************************
*    Initialize the ADS1x9x device                                                                                                                *
**************************************************************************************************************************************************/

void setup_ADS1x9xSPI(void)
{
  ads_init = 0;
} 

unsigned char init_ADS1x9x_done(void)
{
    return ads_init;
}

void pwrp_ADS1x9x(void)
{
    setup_ADS1x9xSPI();
}

void ADS1x9x_Enable_Test_Mode(uint8_t state)
{
    ads_test_mode_state = state;
}

unsigned char init_ADS1x9x (void)
{
    unsigned char Verify_Check = CLEAR;
//	uint8_t timer = 0;
//    unsigned char Module_Present = CLEAR;
//    unsigned char number_of_retries = 10;
//    unsigned char i;

	Stop_Read_Data_Continuous();
	
	Soft_Reset_ADS1x9x();
	
	Stop_Read_Data_Continuous();
	
	ADS1x9x_Version_ID_Number = ADS1x9x_Read_Version ();

	if(ads_test_mode_state > 0)
		ADS1x9x_download_pointer = ADS1x9x_Test_Register_Settings;
	else
		ADS1x9x_download_pointer = ADS1x9x_Default_Register_Settings;
		
	init_ADS1x9x_Via_Constant_Table ((unsigned char*) ADS1x9x_download_pointer);

	Verify_Check = verify_ADS1x9x_Registers ((unsigned char*) ADS1x9x_download_pointer);

    if(ads_test_mode_state > 0)
    {
        /* Set into test mode */
/*
        Enable_ADS1x9x_Test_Mode(TEST_SIGNALS_ARE_DRIVEN_INTERNALLY, DEFAULT_PLUS_MINUS_1_MV_TIMES_VREF_DIVIDED_BY_2_4, 
            PULSED_AT_CLOCK_FREQUENCY_DIVIDED_BY_2_TO_THE_20TH);
*/
        Enable_ADS1x9x_Test_Mode(TEST_SIGNALS_ARE_DRIVEN_INTERNALLY, PLUS_MINUS_2_MV_TIMES_VREF_DIVIDED_BY_2_4, 
	        DEFAULT_PULSED_AT_CLOCK_FREQUENCY_DIVIDED_BY_2_TO_THE_21ST);
    }
    
    //Verify_Check = Initialize_ADS1x9x_Data_Rate (MODULATION_FREQUENCY_DIVIDED_BY_512);    
    
    //Verify_Check = Initialize_ADS1x9x_Mode (HIGH_RESOLUTION_MODE);                      
     
    //for (i = 0; i < ECG_Num_Channels; i++)
    //{
    //    Verify_Check = Initialize_ADS1x9x_Channel                               //  Context Save will store the previous setting of the channel 
    //    (
    //        
    //        i + 1,                                                              // references channels 1 - 8
    //        DEFAULT_ADS1x9x_ELECTRODE_INPUT,                                    // DEFAULT_ADS1x9x_ELECTRODE_INPUT, ADS1x9x_INPUT_SHORTED, ADS1x9x_RIGHT_LEG_DETECT, ADS1x9x_ONE_HALF_DIGITAL_SUPPLY
    //                                                                            // ADS1x9x_TEMPERATURE_SENSOR, ADS1x9x_CALIBRATION_SIGNAL, ADS1x9x_RIGHT_LEG_DETECT_POSITIVE, ADS1x9x_RIGHT_LEG_DETECT_NEGATIVE
    //        DEFAULT_GAIN_OF_6,                                                  // DEFAULT_GAIN_OF_6, GAIN_OF_1, GAIN_OF_2, GAIN_OF_3, GAIN_OF_4, GAIN_OF_8, GAIN_OF_12
    //        DEFAULT_DISABLE_POWER_DOWN,                                         // DEFAULT_DISABLE_POWER_DOWN, ENABLE_POWER_DOWN
    //        IGNORE_PREVIOUS_STATE                                               // CONTEXT_SAVE_CHANNEL, IGNORE_PREVIOUS_STATE
    //    ); 
    //    
    //    if (Verify_Check == ADS_1x9x_VERIFY_ERROR)
    //    {
    //        break;                                                              // exit loop and report verify error
    //    }
    //}
    
    ads_init = 1;
    
    return Verify_Check;
}

/**********************************************************************************************************
*               ADS1x9x Data Rate                                                                         *
**********************************************************************************************************/
unsigned char Initialize_ADS1x9x_Data_Rate (unsigned char Modulation_Frequency_Divider)
{
    unsigned char Verify_status = ADS_1x9x_INIT_SUCCESS;                                    // Error state set Clear

    // Load Values set in the ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (READ_CONFIG_1_REGISTER, SINGLE_BYTE_READ_WRITE);        // Read Device ID, Single Byte the Part Number
    ADS1x9x_Config_1 = ADS1x9x_SPI_Data (SPI_TEST_DATA);                                    // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)

    //  Modify Values for the ADS1x9x
    *ADS1x9x_Config_1_register &= ~(CONFIG_1_OUTPUT_DATA_RATE_MASK);
    *ADS1x9x_Config_1_register |= (Modulation_Frequency_Divider << CONFIG_1_OUTPUT_DATA_RATE);

    // Program Values into ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (WRITE_CONFIG_1_REGISTER, SINGLE_BYTE_READ_WRITE);      // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (ADS1x9x_Config_1);                                // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)
  
  
#ifdef VERIFY
    //  Read Back Register    
    ADS1x9x_SPI_Address_Byte_Count (READ_CONFIG_1_REGISTER, SINGLE_BYTE_READ_WRITE);       // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (SPI_TEST_DATA);                                   // Read the Value from the SPI port   
    if (ADS1x9x_SPI_data != ADS1x9x_Config_1)
    {
        Verify_status = ADS_1x9x_VERIFY_ERROR;
    }        
//  -----------------------------------
#endif /* VERIFY */  
    return Verify_status;   
}
/*********************************************************************************************************/
/**********************************************************************************************************
*                ADS1x9x High_Performance LOW_POWER Mode                                                  *
**********************************************************************************************************/
unsigned char Initialize_ADS1x9x_Mode (unsigned char ADC_Power_Mode)
{
    unsigned char Verify_status = ADS_1x9x_INIT_SUCCESS;                                   // Error state set Clear
    
    // Load Values set in the ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (READ_CONFIG_1_REGISTER, SINGLE_BYTE_READ_WRITE);       // Read Device ID, Single Byte the Part Number
    ADS1x9x_Config_1 = ADS1x9x_SPI_Data (SPI_TEST_DATA);                                   // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)

    //  Modify Values for the ADS1x9x
    *ADS1x9x_Config_1_register &= ~(CONFIG_1_POWER_RES_OPTIMIZATION_MASK);
    *ADS1x9x_Config_1_register |= (ADC_Power_Mode << CONFIG_1_POWER_RES_OPTIMIZATION);             // Default_Low_Power_Mode, High_Resolution_Mode 

    // Program Values into ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (WRITE_CONFIG_1_REGISTER, SINGLE_BYTE_READ_WRITE);      // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (ADS1x9x_Config_1);                                // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)
  
  
#ifdef VERIFY
    //  Read Back Register    
    ADS1x9x_SPI_Address_Byte_Count (READ_CONFIG_1_REGISTER, SINGLE_BYTE_READ_WRITE);        // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (SPI_TEST_DATA);                                    // Read the Value from the SPI port   
    if (ADS1x9x_SPI_data != ADS1x9x_Config_1)
    {
        Verify_status = ADS_1x9x_VERIFY_ERROR;
    }        
//  -----------------------------------
#endif /* VERIFY */  
    return Verify_status;   
}
/*********************************************************************************************************/


/**********************************************************************************************************
*                ADS1x9x Control Register                                                                 *
**********************************************************************************************************/
unsigned char Initialize_ADS1x9x_Registers (void)
{
    unsigned char Verify_status = ADS_1x9x_INIT_SUCCESS;                                       // Error state set Clear
  
    // Load Values set in the ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (READ_CONFIG_1_REGISTER, SINGLE_BYTE_READ_WRITE);           // Read Device ID, Single Byte the Part Number
    ADS1x9x_Config_1 = ADS1x9x_SPI_Data (SPI_TEST_DATA);                                       // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)

    //  Modify Values for the ADS1x9x
    *ADS1x9x_Config_1_register &= ~(CONFIG_1_OSCILLATOR_CLOCK_OUTPUT_EN_MASK | CONFIG_1_READBACK_MODE_MASK);
    *ADS1x9x_Config_1_register |= ( (DEFAULT_DISABLE << CONFIG_1_OSCILLATOR_CLOCK_OUTPUT_EN) | (DEFAULT_DAISY_CHAIN_MODE << CONFIG_1_READBACK_MODE));                     
                                                                                               // DEFAULT_DISABLE, ENABLE
                                                                                               // DEFAULT_DAISY_CHAIN_MODE, MULTIPLE_READBACK_MODE

    // Program Values into ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (WRITE_CONFIG_1_REGISTER, SINGLE_BYTE_READ_WRITE);         // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (ADS1x9x_Config_1);                                   // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)
  
  
#ifdef VERIFY
    //  Read Back Register    
    ADS1x9x_SPI_Address_Byte_Count (READ_CONFIG_1_REGISTER, SINGLE_BYTE_READ_WRITE);          // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (SPI_TEST_DATA);                                      // Read the Value from the SPI port   
    if (ADS1x9x_SPI_data != ADS1x9x_Config_1)
    {
        Verify_status = ADS_1x9x_VERIFY_ERROR;
    }        
//  -----------------------------------
#endif /* VERIFY */  
    
// Load Values set in the ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (READ_CONFIG_3_REGISTER, SINGLE_BYTE_READ_WRITE);   // Read Device ID, Single Byte the Part Number
    ADS1x9x_Config_3 = ADS1x9x_SPI_Data (SPI_TEST_DATA);                               // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)

//  Modify Values for the ADS1x9x 
    *ADS1x9x_Config_3_register = 0x40;  //Bit 6 of this register is reserved and must be written to 1
    *ADS1x9x_Config_3_register |= (DEFAULT_RIGHT_LEG_DETECT_IS_CONNECTED << CONFIG_3_RIGHT_LEG_DETECT_LEAD_OFF_STATUS);
    *ADS1x9x_Config_3_register |= (DEFAULT_RIGHT_LED_DETECT_SENSE_DISABLE << CONFIG_3_RIGHT_LEG_DETECT_SENSE_EN);
    *ADS1x9x_Config_3_register |= (DEFAULT_RIGHT_LEG_DETECT_BUFFER_POWERED_DOWN << CONFIG_3_RIGHT_LEG_DETECT_BUFFER_POWER);
    *ADS1x9x_Config_3_register |= (DEFAULT_IS_FED_EXTERNALLY << CONFIG_3_RIGHT_LEG_DETECT_REFERENCE_SOURCE);
    *ADS1x9x_Config_3_register |= (DEFAULT_MEASUREMENT_SOURCE_IS_OPEN << CONFIG_3_RIGHT_LEG_DETECT_MEASUREMENT_SOURCE);
    *ADS1x9x_Config_3_register |= (DEFAULT_REF_VOLTAGE_IS_SET_TO_2_4_VOLTS << CONFIG_3_REFERENCE_VOLTAGE);
    *ADS1x9x_Config_3_register |= (DEFAULT_POWER_DOWN_INTERNAL_REFERENCE_BUFFER << CONFIG_3_POWER_DOWN_REFERENCE_BUFFER);
      
// Program Values into ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (WRITE_CONFIG_3_REGISTER, SINGLE_BYTE_READ_WRITE);  // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (ADS1x9x_Config_3);                            // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)
  
#ifdef VERIFY
    //  Read Back Register    
    ADS1x9x_SPI_Address_Byte_Count (READ_CONFIG_3_REGISTER, SINGLE_BYTE_READ_WRITE);   // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (SPI_TEST_DATA);                               // Read the Value from the SPI port   
    if (ADS1x9x_SPI_data != ADS1x9x_Config_3)
    {
        Verify_status = ADS_1x9x_VERIFY_ERROR;
    }        
//  -----------------------------------
#endif /* VERIFY */  
    
    // Load Values set in the ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (READ_LEAD_OFF_CONTROL_REGISTER, SINGLE_BYTE_READ_WRITE);   // Read Device ID, Single Byte the Part Number
    ADS1x9x_Lead_Off_Control = ADS1x9x_SPI_Data (SPI_TEST_DATA);                               // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)

//  Modify Values for the ADS1x9x
    *ADS1x9x_Lead_Off_Control_Register = 0x00;
    *ADS1x9x_Lead_Off_Control_Register |= (DEFAULT_LEAD_OFF_DETECTION_DISABLED << LEAD_OFF_FREQUENCY);  
    *ADS1x9x_Lead_Off_Control_Register |= (DEFAULT_12_5_NA << LEAD_OFF_CURRENT);  
    *ADS1x9x_Lead_Off_Control_Register |= (DEFAULT_CURRENT_MODE << LEAD_OFF_DETECTION_MODE);  
    *ADS1x9x_Lead_Off_Control_Register |= (DEFAULT_55_PERCENT << LEAD_OFF_COMPARATOR_THRESHOLD);    
      
// Program Values into ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (WRITE_LEAD_OFF_CONTROL_REGISTER, SINGLE_BYTE_READ_WRITE);  // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (ADS1x9x_Lead_Off_Control);                            // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)
  
#ifdef VERIFY
    //  Read Back Register    
    ADS1x9x_SPI_Address_Byte_Count (READ_LEAD_OFF_CONTROL_REGISTER, SINGLE_BYTE_READ_WRITE);   // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (SPI_TEST_DATA);                                       // Read the Value from the SPI port   
    if (ADS1x9x_SPI_data != ADS1x9x_Lead_Off_Control)
    {
        Verify_status = ADS_1x9x_VERIFY_ERROR;
    }        
//  -----------------------------------
#endif /* VERIFY */  
    
// Load Values set in the ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (READ_CONFIG_4_REGISTER, SINGLE_BYTE_READ_WRITE);       // Read Device ID, Single Byte the Part Number
    ADS1x9x_Config_4 = ADS1x9x_SPI_Data (SPI_TEST_DATA);                                   // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)

    *ADS1x9x_Config_4_register |= SET;                                  // Preview Device 
    
// Program Values into ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (WRITE_CONFIG_4_REGISTER, SINGLE_BYTE_READ_WRITE);      // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (ADS1x9x_Config_4);                                // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)
#ifdef VERIFY
    //  Read Back Register  
    ADS1x9x_SPI_Address_Byte_Count (READ_CONFIG_4_REGISTER, SINGLE_BYTE_READ_WRITE);       // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (SPI_TEST_DATA);                                   // Read the Value from the SPI port   
    if (ADS1x9x_SPI_data != ADS1x9x_Config_4)
    {
        Verify_status = ADS_1x9x_VERIFY_ERROR;
    }
#endif /* VERIFY */   
    return Verify_status;
}
/*********************************************************************************************************/

/**********************************************************************************************************
*                ADS1x9x Channel Initialization                                                           *
**********************************************************************************************************/
unsigned char Initialize_ADS1x9x_Channel 
( 
  unsigned char Channel_Number,
  unsigned char Input_Type,
  unsigned char Input_Gain,
  unsigned char Enable_Power_Setting,
  unsigned char Save_Previous_Setting
)
{
    unsigned char SPI_Read_Register = 0x00;
    unsigned char SPI_Write_Register = 0x00;
    unsigned char Verify_status = ADS_1x9x_INIT_SUCCESS;                        // Error state set Clear
    
    // Set SPI Address to Read and Write
    switch (Channel_Number)
    {
    	case 1:
    	    SPI_Read_Register = READ_CHANNEL_1_SET_REGISTER;
    	    SPI_Write_Register = WRITE_CHANNEL_1_SET_REGISTER;
    	    break;
    	case 2:
    	    SPI_Read_Register = READ_CHANNEL_2_SET_REGISTER;
    	    SPI_Write_Register = WRITE_CHANNEL_2_SET_REGISTER;
    	    break;
    	case 3:
    	    SPI_Read_Register = READ_CHANNEL_3_SET_REGISTER;
    	    SPI_Write_Register = WRITE_CHANNEL_3_SET_REGISTER;
    	    break;    	    
        case 4:
    	    SPI_Read_Register = READ_CHANNEL_4_SET_REGISTER;
    	    SPI_Write_Register = WRITE_CHANNEL_4_SET_REGISTER;
    	    break;    
        case 5:
    	    SPI_Read_Register = READ_CHANNEL_5_SET_REGISTER;
    	    SPI_Write_Register = WRITE_CHANNEL_5_SET_REGISTER;
    	    break;    
    	case 6:
    	    SPI_Read_Register = READ_CHANNEL_6_SET_REGISTER;
    	    SPI_Write_Register = WRITE_CHANNEL_6_SET_REGISTER;
    	    break;    
    	case 7:
    	    SPI_Read_Register = READ_CHANNEL_7_SET_REGISTER;
    	    SPI_Write_Register = WRITE_CHANNEL_7_SET_REGISTER;
    	    break;    
    	case 8:
    	    SPI_Read_Register = READ_CHANNEL_8_SET_REGISTER;
    	    SPI_Write_Register = WRITE_CHANNEL_8_SET_REGISTER;
    	    break; 
    	default:
    	    break;
    }
       
    // Load Values set in the ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (SPI_Read_Register, SINGLE_BYTE_READ_WRITE);    // Read Device ID, Single Byte the Part Number
    ADS1x9x_Channel_Settings = ADS1x9x_SPI_Data (SPI_TEST_DATA);                   // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)

    if (Save_Previous_Setting == CONTEXT_SAVE_CHANNEL)
    {
        ADS_1298_Channel_Stack [Channel_Number - 1] = ADS1x9x_Channel_Settings;             // Context Save Channel Setting
    }
    
    //  Modify Values for the ADS1x9x
    *ADS1x9x_Channel_Settings_Register = 0x00;
    *ADS1x9x_Channel_Settings_Register |= (Input_Type << CHANNEL_SETTINGS_INPUT_SELECTION);
    *ADS1x9x_Channel_Settings_Register |= (Input_Gain << CHANNEL_SETTINGS_PGA_GAIN);
    *ADS1x9x_Channel_Settings_Register |= (Enable_Power_Setting << CHANNEL_SETTINGS_POWER_DOWN);

    // Program Values into ADS1x9x
    ADS1x9x_SPI_Address_Byte_Count (SPI_Write_Register, SINGLE_BYTE_READ_WRITE);   // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (ADS1x9x_Channel_Settings);                // Send Dummy variable (0x55) to return the part number (Chip Select Cleared automatically)
  
  
#ifdef VERIFY
    //  Read Back Register    
    ADS1x9x_SPI_Address_Byte_Count (SPI_Read_Register, SINGLE_BYTE_READ_WRITE);    // Read Device ID, Single Byte the Part Number
    ADS1x9x_SPI_data = ADS1x9x_SPI_Data (SPI_TEST_DATA);                           // Read the Value from the SPI port   
    if (ADS1x9x_SPI_data != ADS1x9x_Channel_Settings)
    {
        Verify_status = ADS_1x9x_VERIFY_ERROR;
    }        
//  -----------------------------------
#endif /* VERIFY */    
    return Verify_status;
}  
/*********************************************************************************************************/
/**********************************************************************************************************
* Initialize the ADS1x9x using a Constants Table of known good settings                        *
**********************************************************************************************************/
void init_ADS1x9x_Via_Constant_Table (unsigned char* constant_pointer)
{
    volatile unsigned char i;
        
    ADS1x9x_SPI_Address_Byte_Count (DEFAULT_WRITE_NUMBER_OF_REGISTERS, ADS1x9x_TOP_REGISTER_SIZE);
    
    for (i = 0; i <= ADS1x9x_SPI_WRITE_DELAY; i++);                             
                                    
    for( i = 0; i < ADS1x9x_TOP_REGISTER_SIZE; i++)
    {
        ADS1x9x_SPI_Burst(constant_pointer[i]);
    }

    for(i = 0; i < 17; i++);
    Clear_ADS1x9x_Chip_Enable ();
    
    ADS1x9x_SPI_Address_Byte_Count ((DEFAULT_WRITE_NUMBER_OF_REGISTERS + ADS1x9x_TOP_REGISTER_SIZE + 2), ADS1x9x_BOTTOM_REGISTER_SIZE);
    for (i = 0; i <= ADS1x9x_SPI_WRITE_DELAY; i++);
    for(i = 0; i < ADS1x9x_BOTTOM_REGISTER_SIZE; i++)
    {
        ADS1x9x_SPI_Burst(constant_pointer[ADS1x9x_REGISTER_OFFSET + i]);
    }

    for(i = 0; i < 17; i++);
    Clear_ADS1x9x_Chip_Enable();
}
/*********************************************************************************************************/

