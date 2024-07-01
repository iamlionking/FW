#include "sapphire_pub.h"

#include "ecg_adc.h"
#include "ecg_battery.h"
#include "ecg_gpio.h"
#include "ecg_led.h"
#include "gpio.h"
#include "timer.h"

#define BATT_LOW_ON						4
#define BATT_LOW_OFF					0

#define MAX_DISCHARGE_PERCENTAGE        100
#define MAX_BATTERY_EQN_OUTPUT			16000000
#define MAX_BATT_VOLTAGE                4200
#define BATT_VOLTAGE_HIGH_THRESHOLD     3800
#define BATT_VOLTAGE_MID_THRESHOLD      3670
#define BATT_LOW_HYSTERISIS				50

#define BATTERY_TIMER_BASE              TIMER2_BASE
#define BATTERY_TIMER                   TIMER_A             //Note: The status driver uses Timer B of Timer 2
#define BATTERY_TIMER_TIMEOUT           TIMER_TIMA_TIMEOUT

#define TIMER_PRESCALE                  0xfa
#define TIMER_TICKS_PER_MSEC            320
#define TIMER_MSEC_PER_INT              1
#define TIMER_TICKS_PER_INT             TIMER_TICKS_PER_MSEC * TIMER_MSEC_PER_INT

static uint8_t charge_status;
static uint8_t led_on;
static uint32_t cur_period;
static uint32_t cur_duty_cycle;
static uint32_t int_count;
static uint8_t batt_low;

const battery_eqn_t batt_eqn_high = { 20691, -70025000 };
const battery_eqn_t batt_eqn_mid = { 45872, -166300000 };
const battery_eqn_t batt_eqn_low = { 6356, -21337000 };

static void battery_timer_handler(void);

void ECG_Battery_Init(void)
{
    cur_period = 0;
    cur_duty_cycle = 0;
    int_count = 0;
    charge_status = 0;
    led_on = 1;
    
    TimerPrescaleSet(BATTERY_TIMER_BASE, BATTERY_TIMER, TIMER_PRESCALE);
    TimerLoadSet(BATTERY_TIMER_BASE, BATTERY_TIMER, TIMER_TICKS_PER_INT); /* 1 ms */
    
    TimerIntRegister(BATTERY_TIMER_BASE, BATTERY_TIMER_TIMEOUT, &battery_timer_handler);
	batt_low = BATT_LOW_OFF;
} /* ECG_Battery_Init() */


/* Public - Retrieves the current charge level of the battery.
 * 
 * Examples
 * 
 *      uint32_t batt_level;
 * 
 *      ECG_Battery_Get_Level(&batt_level);
 * 
 * Returns nothing
 */
void ECG_Battery_Get_Level(uint32_t *level)
{
    uint16_t cur_voltage;
    int32_t temp_level;
    int32_t temp;
	battery_eqn_t eqn;
    
    ECG_ADC_GetValue(ECG_Adc_Battery_Voltage, &cur_voltage);
        
	/* Select Equation */
    if(cur_voltage <= BATT_VOLTAGE_MID_THRESHOLD)
    {
        eqn = batt_eqn_low;
    }
    else if(cur_voltage <= BATT_VOLTAGE_HIGH_THRESHOLD)
    {
        eqn = batt_eqn_mid;
    }
    else
    {
		eqn = batt_eqn_high;
    }   
    
	temp = (cur_voltage * eqn.m);
	temp = (temp + eqn.b) * 100;
	temp_level = temp / MAX_BATTERY_EQN_OUTPUT;
	
	if(temp_level > 100)
		temp_level = 100;
	
    if(temp_level < 0)
        temp_level = 0;

    *level = (uint32_t)temp_level;
} /* ECG_Battery_Get_Level() */


/* Public - Retrieves the status of the battery
 * 
 * Examples
 * 
 *      uint8_t batt_status;
 * 
 *      ECG_Battery_Get_Status(&batt_status);
 * 
 * Returns nothing
 */
void ECG_Battery_Get_Status(uint8_t *status)
{
	uint16_t batt_voltage;
	
    *status = 0;
    *status |= GPIOPinRead(GPIO_BATT_S1_PORT, GPIO_BATT_S1_PIN);
    *status |= GPIOPinRead(GPIO_BATT_S2_PORT, GPIO_BATT_S2_PIN);
	
	ECG_ADC_GetValue(ECG_Adc_Battery_Voltage, &batt_voltage);
	
	if(batt_low == BATT_LOW_ON &&
		batt_voltage > (BATT_VOLTAGE_MID_THRESHOLD + BATT_LOW_HYSTERISIS))
	{
		batt_low = BATT_LOW_OFF;
	}
	else if(batt_voltage <= BATT_VOLTAGE_MID_THRESHOLD)
		batt_low = BATT_LOW_ON;
	
	*status |= batt_low;
} /* ECG_Battery_Get_Status() */


/* Public - Turns on the charging LED when the charger is connected.
 * 
 * Examples
 * 
 *      //After charger is known to have been connected
 *      ECG_Battery_Charger_Connect();
 * 
 * Returns nothing
 */
void ECG_Battery_Charger_Connect(void)
{
    ECG_LED_Enable(ECG_LED_0);
    led_on = 1;
	//led_on = 0;
} /* ECG_Battery_Charger_Connect() */


/* Public - Turns off the charging LED when the charger is connected.
 * 
 * Examples
 * 
 *      //After charger is known to have been disconnected
 *      ECG_Battery_Charger_Disonnected();
 * 
 * Returns nothing
 */
void ECG_Battery_Charger_Disconnect(void)
{
    TimerIntDisable(BATTERY_TIMER_BASE, BATTERY_TIMER_TIMEOUT);
    TimerDisable(BATTERY_TIMER_BASE, BATTERY_TIMER);
    ECG_LED_Disable(ECG_LED_0);
    led_on = 0;
} /* ECG_Battery_Charger_Disconnect() */


/* Public - Sets the status of the charge on the battery
 * 
 * Examples
 * 
 *      //Set the battery charge status to medium
 *      ECG_Battery_Set_Charge_Status(Battery_Charge_Medium);
 * 
 * Returns nothing
 */
void ECG_Battery_Set_Charge_Status(Battery_Charge_Status_t8 status)
{
    uint8_t restart_timer = 0;
    charge_status = 0;
    charge_status = status;
    
    switch(status)
    {
        case Battery_Charge_Low:
            cur_period = 1000;
            cur_duty_cycle = 50;
            restart_timer = 1;
            break;
        case Battery_Charge_High:
            cur_period = 500;
            cur_duty_cycle = 50;
            restart_timer = 1;
            break;
        case Battery_Charge_Full:
            cur_period = 0;
            cur_duty_cycle = 0;
            restart_timer = 0;
            TimerIntDisable(BATTERY_TIMER_BASE, BATTERY_TIMER_TIMEOUT);
            TimerDisable(BATTERY_TIMER_BASE, BATTERY_TIMER);
            ECG_LED_Enable(ECG_LED_0);
            led_on = 1;
            break;
		default:
			break;
    }
    
    if(restart_timer == 1)
    {
        TimerIntDisable(BATTERY_TIMER_BASE, BATTERY_TIMER_TIMEOUT);
        TimerDisable(BATTERY_TIMER_BASE, BATTERY_TIMER);
        int_count = (cur_period * cur_duty_cycle) / 100;
        if(int_count > 0)
        {
            TimerIntEnable(BATTERY_TIMER_BASE, BATTERY_TIMER_TIMEOUT);
            TimerEnable(BATTERY_TIMER_BASE, BATTERY_TIMER);
        }
    }
} /* ECG_Battery_Set_Charge_Status() */


/* Internal - Handles the timing of flashing the charging LED.
 * 
 * Returns nothing
 */
void battery_timer_handler(void)
{
    /* Clear the interrupt */
    TimerIntClear(BATTERY_TIMER_BASE, BATTERY_TIMER_TIMEOUT);
    
    if(--int_count == 0)
    {
        if(led_on == 1)
        {
            ECG_LED_Disable(ECG_LED_0);
            led_on = 0;
            int_count = (cur_period * (100 - cur_duty_cycle)) / 100;
        }
        else
        {
            ECG_LED_Enable(ECG_LED_0);
            led_on = 1;
            int_count = (cur_period * cur_duty_cycle) / 100;
        }
    }
} /* battery_timer_handler() */