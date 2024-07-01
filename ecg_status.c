#include "sapphire_pub.h"

#include "ecg_led.h"
#include "ecg_status.h"
#include "timer.h"

#define STATUS_TIMER_BASE       TIMER2_BASE
#define STATUS_TIMER            TIMER_B             //Note: The battery driver uses Timer A of Timer 2
#define STATUS_TIMER_TIMEOUT    TIMER_TIMB_TIMEOUT

#define TIMER_PRESCALE              0xfa
#define TIMER_TICKS_PER_MSEC        320
#define TIMER_MSEC_PER_INT          1
#define TIMER_TICKS_PER_INT         TIMER_TICKS_PER_MSEC * TIMER_MSEC_PER_INT

#define CAPTURE_PERIOD              750   //MS
#define CAPTURE_DUTY_CYCLE          85

#define DISCOVER_PERIOD             500   //MS
#define DISCOVER_DUTY_CYCLE         50

#define FLASH_PERIOD                1000  //MS
#define FLASH_DUTY_CYCLE            50

#define POST_PERIOD                 100   //MS            
#define POST_DUTY_CYCLE             50


static uint8_t led_on;
static uint8_t led_state;
static uint8_t unit_status;
static uint8_t restart_timer;
static uint8_t status_changed;
static uint8_t error_before_discoverable;
static uint32_t int_count;
static uint32_t cur_period;
static uint32_t cur_duty_cycle;

static void status_timer_handler(void);
static void update_leds();

void ECG_Status_Init()
{
    unit_status = ECG_Status_Off;
    
    TimerPrescaleSet(STATUS_TIMER_BASE, STATUS_TIMER, TIMER_PRESCALE);
    TimerLoadSet(STATUS_TIMER_BASE, STATUS_TIMER, TIMER_TICKS_PER_INT); /* 1 ms */
    
//    TimerIntRegister(STATUS_TIMER_BASE, STATUS_TIMER_TIMEOUT, &status_timer_handler);
    TimerIntRegister(STATUS_TIMER_BASE, STATUS_TIMER, &status_timer_handler);
    
    int_count = 0;
    cur_period = 0;
    cur_duty_cycle = 0;
    led_state = 0;
    led_on = 0;
    status_changed = 0;
	error_before_discoverable = 0;
}

void ECG_Status_Set(ECG_Status_ID_t8 id)
{
    uint8_t cur_status = unit_status;
    
    if(cur_status & ECG_Status_Error &&
        (id != ECG_Status_Normal &&
        id != ECG_Status_Off &&
		id != ECG_Status_Discoverable))
        return;

//	error_before_discoverable = 0;
	if(error_before_discoverable && id == ECG_Status_Normal)
	{
		id = ECG_Status_Error;
	}

	if(cur_status & ECG_Status_Error && id == ECG_Status_Discoverable)
	{
		error_before_discoverable = 1;
	}
    
    unit_status = unit_status & 0x80;
    unit_status |= id;
    
    if((cur_status & 0x7f) == (unit_status & 0x7f))
        status_changed = 0;
    else
        status_changed = 1;
    
    update_leds();
}

void ECG_Status_Clear(ECG_Status_ID_t8 id)
{
    unit_status &= ~id;
    
    update_leds();
}

uint8_t ECG_Status_Check(ECG_Status_ID_t8 id)
{
    return (unit_status & id);
}


static void update_leds()
{
    if(unit_status & ECG_Status_Low_Battery)
    {
        led_state |= 0x02;
        restart_timer = 0;
    }
    else
    {
        led_state &= ~0x02;
        restart_timer = 0;
    }

    /* If nothing other than the Low Battery Status changed, exit function */
    if(status_changed == 0)
        return;

    if(unit_status & ECG_Status_Off)
    {
        cur_period = 0;
        cur_duty_cycle = 0;
        led_state = 0;
        led_on = 0;
        restart_timer = 0;
        
        TimerIntDisable(STATUS_TIMER_BASE, STATUS_TIMER_TIMEOUT);
        TimerDisable(STATUS_TIMER_BASE, STATUS_TIMER);
    }
    else if(unit_status & ECG_Status_POST)
    {
        cur_period = POST_PERIOD;
        cur_duty_cycle = POST_DUTY_CYCLE;
        led_state |= 1;
        restart_timer = 1;
        
        TimerIntDisable(STATUS_TIMER_BASE, STATUS_TIMER_TIMEOUT);
        TimerDisable(STATUS_TIMER_BASE, STATUS_TIMER);
    }
    else if(unit_status & ECG_Status_Normal)
    {
        cur_period = 0;
        cur_duty_cycle = 0;
        led_state |= 1;
        restart_timer = 0;
        
        TimerIntDisable(STATUS_TIMER_BASE, STATUS_TIMER_TIMEOUT);
        TimerDisable(STATUS_TIMER_BASE, STATUS_TIMER);
    }
    else if(unit_status & ECG_Status_Discoverable)
    {
        cur_period = DISCOVER_PERIOD;
        cur_duty_cycle = DISCOVER_DUTY_CYCLE;

		if(error_before_discoverable)
		{
			led_state = 2;
		}
		else
		{
			led_state |= 0x01;
		}

        restart_timer = 1;
        
        TimerIntDisable(STATUS_TIMER_BASE, STATUS_TIMER_TIMEOUT);
        TimerDisable(STATUS_TIMER_BASE, STATUS_TIMER);
    }
    else if(unit_status & ECG_Status_Error)
    {
        cur_period = 0;
        cur_duty_cycle = 0;
        led_state = 2;
        restart_timer = 0;
        
        TimerIntDisable(STATUS_TIMER_BASE, STATUS_TIMER_TIMEOUT);
        TimerDisable(STATUS_TIMER_BASE, STATUS_TIMER);
    }
    else if(unit_status & ECG_Status_Data_Capture)
    {
        cur_period = CAPTURE_PERIOD;
        cur_duty_cycle = CAPTURE_DUTY_CYCLE;
        led_state |= 1;
        restart_timer = 1;
        
        TimerIntDisable(STATUS_TIMER_BASE, STATUS_TIMER_TIMEOUT);
        TimerDisable(STATUS_TIMER_BASE, STATUS_TIMER);
    }
    else if(unit_status & ECG_Status_Flash)
    {
        cur_period = FLASH_PERIOD;
        cur_duty_cycle = FLASH_DUTY_CYCLE;
        led_state |= 1;
        restart_timer = 1;
        
        TimerIntDisable(STATUS_TIMER_BASE, STATUS_TIMER_TIMEOUT);
        TimerDisable(STATUS_TIMER_BASE, STATUS_TIMER);
    }
    
    if(led_state != 0)
        led_on = 1;
        
    if(led_state & 0x01)
        ECG_LED_Enable(ECG_LED_1);
    else
        ECG_LED_Disable(ECG_LED_1);
    
    if(led_state & 0x02)
    {
        if(led_state & 0x01)
            ECG_PWM_Set_Period_Duty_Cycle(ECG_Pwm_Led_2, 8000, 400);
        else
            ECG_PWM_Set_Period_Duty_Cycle(ECG_Pwm_Led_2, 8000, 7999);
        ECG_LED_Enable(ECG_LED_2);
    }
    else
        ECG_LED_Disable(ECG_LED_2);
    
    if(restart_timer == 1)
    {
        int_count = (cur_period * cur_duty_cycle) / 100;
        if(int_count > 0)
        {
            TimerIntEnable(STATUS_TIMER_BASE, STATUS_TIMER_TIMEOUT);
            TimerEnable(STATUS_TIMER_BASE, STATUS_TIMER);
        }        
    }
} /* update_leds() */


/* Internal - Timer timeout interrupt for handling the flash frequency of LED 1.
 * 
 * Returns nothing
 */
static void status_timer_handler(void)
{
    /* Clear the interrupt */
    TimerIntClear(STATUS_TIMER_BASE, STATUS_TIMER_TIMEOUT);

    if(--int_count == 0)
    {
        if(led_on == 1)
        {
            led_on = 0;
            if(led_state & 0x01)
                ECG_LED_Disable(ECG_LED_1);
            if(led_state & 0x02)
                ECG_LED_Disable(ECG_LED_2);
            int_count = (cur_period * (100 - cur_duty_cycle)) / 100;
        }
        else
        {
            led_on = 1;
            if(led_state & 0x01)
                ECG_LED_Enable(ECG_LED_1);
            if(led_state & 0x02)
            {
                if(led_state & 0x01)
                    ECG_PWM_Set_Period_Duty_Cycle(ECG_Pwm_Led_2, 8000, 1200);
                else
                    ECG_PWM_Set_Period_Duty_Cycle(ECG_Pwm_Led_2, 8000, 7999);
                ECG_LED_Enable(ECG_LED_2);
            }
            int_count = (cur_period * cur_duty_cycle) / 100;
        }
    }
} /* status_timer_handler() */
