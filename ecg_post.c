#include "sapphire_pub.h"

#include "ADS1x9x.h"
#include "bluetooth.h"
#include "device_api.h"
#include "ecg_adc.h"
#include "ecg_gpio.h"
#include "ecg_post.h"
#include "ecg_power_ctl.h"
#include "ecg_ssi.h"
#include "ecg_status.h"
#include "gpio.h"
#include "interrupt.h"
#include "ssi.h"
#include "timer.h"

#include "ecg_pwm.h"

#define POST_TEST_SIGNAL_CYCLE_COUNT		2

#define BT_INITIALIZATION_SHIFT             0
#define SUPPLY_5V0_SHIFT                    1
#define SUPPLY_1V8_SHIFT                    2
#define SUPPLY_VCORE_SHIFT                  3
#define BATTERY_STATUS1_SHIFT               4
#define BATTERY_STATUS2_SHIFT               5
#define ADS_COMM_VALID_SHIFT                6
#define ADS_TEST_SIGNAL_VALID_SHIFT         7
#define BATTERY_PRESENT_SHIFT               8
#define EXTERNAL_VOLTAGE_PRESENT_SHIFT      9

#define ECG_SAMPLES_PER_CYCLE_AT_250SPS     256

#define POST_TIMEOUT_TIMER_BASE				TIMER1_BASE
#define POST_TIMEOUT_TIMER					TIMER_B
#define POST_TIMEOUT_PRESCALER				250
#define POST_TIMEOUT_RELOAD					64000 //200 ms
#define POST_TIMEOUT_PERIOD					30 //6 seconds * 5 200ms interrupts

#define ADC_ANALYSIS_MINIMUM_INDEX			0
#define ADC_ANALYSIS_MAXIMUM_INDEX			1
#define ADC_ANALYSIS_MEAN_INDEX				2

static uint32_t status;
static uint8_t drdy_int_state;
volatile uint8_t ads_data_analysis_running;
static uint8_t rx_data[27];
static uint32_t sample_count;
static uint8_t current_msb;
static uint8_t starting_cycle_msb;
static uint8_t last_transition_sample_number;
static uint8_t frequency_ok;
static uint8_t cycle_count;
static uint8_t first_transition_occurred;
static uint16_t starting_cycle_sample_count;
static uint8_t post_timeout_int_cnt;

static uint16_t frequency_sample_count;
static int32_t adc_analysis[2][8];

#ifdef ECG_HARDWARE_BATTERY
static void set_battery_status_bits(void);
#endif

static void check_ads(void);
static void data_ready_handler(void);
static void post_timeout_handler(void);
static double convert_adc_mv(int32_t);
static double calculate_adc_accuracy(double);


/* Public - Performs a Power-on Self Test
 * 
 * Examples
 * 
 *      ECG_Post_Run()
 * 
 * Returns nothing
 */
void ECG_POST_Run(void)
{
    uint16_t adc_5v0 = 0;
    uint16_t adc_1v8 = 0;
    uint16_t adc_vc = 0;
    uint16_t adc_ext_5v = 0;
	double adc_mvs = 0;
	double adc_accuracy_ratio = 0;
//	uint8_t adc_accuracy_percent = 0;
	uint8_t i;

    for(i = 0; i < ECG_Pwm_Count; i++)
	{
		ECG_PWM_Disable(i);
	}
    
    ads_data_analysis_running = 0;
    status = 0;
    

    Display(("Running Power-on Self Test...\r\n"));

    /*
    Verify BT initialized (BT)
    Verify Power supply voltages (50 18 VC)
    Check battery charger status bits (S1 S0)
    Verify ADS communication (configure and request version number) (AD)
    Check for battery (ADC) (BA)
    Check if wall power is connected (EX) */
    
#ifdef ECG_HARDWARE_BLUETOOTH
    /* Verify BT Initialized */
    if(GetInitializationStatus())
    {
        status |= (1 << BT_INITIALIZATION_SHIFT);
        Display(("Bluetooth Initialization..............[Pass]\r\n"));
    }
    else
    {
        Display(("Bluetooth Initialization..............[Fail]\r\n"));
        ECG_Status_Set(ECG_Status_Error);
    }
#endif

    ECG_ADC_GetValue(ECG_Adc_1V8, &adc_1v8);
    ECG_ADC_GetValue(ECG_Adc_VCore, &adc_vc);
    ECG_ADC_GetValue(ECG_Adc_External_5V, &adc_ext_5v);
        
#ifdef ECG_HARDWARE_BATTERY
    if(adc_1v8 > 1600)
    {
        status |= (1 << SUPPLY_1V8_SHIFT);
        Display(("1V8 Supply............................[Pass]\r\n"));
    }
    else
    {
        Display(("1V8 Supply............................[Fail]\r\n"));
        ECG_Status_Set(ECG_Status_Error);
    }
#endif

#ifdef ECG_HARDWARE_BLUETOOTH
    if(adc_vc > 1150)
    {
        status |= (1 << SUPPLY_VCORE_SHIFT);
        Display(("VCore Supply..........................[Pass]\r\n"));
    }
    else
    {
        Display(("VCore Supply..........................[Fail]\r\n"));
        ECG_Status_Set(ECG_Status_Error);
    }
#endif

#ifdef ECG_HARDWARE_BATTERY
    /* Battery Status */
    set_battery_status_bits();
    Display(("Set battery status bits...............[Pass]\r\n"));
#endif

    ECG_ADC_GetValue(ECG_Adc_5V0, &adc_5v0);
    if(adc_ext_5v > 4400)
    {
        status |= (1 << EXTERNAL_VOLTAGE_PRESENT_SHIFT);
        status |= (1 << SUPPLY_5V0_SHIFT);
        Display(("5V0 Supply............................[Pass]\r\n"));
    }
    else
    {
        Display(("5V0 Supply %d........................[Fail]\r\n", adc_ext_5v));
        ECG_Status_Set(ECG_Status_Error);
    }

	/* Configure POST timeout handler */
	post_timeout_int_cnt = 0;
//	TimerConfigure(POST_TIMEOUT_TIMER_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_ONE_SHOT));
	TimerConfigure(POST_TIMEOUT_TIMER_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT | TIMER_CFG_B_ONE_SHOT);
    TimerPrescaleSet(POST_TIMEOUT_TIMER_BASE, POST_TIMEOUT_TIMER, POST_TIMEOUT_PRESCALER);
	TimerLoadSet(POST_TIMEOUT_TIMER_BASE, POST_TIMEOUT_TIMER, POST_TIMEOUT_RELOAD);
    TimerIntRegister(POST_TIMEOUT_TIMER_BASE, POST_TIMEOUT_TIMER, post_timeout_handler);
    TimerIntEnable(POST_TIMEOUT_TIMER_BASE, TIMER_TIMB_TIMEOUT);
	TimerEnable(POST_TIMEOUT_TIMER_BASE, POST_TIMEOUT_TIMER);
    
    ADS1x9x_Enable_Test_Mode(1);
    check_ads();
    ADS1x9x_Enable_Test_Mode(0);
    
	TimerIntClear(POST_TIMEOUT_TIMER_BASE, TIMER_TIMB_TIMEOUT);
    TimerIntDisable(POST_TIMEOUT_TIMER_BASE, TIMER_TIMB_TIMEOUT);
	TimerDisable(POST_TIMEOUT_TIMER_BASE, POST_TIMEOUT_TIMER);

	for(int i = 0;i < 8;i++)
	{
/*
		Display(("\r\nADC Index................................[%d]\r\n", i));
		Display(("Minimum Voltage.....................[%d]\r\n", adc_analysis[ADC_ANALYSIS_MINIMUM_INDEX][i]));
		Display(("Maximum Voltage......................[%d]\r\n", adc_analysis[ADC_ANALYSIS_MAXIMUM_INDEX][i]));
*/
		adc_mvs = convert_adc_mv(adc_analysis[ADC_ANALYSIS_MAXIMUM_INDEX][i] - adc_analysis[ADC_ANALYSIS_MINIMUM_INDEX][i]);

		adc_accuracy_ratio = calculate_adc_accuracy(adc_mvs);
//		adc_accuracy_percent = 100 - (uint8_t)(adc_accuracy_ratio * 100);

/*
		if(adc_accuracy_ratio >= -0.07f && adc_accuracy_ratio <= 0.07f)
		{
			Display(("ADS Channel %d Accuracy = %d%%.........[Pass]\r\n", i, adc_accuracy_percent));
		}
		else
		{
			Display(("ADS Channel %d Accuracy = %d%%.........[Fail]\r\n", i, adc_accuracy_percent));
			frequency_ok = 0;
			ECG_Status_Set(ECG_Status_Error);
		}
	}
*/	
		if(adc_accuracy_ratio < -0.07f || adc_accuracy_ratio > 0.07f)
		{
			frequency_ok = 0;
		}
	}

    if(frequency_ok)
    {
		Display(("ADS Test Signal.......................[Pass]\r\n"));
		status |= (1 << ADS_TEST_SIGNAL_VALID_SHIFT);
    }
    else
    {
		Display(("ADS Test Signal.......................[Fail]\r\n"));
        ECG_Status_Set(ECG_Status_Error);
    }

	/* Initialize the ADS back to a normal state */
	init_ADS1x9x();
} /* ECG_Power_On_Self_Test() */


/* Public - Returns the result of the Power-on Self Test
 * 
 * Examples
 * 
 *      uint32_t post_result;
 * 
 *      post_result = ECG_POST_Get_Result();
 * 
 * Returns the result of the POST.
 */
uint32_t ECG_POST_Get_Result(void)
{
    return status;
} /* ECG_POST_Get_Result() */

/* Internal - Converts the three bytes received from the ADC into a 32bit integer
 * address MUST be followed by two more bytes or we will be reading arbitrary memory
 * 
 * Returns the ADC value as a 32bit integer
 */
static int32_t process_adc_value(uint8_t* address)
{
	int32_t result = 0;

	/* Handle the sign bit of our two's complement data */
	if(*address & 0x80)
	{
		/* Make the first byte all 1s because we are negative */
		result = 0xFF000000;
	}

	result |= *address << 16;
	result |= *(++address) << 8;
	result |= *(++address);

	return result;
} /*process_adc_value(uint8_t* address)*/

/* Internal - Converts the value received from an ADC into mV
 * 
 * Returns a double representing the mV read from the ADC
 */
static double convert_adc_mv(int32_t adc)
{
	double result = 0;
	
	/* LSb value of ADC as per data sheet. 1 << 23 is 2 to the 23rd power */
	double unit_voltage = 4.0f / ((1 << 23) - 1);

	result = unit_voltage * adc;

	/* Convert to mV */
	result *= 1000;

	/* Account for gain of 8 */
	result /= 8.0f;

	return result;
} /*convert_adc_mv(uint32_t adc)*/

/* Internal - Converts the value received from an ADC into mV
 * 
 * Returns a double representing the mV read from the ADC
 */
static double calculate_adc_accuracy(double amplitude_mvs)
{
	double result = 0;
	double expected_amplitude = (2.0f * (4.0f / 2.4f)) * 2;

	if(amplitude_mvs < 0)
	{
		amplitude_mvs *= -1.0f;
	}

	result = amplitude_mvs / expected_amplitude;

	if(result < 1.0f)
	{
		result = (result - 1.0f) * -1.0f;
	}
	else
	{
		result -= 1.0f;
	}

	return result;
} /*convert_adc_accuracy(double adc) */

/* Internal - Verifies communication with the ADS.
 * 
 * Returns nothing.
 */
static void check_ads(void)
{
    uint8_t result = 0;

	/* Initialize ADC Analysis array */
	for(int i = 0;i < 8;i++)
	{
		adc_analysis[ADC_ANALYSIS_MINIMUM_INDEX][i] = 0x7FFFFFFF;
		adc_analysis[ADC_ANALYSIS_MAXIMUM_INDEX][i] = 0x80000000;
	}
    
    /* Set SSI Bus to ADS mode */
    if(ECG_SSI_Get_Mode() != SSI_ADS_Mode)
        ECG_SSI_Set_Mode(SSI_ADS_Mode);
    
    /* Initialize ADS */
    result = init_ADS1x9x();
    if(result == ADS_1x9x_INIT_SUCCESS)
    {
        status |= (1 << ADS_COMM_VALID_SHIFT);
        Display(("ADS Communication.....................[Pass]\r\n"));
    }
    else
    {
        Display(("ADS Communication: %d...................[Fail]\r\n", result));
        ECG_Status_Set(ECG_Status_Error);
		// TODO: Uncomment
        //return;
    }
    
    /* Set ADS Data Rate */
    ADS1x9x_SPI_Address_Byte_Count(WRITE_CONFIG_1_REGISTER, SINGLE_BYTE_READ_WRITE);
    ADS1x9x_SPI_Data(6);
    
    /* Initialize Data Ready Interrupt */
    drdy_int_state = GPIO_HIGH_LEVEL;
    GPIOIntTypeSet(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN, GPIO_HIGH_LEVEL);
    GPIOIntEnable(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN);
    GPIOIntRegister(GPIO_ADS_N_DRDY_PORT, data_ready_handler);
    
    ads_data_analysis_running = 1;
    last_transition_sample_number = 0;
    sample_count = 0;
    starting_cycle_msb = 0;
    current_msb = 0;
    frequency_ok = 1;

	frequency_sample_count = 0;

    first_transition_occurred = 0;
    
    /* Start ADS Conversion */
    enable_ADS1x9x_Conversion();
    
    while(ads_data_analysis_running == 1);
    
    IntMasterDisable();
    Soft_Stop_ADS1x9x();
    Stop_Read_Data_Continuous();
    GPIOIntDisable(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN);
    GPIOIntUnregister(GPIO_ADS_N_DRDY_PORT);
    IntMasterEnable();
} /* check_ads() */


#ifdef ECG_HARDWARE_BATTERY
/* Internal - Sets the battery status bits in the POST result.
 * 
 * Returns nothing
 */
static void set_battery_status_bits(void)
{
    if(GPIOPinRead(GPIO_BATT_S1_PORT, GPIO_BATT_S1_PIN) & GPIO_BATT_S1_PIN)
        status |= (1 << BATTERY_STATUS1_SHIFT);
    
    if(GPIOPinRead(GPIO_BATT_S2_PORT, GPIO_BATT_S2_PIN) & GPIO_BATT_S2_PIN)
        status |= (1 << BATTERY_STATUS2_SHIFT);
} /* set_battery_status_bits() */
#endif


/* Internal - Interrupt handler for verifying ADS test signal.
 * 
 * Returns nothing
 */
static void data_ready_handler(void)
{
    uint8_t i;
    uint32_t temp;
    uint8_t msb_diff_count;
	int32_t adc_value;
	uint8_t adc_index;
    
    /* Make sure we are looking at the correct interrupt */
    uint32_t status = GPIOIntStatus(GPIO_ADS_N_DRDY_PORT, true);
    
    /* Don't handle interrupt if not data ready pin */
    if(!(status & GPIO_ADS_N_DRDY_PIN))
    {
        return;
    }
    
    GPIOIntClear(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN);
    GPIOIntDisable(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN);
    if(drdy_int_state == GPIO_HIGH_LEVEL)
    {
        GPIOIntTypeSet(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN, GPIO_LOW_LEVEL);
        drdy_int_state = GPIO_LOW_LEVEL;
    }
    else
    {
        /* Set Interrupt to wait for high level so we don't constantly trigger */
        GPIOIntTypeSet(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN, GPIO_HIGH_LEVEL);
        drdy_int_state = GPIO_HIGH_LEVEL;
        
        BTPS_MemInitialize(rx_data, 0x00, ADS1x9x_DATA_SIZE);
        
        Set_ADS1x9x_Chip_Enable();
        for(i = 0; i < ADS1x9x_DATA_SIZE; i++)
        {
            SSIDataPut(SSI0_BASE, 0x00);
            while(SSIBusy(SSI0_BASE));
            SSIDataGet(SSI0_BASE, &temp);
            rx_data[i] = (uint8_t)temp;
        }
        Clear_ADS1x9x_Chip_Enable();
        
        msb_diff_count = 0;
        /* Examine data */
        for(i = 3; i < ADS1x9x_DATA_SIZE; i += 3)
        {
            if(first_transition_occurred == 1)
            {
				adc_index = (i / 3) - 1;

				adc_value = process_adc_value(rx_data + i);

				if(adc_value < adc_analysis[ADC_ANALYSIS_MINIMUM_INDEX][adc_index])
				{
					adc_analysis[ADC_ANALYSIS_MINIMUM_INDEX][adc_index] = adc_value;
				}
				
				if(adc_value > adc_analysis[ADC_ANALYSIS_MAXIMUM_INDEX][adc_index])
				{
					adc_analysis[ADC_ANALYSIS_MAXIMUM_INDEX][adc_index] = adc_value;
				}
			}

            /* Look at MSB */
            if(current_msb != rx_data[i])
                msb_diff_count++;
        }
        
        /* If all 8 MSB's are different from the current MSB, then the signal has transitioned */
        if(msb_diff_count == 8)
        {
            /* If we have had at least one transition, then we can procede with further calculations 
             * This is to account for the fact that a few samples are missed in the beginning while the
             * ADS sets up it's data.  So, the first transition may occur a few samples early and the
             * frequency would be wrong if we started based on the first transition. */
            if(first_transition_occurred == 1)
            {
                current_msb = rx_data[3];
                
                /* If the current_msb matches the MSB of cycle 0 (the MSB of the first transition), then one cycle has
                 * passed.  We should have had 128 samples since this last occurred */
                if(current_msb == starting_cycle_msb)
                {
					frequency_sample_count = sample_count - starting_cycle_sample_count;

//                    if(frequency_sample_count % 128 != 0)
                    if(frequency_sample_count % 128 > 2 && frequency_sample_count % 128 < 125)
                        frequency_ok = 0;

                    cycle_count++;
                }
            }
            else
            {
                /* This is the first transition on record.
                 * Set the current msb and starting cycle msb values.
                 * Also, set the current sample count number as the first sample of the first cycle */
                first_transition_occurred = 1;
                current_msb = rx_data[3];
                starting_cycle_msb = current_msb;
                starting_cycle_sample_count = sample_count;
            }
            last_transition_sample_number = sample_count;
        }   
        
        sample_count++;
    }
    if(cycle_count < POST_TEST_SIGNAL_CYCLE_COUNT)
        GPIOIntEnable(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN);
    else
        ads_data_analysis_running = 0;
} /* data_ready_handler() */


/* Internal - Interrupt handler for POST timeout.
 * 
 * Returns nothing
 */
static void post_timeout_handler(void)
{
	TimerIntClear(POST_TIMEOUT_TIMER_BASE, TIMER_TIMB_TIMEOUT);
    TimerIntDisable(POST_TIMEOUT_TIMER_BASE, TIMER_TIMB_TIMEOUT);
	
	if(++post_timeout_int_cnt >= POST_TIMEOUT_PERIOD)
    {   
        ads_data_analysis_running = 0;
		frequency_ok = 0;
		TimerDisable(POST_TIMEOUT_TIMER_BASE, POST_TIMEOUT_TIMER);
    }
    else
    {
        /* Reload Timer */
        TimerLoadSet(POST_TIMEOUT_TIMER_BASE, POST_TIMEOUT_TIMER, POST_TIMEOUT_RELOAD);
        TimerIntEnable(POST_TIMEOUT_TIMER_BASE, TIMER_TIMB_TIMEOUT);
        TimerEnable(POST_TIMEOUT_TIMER_BASE, POST_TIMEOUT_TIMER);
    }
} /* post_timeout_handler() */

