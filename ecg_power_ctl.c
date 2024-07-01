#include "sapphire_pub.h"

#include "BKRNLAPI.h"
#include "bluetooth.h"
#include "bt_transport.h"
#include "ecg_adc.h"
#include "ecg_battery.h"
#include "ecg_data_capture.h"
#include "ecg_power_ctl.h"
#include "gpio.h"
#include "ecg_gpio.h"
#include "ecg_pwm.h"
#include "ecg_led.h"
#include "ecg_status.h"
#include "interrupt.h"
#include "sysctl.h"
#include "timer.h"

#include "BSCAPI.h"
#include "SPPAPI.h"

#define BATT_LOW_THRESHOLD			3670
#define BATT_VOLTAGE_HYSTERISIS		50

#define EXTERNAL_5V_ON_THRESHOLD    4200
#define POWER_OFF_MINUTES           30
#define POWER_OFF_MSEC              POWER_OFF_MINUTES * 60 * 1000

#define BATTERY_NOT_CHARGING		0x01
#define BATTERY_CHARGING			0x02

static const ECG_Power_Supply_t power_supply_config[] =
{
    {
        ECG_Power_Supply_1V8,
        GPIO_PS1V8_EN_PORT,
        GPIO_PS1V8_EN_PIN
    },
    {
        ECG_Power_Supply_3V3,
        GPIO_PS3V3_EN_PORT,
        GPIO_PS3V3_EN_PIN
    },
    {
        ECG_Power_Supply_5V0,
        GPIO_PS5V0_EN_PORT,
        GPIO_PS5V0_EN_PIN
    }
};

static uint8_t ext_connected = 0;

#ifdef ECG_HARDWARE_BATTERY
static uint32_t starting_tick;
static Battery_Charge_Status_t8 charge_status = Battery_Charge_Unknown;
#endif

/* Public - Initializes the power supplies
 * 
 * Examples
 * 
 *      ECG_Power_Init();
 * 
 * Returns nothing
 */
void ECG_Power_Init(void)
{
#ifdef ECG_HARDWARE_BATTERY
    GPIODirModeSet(GPIO_PS1V8_EN_PORT, GPIO_PS1V8_EN_PIN, GPIO_PS1V8_EN_DIR);
    GPIOPadConfigSet(GPIO_PS1V8_EN_PORT, GPIO_PS1V8_EN_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
#endif

    GPIODirModeSet(GPIO_PS5V0_EN_PORT, GPIO_PS5V0_EN_PIN, GPIO_PS5V0_EN_DIR);
    GPIOPadConfigSet(GPIO_PS5V0_EN_PORT, GPIO_PS5V0_EN_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);

#ifdef ECG_HARDWARE_BATTERY
    ECG_Power_Supply_Enable(ECG_Power_Supply_1V8);
#endif

    ECG_Power_Supply_Enable(ECG_Power_Supply_5V0);
} /* ECG_Power_Init() */


#ifdef ECG_HARDWARE_BATTERY
/* Public - Checks to see if an external connection is preset
 * and configures the charging LED accordingly.
 * 
 * Examples
 * 
 *      ECG_Power_Check_External_Connection();
 * 
 * Returns nothing
 */
void ECG_Power_Update_Charge_LED(void)
{
    uint32_t batt_level = 0;
    uint8_t batt_status = 0;
	uint16_t batt_voltage;
    
    ECG_Battery_Get_Level(&batt_level);
	
	ECG_ADC_GetValue(ECG_Adc_Battery_Voltage, &batt_voltage);

    if(ext_connected == 0)
    {
        return;
    }

    ECG_Battery_Get_Status(&batt_status);
    
    if(batt_status == BATTERY_NOT_CHARGING && 
		charge_status != Battery_Charge_Full)
    {
		Display(("Set charge status to full\r\n"));
        ECG_Battery_Set_Charge_Status(Battery_Charge_Full);
        charge_status = Battery_Charge_Full;
        return;
    }
	else if(batt_status == BATTERY_CHARGING)
	{
		if(charge_status == Battery_Charge_Full || charge_status == Battery_Charge_Unknown)
		{
			if(batt_voltage <= 4050)
			{
				ECG_Battery_Set_Charge_Status(Battery_Charge_Low);
				charge_status = Battery_Charge_Low;
			}
			else
			{
				ECG_Battery_Set_Charge_Status(Battery_Charge_High);
				charge_status = Battery_Charge_High;
			}
		}
		else if(charge_status == Battery_Charge_Low)
		{
			if(batt_voltage > (4050 + 15))
			{
				ECG_Battery_Set_Charge_Status(Battery_Charge_High);
				charge_status = Battery_Charge_High;
			}
		}
		else if(charge_status == Battery_Charge_High)
		{
			if(batt_voltage <= 4050)
			{
				ECG_Battery_Set_Charge_Status(Battery_Charge_Low);
				charge_status = Battery_Charge_Low;
			}
		}
	}
} /* ECG_Power_Update_Charge_LED() */
#endif


/* Public - Get the external connection state
 * 
 * Examples
 * 
 *      //Retrieve the external connection state
 *      uint8_t ext_connected;
 * 
 *      ECG_Power_Get_External_Connection_State();
 * 
 * Returns the state of the external voltage connection
 */
uint8_t ECG_Power_Get_External_Connection_State(void)
{
    return ext_connected;
} /* ECG_Power_Get_External_Connection_State() */


/* Public - Set External connection state
 * 
 * Examples
 * 
 *      //Set external connection state to connected
 *      ECG_Power_Set_External_Connection(1);
 * 
 * Returns nothing
 */
void ECG_Power_Set_External_Connection(uint8_t state)
{
    if(state != 1 && state != 0)
        return;

	if(state == 0)
	{
		/* The charger was disconnected. Clear the most recent 
		 * battery charge status.  It will be determined the
		 * next time the charger is plugged in. */
#ifdef ECG_HARDWARE_BATTERY
		if(charge_status != Battery_Charge_Unknown)
			charge_status = Battery_Charge_Unknown;
#endif
	}
    ext_connected = state;
} /* ECG_Power_Set_External_Connection() */


/* Public - Turns on the main processor power supply.
 * This MUST be called just after reset.
 * 
 * Examples
 * 
 *      ECG_Power_Poweron();
 * 
 * Returns nothing.
 */
void ECG_Power_Poweron(void)
{
    /* Turn on 3V3 Power Supply as soon as possible (Init GPIO pin here also) */
    GPIODirModeSet(GPIO_PS3V3_EN_PORT, GPIO_PS3V3_EN_PIN, GPIO_PS3V3_EN_DIR);
    GPIOPadConfigSet(GPIO_PS3V3_EN_PORT, GPIO_PS3V3_EN_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    
    ECG_Power_Supply_Enable(ECG_Power_Supply_3V3);
} /* ECG_Power_Poweron() */


/* Public - Disables a power supply.
 * 
 * Examples
 * 
 *      //Disable 1.8V power supply.
 *      ECG_Power_Supply_Disable(ECG_Power_Supply_1V8);
 * 
 * Returns nothing
 */
void ECG_Power_Supply_Disable(ECG_Power_Supply_ID_t8 id)
{   
    if(id >= ECG_Power_Supply_Count)
        return;
        
    GPIOPinWrite(power_supply_config[id].base, power_supply_config[id].pin, ~power_supply_config[id].pin);
} /* ECG_Power_Supply_Disable() */


/* Public - Enables a power supply.
 * 
 * Examples
 * 
 *      //Enable 1.8V Power supply
 *      ECG_Power_Supply_Enable(ECG_Power_Supply_1V8);
 * 
 * Returns nothing
 */
void ECG_Power_Supply_Enable(ECG_Power_Supply_ID_t8 id)
{
    if(id >= ECG_Power_Supply_Count)
        return;
        
    GPIOPinWrite(power_supply_config[id].base, power_supply_config[id].pin, power_supply_config[id].pin);
} /* ECG_Power_Supply_Enable() */


/* Public - Power off the system.
 * 
 * Examples
 * 
 *      ECG_Power_Off();
 *
 * Returns nothing
 */
void ECG_Power_Off(void)
{
#if defined(__ECG_DEBUG__)
    //volatile uint8_t i;
    //uint32_t tick_count;
    
    //tick_count = BTPS_GetTickCount();
    //Display(("System Shutting down... %d ms\r\n", tick_count));
    //Display(("System Shutdown\r\n"));
    //for(i = 0; i < 255; i++);
#endif

#ifdef ECG_HARDWARE_BATTERY
    ECG_Power_Supply_Disable(ECG_Power_Supply_1V8);
#endif

    ECG_Power_Supply_Disable(ECG_Power_Supply_5V0);
    
    ECG_Power_Supply_Disable(ECG_Power_Supply_3V3);
    
    ECG_Status_Set(ECG_Status_Off);
    
    IntMasterDisable();
} /* ECG_Power_Off() */


#ifdef ECG_HARDWARE_BATTERY
/* Public - Checks to see if the system should automatically turn off after 15 minutes.
 * Sets the system state to Off if auto-poweroff is necessary.
 * 
 * Examples
 * 
 *      while(some_loop_timer)
 *          ECG_Power_Off_Check();
 * 
 * Returns nothing.
 */
void ECG_Power_Off_Check(void)
{
    uint32_t current_tick;
    uint32_t check_val;
    
    current_tick = BTPS_GetTickCount();
    
    /* Check for overflow */
    if(current_tick < starting_tick)
    {
        check_val = (UINT32_MAX - starting_tick) + current_tick;
    }
    else
        check_val = current_tick - starting_tick;
    
    if(check_val < POWER_OFF_MSEC)
        return;
        
	Display(("Auto-power off!\r\n"));
    g_ssSystemState = System_State_Off;
    ECG_LED_Disable(ECG_LED_0);
} /* ECG_Power_Off_Check() */


/* Public - Check to see if the unit should power off due to extremely low battery conditions.
 * 
 * Examples
 * 
 * 		//Inside of some loop
 * 		ECG_Power_Check_Low_Battery_Power_Down();
 * 
 * Returns nothing
 */
void ECG_Power_Check_Low_Battery_Power_Down(void)
{
	uint16_t cur_voltage;
	
	ECG_ADC_GetValue(ECG_Adc_Battery_Voltage, &cur_voltage);
	
	if(cur_voltage >= 3500)
		return;
		
	Display(("Shutting Down due to low battery!\r\n"));
	g_ssSystemState = System_State_Off;
    ECG_LED_Disable(ECG_LED_0);
} /* ECG_Power_Check_Low_Battery_Power_Down() */


/* Public - Resets the auto-power off timer.  This should be called
 * when the unit is doing anything of interest (responding to BT commands,
 * capturing data, etc).
 * 
 * Examples
 * 
 *      ECG_Power_Reset_Off_Timer();
 * 
 * Returns nothing.
 */
void ECG_Power_Reset_Off_Timer(void)
{
    starting_tick = BTPS_GetTickCount();
} /* ECG_Power_Timer_Reset() */
#endif
