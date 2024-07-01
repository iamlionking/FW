/**************************************************************************************************************************************************
*       ADS1x9x.h           Register File to set up system registers of the ADS1x9x                                                               *
*                                                                                                                                                 *
*       Author:             Mike Claassen                                                                                                         *
*                                                                                                                                                 *
*       Revision Date:      August, 2009                                                                                                          *
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
***************************************************************************************************************************************************/

/**************************************************************************************************************************************************
*               Union Structures and Definitions                                                                                                  *
**************************************************************************************************************************************************/
#ifndef SPI_TEST_DATA  
#define SPI_TEST_DATA   0x00                                           // Use 1010 1010 to SPI port for debug purposes  
#endif 

#define ADS1x9x_DEFAULT_GPIO_STATE                      0x07

enum Context_Save_Channel_Info_Type
{
    CONTEXT_SAVE_CHANNEL                                = 1,
    IGNORE_PREVIOUS_STATE                               = 0
};

enum ADS_1x9x_Status_Type
{
    ADS_1x9x_VERIFY_ERROR                               = 3,
    ADS_1x9x_INIT_SUCCESS                               = 2,
    ADS_1x9x_NOT_FOUND                                  = 1
};

#define NEW_ADS1x9x_DATA                                  5

#define ADS1x9x_DATA_SIZE                                 27 //3 bytes status, 3 bytes data * 8 channels

typedef uint8_t ADS1x9x_Status_Flags_type;

#define ADC_STATUS_DATA_READY1                          0x00
#define ADC_STATUS_NEW_DATA1                            0x01  
#define ADS_STATUS_1298_MODULE_STATUS                   0x02 //2 bits [3:2]
#define ADS_STATUS_RESERVED                             0x04 // bits [7:4]
 
#define ADS1x9x_PREAMBLE_GOOD                           0x0C

typedef uint8_t ADS1x9x_ID_Register_type;
#define ID_CHANNEL_NUMBER_ID                            0x00            // Channel Number Identification (DEV_ID[2:0])
#define ID_CHANNEL_NUMBER_ID_MASK                       0x07            // Channel Number ID Mask
#define ID_RESERVED_ALWAYS_LOW                          0x03            // Reserved : Always reads low (0)
#define ID_RESERVED_ALWAYS_LOW_MASK                     0x08            
#define ID_RESERVED_ALWAYS_HIGH                         0x04            // Reserved : Always reads high (1)
#define ID_RESERVED_ALWAYS_HIGH_MASK                    0x10
#define ID_REVISION_ID                                  0x05            // Device Family Identification (DEV_ID[7:5])
#define ID_REVISION_ID_MASK                             0xE0            // Device Family ID Mask
 
// REVISION_ID bit field definition
#define ADS1x9x_REV                                       0

typedef uint8_t ADS1x9x_Config_1_Register_type;
#define CONFIG_1_OUTPUT_DATA_RATE                       0x00            // Output Data Rate (DR[2:0])
#define CONFIG_1_OUTPUT_DATA_RATE_MASK                  0x07            // Output Data Rate Mask
#define CONFIG_1_RESERVED                               0x03            // Reserved ([4:3])
#define CONFIG_1_RESERVED_MASK                          0x18            
#define CONFIG_1_OSCILLATOR_CLOCK_OUTPUT_EN             0x05            // CLKOUT Connection (CLK_EN)
#define CONFIG_1_OSCILLATOR_CLOCK_OUTPUT_EN_MASK        0x20            // CLKOUT Connection Mask
#define CONFIG_1_READBACK_MODE                          0x06            // Daisy-chain/multiple readback mode (DAISY_EN')
#define CONFIG_1_READBACK_MODE_MASK                     0x40            // Readback Mode Mask
#define CONFIG_1_POWER_RES_OPTIMIZATION                 0x07            // Low-Power/High-Resolution mode (HR)  
#define CONFIG_1_POWER_RES_OPTIMIZATION_MASK            0x80            // Low-Power/High-Res Mask


// OUTPUT_DATA_RATE bit field definition
enum Output_Data_Rate_Type
{
    DEFAULT_MODULATION_FREQUENCY_DIVIDED_BY_16          = 0,  
    MODULATION_FREQUENCY_DIVIDED_BY_32                  = 1,
    MODULATION_FREQUENCY_DIVIDED_BY_64                  = 2, 
    MODULATION_FREQUENCY_DIVIDED_BY_128                 = 3,
    MODULATION_FREQUENCY_DIVIDED_BY_256                 = 4,
    MODULATION_FREQUENCY_DIVIDED_BY_512                 = 5,
    MODULATION_FREQUENCY_DIVIDED_BY_1024                = 6 
};

enum Readback_Mode_Type
{
    DEFAULT_DAISY_CHAIN_MODE                            = 0,  
    MULTIPLE_READBACK_MODE                              = 1
};

enum Power_Resolution_Optimization_Type
{
    DEFAULT_LOW_POWER_MODE                              = 0,  
    HIGH_RESOLUTION_MODE                                = 1
};

/* Config Register 2 */
typedef uint8_t ADS1x9x_Config_2_Register_type;
#define CONFIG_2_TEST_SIGNAL_FREQUENCY                  0x00            // Test Signal Frequency (TEST_FREQ[1:0])
#define CONFIG_2_TEST_SIGNAL_AMPLITUDE                  0x02            // Test Signal Amplitude (TEST_AMP)
#define CONFIG_2_RESERVED_1                             0x03            // Reserved - Needs to be written with 0
#define CONFIG_2_TEST_SIGNAL_SOURCE                     0x04            // Test Source (INT_TEST)
#define CONFIG_2_WCT_CHOPPING_SCHEME                    0x05            // WCT Chopping Scheme (WCT_CHOP)
#define CONFIG_2_RESERVED_2                             0x06            // Reserved - Needs to be written with 0's

/* Config Register 2 Bit Masks */ 
#define CONFIG_2_TEST_SIGNAL_FREQUENCY_MASK             0x03            // Test Signal Frequency Mask
#define CONFIG_2_TEST_SIGNAL_AMPLITUDE_MASK             0x04            // Test Signal Amplitude Mask
#define CONFIG_2_RESERVED_1_MASK                        0x08            // Reserved - Needs to be written with 0
#define CONFIG_2_TEST_SIGNAL_SOURCE_MASK                0x10            // Test Source (INT_TEST)
#define CONFIG_2_WCT_CHOPPING_SCHEME_MASK               0x20            // WCT Chopping Scheme (WCT_CHOP)
#define CONFIG_2_RESERVED_2_MASK                        0xC0            // Reserved - Needs to be written with 0's



// CONFIG_2_RESERVED bit field definition
#define CONFIG_2_RESERVED_VALUE                           7  

enum Wct_Chopping_Scheme
{
    DEFAULT_CHOPPING_FREQ_VARIES                        = 0,
    CHOPPING_FREQ_CONSTANT_AT_FMOD_DIV_16               = 1
};

enum Test_Source_Type
{
    DEFAULT_TEST_SIGNALS_ARE_DRIVEN_EXTERNALLY          = 0,  
    TEST_SIGNALS_ARE_DRIVEN_INTERNALLY                  = 1
}; 

enum Test_Signal_Amplitude_Type
{
    DEFAULT_PLUS_MINUS_1_MV_TIMES_VREF_DIVIDED_BY_2_4   = 0,  
    PLUS_MINUS_2_MV_TIMES_VREF_DIVIDED_BY_2_4           = 1, 
    PLUS_MINUS_10_MV_TIMES_VREF_DIVIDED_BY_2_4          = 2, 
    PLUS_MINUS_1V_TIMES_VREF_DIVIDED_BY_2_4             = 3 
};

enum Test_Signal_Frequency_Type
{
    DEFAULT_PULSED_AT_CLOCK_FREQUENCY_DIVIDED_BY_2_TO_THE_21ST   = 0,  
    PULSED_AT_CLOCK_FREQUENCY_DIVIDED_BY_2_TO_THE_20TH           = 1, 
    AT_DC                                                        = 3 
};

/* Config Register 3 */
typedef uint8_t ADS1x9x_Config_3_Register_type;
#define CONFIG_3_RIGHT_LEG_DETECT_LEAD_OFF_STATUS         0x00          // RLD Lead Off Status (RLD_STAT)
#define CONFIG_3_RIGHT_LEG_DETECT_SENSE_EN                0x01          // RLD Lead-Off Sense Function (RLD_LOFF_SENS)
#define CONFIG_3_RIGHT_LEG_DETECT_BUFFER_POWER            0x02          // RLD Buffer Power' (PD_RLD')
#define CONFIG_3_RIGHT_LEG_DETECT_REFERENCE_SOURCE        0x03          // RLD Reference Select (RLDREF)
#define CONFIG_3_RIGHT_LEG_DETECT_MEASUREMENT_SOURCE      0x04          // RLD Output Routed to ADC Mux (RLD_MEAS)
#define CONFIG_3_REFERENCE_VOLTAGE                        0x05          // Reference Voltage (VREF_4V)
#define CONFIG_3_RESERVED                                 0x06          // Reserved - Needs to be written with 1
#define CONFIG_3_POWER_DOWN_REFERENCE_BUFFER              0x07          // Reference Voltage (PD_REFBUR')

/* Config Register 3 Bit Masks */
#define CONFIG_3_RIGHT_LEG_DETECT_LEAD_OFF_STATUS_MASK    0x01          // RLD Lead Off Status (RLD_STAT)
#define CONFIG_3_RIGHT_LEG_DETECT_SENSE_EN_MASK           0x02          // RLD Lead-Off Sense Function (RLD_LOFF_SENS)
#define CONFIG_3_RIGHT_LEG_DETECT_BUFFER_POWER_MASK       0x04          // RLD Buffer Power' (PD_RLD')
#define CONFIG_3_RIGHT_LEG_DETECT_REFERENCE_SOURCE_MASK   0x08          // RLD Reference Select (RLDREF)
#define CONFIG_3_RIGHT_LEG_DETECT_MEASUREMENT_SOURCE_MASK 0x10          // RLD Output Routed to ADC Mux (RLD_MEAS)
#define CONFIG_3_REFERENCE_VOLTAGE_MASK                   0x20          // Reference Voltage (VREF_4V)
#define CONFIG_3_RESERVED_MASK                            0x40          // Reserved - Needs to be written with 1
#define CONFIG_3_POWER_DOWN_REFERENCE_BUFFER_MASK         0x80          // Reference Voltage (PD_REFBUR')

// CONFIG_3_RESERVED_1 bit field definition
#define CONFIG_3_RESERVED_1_VALUE                         1  

enum Power_Down_Reference_Buffer
{
    DEFAULT_POWER_DOWN_INTERNAL_REFERENCE_BUFFER        = 0,
    ENABLE_INTERNAL_REFERENCE_BUFFER                    = 1
};

enum Reference_Voltage
{
    DEFAULT_REF_VOLTAGE_IS_SET_TO_2_4_VOLTS             = 0,  
    REF_VOLTAGE_IS_SET_TO_4_VOLTS                       = 1 
};

enum Right_Leg_Detect_Measurement_Source
{
    DEFAULT_MEASUREMENT_SOURCE_IS_OPEN                  = 0,  
    MESASUREMENT_SOURCE_ROUTED_TO_MUX_SETTING_VREF      = 1
};

enum Right_Leg_Detect_Reference_Source
{
    DEFAULT_IS_FED_EXTERNALLY                           = 0,  
    IS_FED_INTERNALLY                                   = 1 
};

enum Right_Leg_Detect_Buffer_Power_Down
{
    DEFAULT_RIGHT_LEG_DETECT_BUFFER_POWERED_DOWN        = 0,
    RIGHT_LEG_DETECTH_BUFFER_POWERED_UP                 = 1
};

enum Right_Leg_Detect_Sense_Enable
{
    DEFAULT_RIGHT_LED_DETECT_SENSE_DISABLE              = 0,
    RIGHT_LEG_DETECT_SENSE_ENABLED                      = 1
};

enum Right_Leg_Detect_Lead_Off_Status
{
    DEFAULT_RIGHT_LEG_DETECT_IS_CONNECTED               = 0,
    RIGHT_LEG_DETECT_IS_NOT_CONNECTED                   = 1
};

typedef uint8_t ADS1x9x_Lead_Off_Control_Register_type;
#define LEAD_OFF_FREQUENCY                              0x00            // Frequency Lead (FLEAD_OFF[1:0])
#define LEAD_OFF_CURRENT                                0x02            // Lead off Current Magnitude (ILEAD_OFF[1:0])
#define LEAD_OFF_DETECTION_MODE                         0x04            // Lead off Detection Mode (VLEAD_OFF_EN)
#define LEAD_OFF_COMPARATOR_THRESHOLD                   0x05            // Lead off Comparator Threshold (COMP_TH[2:0])

#define LEAD_OFF_FREQUENCY_MASK                         0x03            // Frequency Lead (FLEAD_OFF[1:0])
#define LEAD_OFF_CURRENT_MASK                           0x0C            // Lead off Current Magnitude (ILEAD_OFF[1:0])
#define LEAD_OFF_DETECTION_MODE_MASK                    0x10            // Lead off Detection Mode (VLEAD_OFF_EN)
#define LEAD_OFF_COMPARATOR_THRESHOLD_MASK              0xE0            // Lead off Comparator Threshold (COMP_TH[2:0])

enum Lead_Off_Frequency_Type
{
    DEFAULT_LEAD_OFF_DETECTION_DISABLED                 = 0,  
    ONE_HALF_THE_OUTPUT_DATA_RATE                       = 1, 
    ONE_FOURTH_THE_OUTPUT_DATA_RATE                     = 2,
    DC_LEAD_OFF_DETECT                                  = 3
};

enum Lead_Off_Current_Type
{
    DEFAULT_12_5_NA                                     = 0,  
    _25_NA                                              = 1, 
    _35_5NA                                             = 2,
    _50NA                                               = 3
};

enum Lead_Off_Detection_Mode_Type
{
    DEFAULT_CURRENT_MODE                                = 0,  
    VOLTAGE_MODE                                        = 1 
};

enum Lead_Off_Comparator_Threshold_Type
{
    DEFAULT_55_PERCENT                                  = 0,  
    _60_PERCENT                                         = 1,
    _65_PERCENT                                         = 2, 
    _70_PERCENT                                         = 3, 
    _75_PERCENT                                         = 4, 
    _80_PERCENT                                         = 5, 
    _85_PERCENT                                         = 6, 
    _90_PERCENT                                         = 7
};

typedef uint8_t ADS1x9x_Channel_Settings_Register_type;
#define CHANNEL_SETTINGS_INPUT_SELECTION                0x00           // Channel Input (MUX[2:0])
#define CHANNEL_SETTINGS_RESERVED                       0x03           // Reserved (Must Write 0)
#define CHANNEL_SETTINGS_PGA_GAIN                       0x04           // PGA Gain (GAIN[2:0])
#define CHANNEL_SETTINGS_POWER_DOWN                     0x07           // Power-Down (PD) 

#define CHANNEL_SETTINGS_INPUT_SELECTION_MASK           0x07           // Channel Input (MUX[2:0])
#define CHANNEL_SETTINGS_RESERVED_MASK                  0x08           // Reserved (Must Write 0)
#define CHANNEL_SETTINGS_PGA_GAIN_MASK                  0x70           // PGA Gain (GAIN[2:0])
#define CHANNEL_SETTINGS_POWER_DOWN_MASK                0x80           // Power-Down (PD) 

enum Programmable_Gain_Setting_Type
{
    DEFAULT_GAIN_OF_6                                   = 0,  
    GAIN_OF_1                                           = 1, 
    GAIN_OF_2                                           = 2, 
    GAIN_OF_3                                           = 3,
    GAIN_OF_4                                           = 4, 
    GAIN_OF_8                                           = 5, 
    GAIN_OF_12                                          = 6 
};

enum Channel_Input_Is_Type
{
    DEFAULT_ADS1x9x_ELECTRODE_INPUT                     = 0,  
    ADS1x9x_INPUT_SHORTED                               = 1, 
    ADS1x9x_RIGHT_LEG_DETECT                            = 2, 
    ADS1x9x_MVDD_SUPPLY_MEASUREMENT                     = 3, 
    ADS1x9x_TEMPERATURE_SENSOR                          = 4,
    ADS1x9x_TEST_SIGNAL                                 = 5,
    ADS1x9x_RIGHT_LEG_DETECT_POSITIVE_DRIVER            = 6,
    ADS1x9x_RIGHT_LEG_DETECT_NEGATIVE_DRIVER            = 7
};

// Used in Test Routine
#define ADS1x9x_CHANNEL_IS_DISABLED                     0xFF

enum Power_Down_Channel_Type
{
    DEFAULT_DISABLE_POWER_DOWN                          = 0,
    ENABLE_POWER_DOWN                                   = 1
};

/* GPIO Settings Register */
typedef uint8_t ADS1x9x_GPIO_Settings_Register_type;
#define GPIO_DIRECTION                         			0x00           // Corresponding GPIOD (GPIOC[3:0])
#define GPIO_DATA										0x04           // GPIO Data (GPIOD[4:7])

#define GPIO_DIRECTION_MASK                   			0x0F           // Corresponding GPIOD (GPIOC[3:0])
#define GPIO_DATA_MASK		    						0xF0           // GPIO Data (GPIOD[4:7])

/* PACE Detect Settings Register */
typedef uint8_t  ADS1x9x_PACE_Detect_Settings_Register_type;
#define PACE_DETECT_BUFFER								0x00           	// PACE detect buffer (PD_PACE')
#define PACE_DETECT_ODD_CHANNEL_SELECT					0x01           	// PACE_OUT1 odd (PACEO[1:0])
#define PACE_DETECT_EVEN_CHANNEL_SELECT 				0x03           	// PACE_OUT2 even (PACEE[1:0])
#define PACE_DETECT_RESERVED							0x05			// Reserved Bits[7:5]

#define PACE_DETECT_BUFFER_MASK					    	0x01           	// PACE detect buffer (PD_PACE')
#define PACE_DETECT_ODD_CHANNEL_SELECT_MASK				0x06           	// PACE_OUT1 odd (PACEO[1:0])
#define PACE_DETECT_EVEN_CHANNEL_SELECT_MASK 			0x18           	// PACE_OUT2 even (PACEE[1:0])
#define PACE_DETECT_RESERVED_MASK						0xE0			// Reserved Bits[7:5]

// ODD_PACE_CHANNEL_SELECT & EVEN_PACE_CHANNEL_SELECT bit field definition
enum Pace_Channel_Select_Type
{
    DEFAULT_PACE_CHANNEL_IS_1                           = 0,  
    DEFAULT_PACE_CHANNEL_IS_2                           = 0, 
    PACE_CHANNEL_IS_3                                   = 1,  
    PACE_CHANNEL_IS_4                                   = 1, 
    PACE_CHANNEL_IS_5                                   = 2,  
    PACE_CHANNEL_IS_6                                   = 2, 
    PACE_CHANNEL_IS_7                                   = 3,  
    PACE_CHANNEL_IS_8                                   = 3 
};
/* The following register only applies to ADS1x9xR
typedef struct
{
    unsigned char Respiration_Clock_Source:               2;           // Respiration Control Mode (RESP_CTRL[1:0])
    unsigned char Respiration_Signal_Phase:               2;           // Respiration Phase (RESP_PH[2:0])
    unsigned char Respiration_Frequency:                  1;           // Respiration Frequency (RESP_FREQ)
    unsigned char Channel_1_Internal_Modulation:          1;           // Channel 1 internal modulation (RESP_MOD_EN1)
    unsigned char Channel_2_Internal_Modulation:          1;           // Channel 2 internal modulation (RESP_MOD_EN2)
} ADS1x9x_Respiration_Control_Register_type;*/

enum Respiration_Signal_Phase_Type
{
    DEFAULT_PHASE_IS_22_5_DEGREES                       = 0,  
    PHASE_IS_45_DEGREES                                 = 1,  
    PHASE_IS_67_5_DEGREES                               = 2,  
    PHASE_IS_90_DEGREES                                 = 3,  
    PHASE_IS_112_5_DEGREES                              = 4,  
    PHASE_IS_135_DEGREES                                = 5,  
    PHASE_IS_157_5_DEGREES                              = 6,
    PHASE_IS_180_DEGREES                                = 7
};

enum Respiration_Clock_Source_Type
{
    DEFAULT_RESPIRATION_MODE_DISABLED                        = 0,  
    GPIO_3_IS_RESP_CLK_OUT_AND_GPIO_4_IS_RESP_PHASE_CLK_OUT  = 1,  
    INTERNAL_RESPIRATION_USING_INTERNAL_CLOCK                = 2,  
    GPIO_3_IS_RESP_CLK_IN_AND_GPIO_4_IS_RESP_PHASE_CLK_IN    = 3
};  

/* Config 4 Register */
typedef unsigned char ADS1x9x_Config_4_Register_type;
#define CONFIG_4_RESERVED_1					            0x00           // Reserved (Must be One)
#define CONFIG_4_DISABLE_LEAD_OFF_COMPARATOR            0x01           // Lead Off Comparator Disable (Active Low)
#define CONFIG_4_WCT_TO_RLD_CONNECTED				    0x02           // Reserved (Must be Zero)
#define CONFIG_4_CONVERSION_MODE						0x03           // Pulse Mode (PULSE_MODE)
#define CONFIG_4_RESERVED_2							    0x04           // Reserved (Must be Zero)
#define CONFIG_4_RESPIRATION_FREQUENCY				    0x05		   // Respiration Frequency (Bit 7:5)

#define CONFIG_4_RESERVED_1_MASK					    0x01           // Reserved (Must be One)
#define CONFIG_4_DISABLE_LEAD_OFF_COMPARATOR_MASK       0x02           // Lead Off Comparator Disable (Active Low)
#define CONFIG_4_WCT_TO_RLD_CONNECTED_MASK			    0x04           // Reserved (Must be Zero)
#define CONFIG_4_CONVERSION_MODE_MASK					0x08           // Pulse Mode (PULSE_MODE)
#define CONFIG_4_RESERVED_2_MASK						0x10           // Reserved (Must be Zero)
#define CONFIG_4_RESPIRATION_FREQUENCY_MASK				0xE0		   // Respiration Frequency (Bit 7:5)

// CONFIG_2_RESERVED bit field definition
#define CONFIG_4_RESERVED_2_VALUE                         1  

//-----------------------------------------------------------------------------------------------------------------
enum ADS1x9x_Command_Values
{
// System Commands                                                     //  ADS1x9x Command Definitions
    WAKE_CONVERTER_FROM_SLEEP                           = 0x02,        //  WAKEUP
    PLACE_CONVERTER_IN_SLEEP_MODE                       = 0x04,        //  SLEEP
    RESET_CONVERTER                                     = 0x06,        //  RESET
    START_RESTART_CONVERSION                            = 0x08,        //  START
    STOP_CONVERSION                                     = 0x0A,        //  STOP

// Cal Commands 
    CALIBRATE_OFFSET_FOR_ALL_CHANNELS                   = 0x1A,        //  OFFCAL

// Data Read Commands 
    SET_READ_DATA_CONTINUOUSLY                          = 0x10,        //  RDATAC
    STOP_READ_DATA_CONTINUOUSLY                         = 0x11,        //  SDATAC
    READ_DATA_MANUALLY                                  = 0x12,        //  RDATA

//  Register Read Commands
    DEFAULT_READ_NUMBER_OF_REGISTERS                    = 0x21,
    DEFAULT_WRITE_NUMBER_OF_REGISTERS                   = 0x41
};

//  Specific Register Read Commands
enum Specific_Register_Read_Command_Type
{
//  Device Settings
    READ_DEVICE_ID                                      = 0x20,
// Glocal Settings Across Channels
    READ_CONFIG_1_REGISTER                              = 0x21,
    WRITE_CONFIG_1_REGISTER                             = 0x41,
    READ_CONFIG_2_REGISTER                              = 0x22,
    WRITE_CONFIG_2_REGISTER                             = 0x42,
    READ_CONFIG_3_REGISTER                              = 0x23,
    WRITE_CONFIG_3_REGISTER                             = 0x43,
    READ_CONFIG_4_REGISTER                              = 0x37,
    WRITE_CONFIG_4_REGISTER                             = 0x57,
    READ_LEAD_OFF_CONTROL_REGISTER                      = 0x24,
    WRITE_LEAD_OFF_CONTROL_REGISTER                     = 0x44,
// Channel Specific Settings
    READ_CHANNEL_1_SET_REGISTER                         = 0x25,
    WRITE_CHANNEL_1_SET_REGISTER                        = 0x45,
    READ_CHANNEL_2_SET_REGISTER                         = 0x26,
    WRITE_CHANNEL_2_SET_REGISTER                        = 0x46,
    READ_CHANNEL_3_SET_REGISTER                         = 0x27,
    WRITE_CHANNEL_3_SET_REGISTER                        = 0x47,
    READ_CHANNEL_4_SET_REGISTER                         = 0x28,
    WRITE_CHANNEL_4_SET_REGISTER                        = 0x48,
    READ_CHANNEL_5_SET_REGISTER                         = 0x29,
    WRITE_CHANNEL_5_SET_REGISTER                        = 0x49,
    READ_CHANNEL_6_SET_REGISTER                         = 0x2A,
    WRITE_CHANNEL_6_SET_REGISTER                        = 0x4A,
    READ_CHANNEL_7_SET_REGISTER                         = 0x2B,
    WRITE_CHANNEL_7_SET_REGISTER                        = 0x4B,
    READ_CHANNEL_8_SET_REGISTER                         = 0x2C,
    WRITE_CHANNEL_8_SET_REGISTER                        = 0x4C,

    READ_RIGHT_LEG_DRIVE_SENSE_POSITIVE_REGISTER        = 0x2D,
    WRITE_RIGHT_LEG_DRIVE_SENSE_POSITIVE_REGISTER       = 0x4D,
    READ_RIGHT_LEG_DRIVE_SENSE_NEGATIVE_REGISTER        = 0x2E,
    WRITE_RIGHT_LEG_DRIVE_SENSE_NEGATIVE_REGISTER       = 0x4E,

    READ_LEAD_OFF_SENSE_POSITIVE_REGISTER               = 0x2F,
    WRITE_LEAD_OFF_SENSE_POSITIVE_REGISTER              = 0x4F,
    READ_LEAD_OFF_SENSE_NEGATIVE_REGISTER               = 0x30,
    WRITE_LEAD_OFF_SENSE_NEGATIVE_REGISTER              = 0x50,

    READ_LEAD_OFF_FLIP_REGISTER                         = 0x31,
    WRITE_LEAD_OFF_FLIP_REGISTER                        = 0x51,
// Lead Off Status Registers
    READ_LEAD_OFF_STATUS_POSITIVE_REGISTER              = 0x32,
    READ_LEAD_OFF_STATUS_NEGATIVE_REGISTER              = 0x33,
// GPIO and OTHER Registers
    READ_GENERAL_PORT_IO                                = 0x34,
    WRITE_GENERAL_PORT_IO                               = 0x54,
    READ_PACE_DETECT_REGISTER                           = 0x35,
    WRITE_PACE_DETECT_REGISTER                          = 0x55,

    READ_RESPIRATION_CONTROL_REGISTER                   = 0x36,
    WRITE_RESPIRATION_CONTROL_REGISTER                  = 0x56,
    READ_CONFIGURATION_CONTROL_REGISTER                 = 0x37,
    WRITE_CONFIGURATION_CONTROL_REGISTER                = 0x57
};

#define SINGLE_BYTE_READ_WRITE                            0x00

#define ADS1x9x_SPI_WRITE_DELAY                           0x04

#define ADS1x9x_TOP_REGISTER_SIZE                         0x11  
#define ADS1x9x_REGISTER_OFFSET                           0x13
#define ADS1x9x_BOTTOM_REGISTER_SIZE                      0x06    


#define ADS_BUF_SIZE 27
#define PIN0  0x01
#define PIN1  0x02
#define PIN2  0x04
#define PIN3  0x08
#define PIN4  0x10
#define PIN5  0x20
#define PIN6  0x40
#define PIN7  0x80

#define SPI_CS_BASE 		    GPIO_PORTA_BASE
#define SPI_CS_PIN              GPIO_PIN_3
#define SPI_CS_SEL              P10SEL
#define SPI_CS_DIR              GPIO_DIR_MODE_OUT
#define SPI_CS_PULL_UP_ENABLE   

#define DRDY_BASE               GPIO_ADS_N_DRDY_PORT
#define DRDY_PIN                GPIO_ADS_N_DRDY_PIN
#define DRDY_DIRECTION          GPIO_ADS_N_DRDY_DIR
#define DRDY_PULL_UP_ENABLE     P2REN
#define DRDY_OUTPUT             P2OUT
#define DRDY_IN             	P2IN
#define DRDY_INTERRUPT_FLAG     P2IFG
#define DRDY_INTERRUPT_ENABLE   
#define DRDY_INT_EDGE           P2IES
#define DRDY_IV                 P2IV
#define DRDY_IFG                P2IV_P2IFG6
#define DRDY_PIN_VECTOR         PORT2_VECTOR

#define START_BASE              GPIO_ADS_START_PORT
#define START_PIN               GPIO_ADS_START_PIN
#define START_SEL               
#define START_DIR               GPIO_ADS_START_DIR

#define DEBUG_OUT               P10OUT
#define DEBUG_PIN               PIN6
#define DEBUG_SEL               P10SEL
#define DEBUG_DIR               P10DIR

#define ADS_RESET_BASE          GPIO_ADS_N_RESET_PORT
#define ADS_RESET_PIN           GPIO_ADS_N_RESET_PIN
#define ADS_RESET_SEL           
#define ADS_RESET_DIR           GPIO_ADS_N_RESET_DIR

#define ADS_PWR_DOWN_OUT        
#define ADS_PWR_DOWN_PIN
#define ADS_PWR_DOWN_SEL
#define ADS_PWR_DOWN_DIR

#define ADS_SPI_BASE               SPI0_BASE

#define HIGH_TO_LOW 1
#define LOW_TO_HIGH 0


/**************************************************************************************************************************************************
*               Prototypes                                                                                                                        *
**************************************************************************************************************************************************/
unsigned char init_ADS1x9x_done(void);
unsigned char init_ADS1x9x (void);
unsigned char ADS1x9x_SPI_Burst (unsigned char);
void init_ADS1x9x_IO (void);
void pwrp_ADS1x9x(void);
void POR_Reset_ADS1x9x (void);
void Hard_Start_ReStart_ADS1x9x (void);
void Hard_Stop_ADS1x9x(void);
void Soft_Start_ReStart_ADS1x9x (void);
void Soft_Stop_ADS1x9x (void);
void Hard_Reset_ADS1x9x (void);
void Soft_Reset_ADS1x9x (void);
void Stop_Read_Data_Continuous (void);
void Start_Read_Data_Continuous (void);
void enable_ADS1x9x_Conversion (void);
unsigned char Initialize_ADS1x9x_Registers (void);
unsigned char ADS1x9x_Read_Version (void);
void ADS1x9x_SPI_Address_Byte_Count (unsigned char, unsigned char);
unsigned char ADS1x9x_SPI_Data (unsigned char);
void init_ADS1x9x_Via_Constant_Table (unsigned char*);
unsigned char verify_ADS1x9x_Registers (unsigned char*);
unsigned char Enable_ADS1x9x_Test_Mode (unsigned char, unsigned char, unsigned char);
void Set_ADS1x9x_Chip_Enable (void);
void Clear_ADS1x9x_Chip_Enable (void);
void setup_ADS1x9xSPI(void);
void hillbilly(void);
void ADS1x9x_Toggle_Read();
uint8_t ADS1x9x_Should_Read();
uint8_t ADS1x9x_Do_Read_Continuous();
void ADS1x9x_Enable_Test_Mode(uint8_t state);

unsigned char Initialize_ADS1x9x_Data_Rate (unsigned char);
unsigned char Initialize_ADS1x9x_Mode (unsigned char);
unsigned char Initialize_ADS1x9x_Channel (unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
