#include "sapphire_pub.h"

#include "bluetooth.h"
#include "ecg_button.h"
#include "ecg_discover.h"
#include "ecg_gpio.h"
#include "ecg_init.h"
#include "ecg_led.h"
#include "ecg_power_ctl.h"
#include "ecg_status.h"
#include "gpio.h"
#include "inc/hw_timer.h"
#include "sysctl.h"
#include "timer.h"

#include "BTVSAPI.h"
#include "GAPAPI.h"

#define DISABLE_BUTTON_INT()                                            \
{                                                                       \
    GPIOIntClear(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN);     \
    GPIOIntDisable(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN);   \
}

static uint8_t button_int_level;
Bluetooth_Pair_State_t8 g_bpsBTPairState;
static uint32_t power_int_count;
static uint32_t pair_int_count;
static uint8_t power_int_occurred;

static void button_handler(void);
static void init_button(void);
static void init_power_pair_timer(void);
static void pair_timer_handler(void);
static void power_timer_handler(void);


/* Public - Initialize the button interface
 * 
 * Examples
 *      
 *      ECG_Button_Init();
 * 
 * Returns nothing
 */
void ECG_Button_Init()
{
    init_power_pair_timer();
    
    init_button();
    
    g_bpsBTPairState = Bluetooth_Pair_State_Not_Discoverable;
    button_int_level = GPIO_LOW_LEVEL;
    power_int_occurred = 0;
} /* ECG_Button_Init() */


/* Internal - Initializes the button GPIO and interrupts necessary for operation.
 * 
 * Returns nothing
 */
void init_button()
{
    /* Initialize Button */
    GPIODirModeSet(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    GPIOIntTypeSet(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN, GPIO_LOW_LEVEL);
    GPIOIntEnable(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN);
    GPIOIntRegister(POWER_PAIR_BUTTON_BASE, &button_handler);
} /* init_button() */


/* Internal - Initializes the timers used for entering pairing mode and powering down the device
 * 
 * Returns nothing
 */
void init_power_pair_timer()
{
    /* Enable Timer 3 Peripheral */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    
    /* Configure Timers A and B as a 16-bit One Shot Timer */
    TimerConfigure(POWER_PAIR_TIMER_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT | TIMER_CFG_B_ONE_SHOT));
    
    /* Set Timers A and B prescaler to 250 and load timeout value */
    TimerLoadSet(POWER_PAIR_TIMER_BASE, TIMER_BOTH, POWER_PAIR_TICKS_PER_INT); /* 200 milliseconds */
    TimerPrescaleSet(POWER_PAIR_TIMER_BASE, PAIR_TIMER, POWER_PAIR_PRESCALE);
    TimerPrescaleSet(POWER_PAIR_TIMER_BASE, TIMER_BOTH, POWER_PAIR_PRESCALE);
    
    /* Register interrupt handlers for Timer A and B respectively */
    TimerIntRegister(POWER_PAIR_TIMER_BASE, PAIR_TIMER_TIMEOUT, &pair_timer_handler);
    TimerIntRegister(POWER_PAIR_TIMER_BASE, POWER_TIMER_TIMEOUT, &power_timer_handler);
} /* init_power_pair_timer() */


/* Internal - Interrupt handler for the button.  The following is some pseudocode for how the interrupt
 * handler operates:
 * 
 * System and Pairing states 
 * System is on
 *      Button is pressed and held for 1 second but less than 3 seconds
 *          System is placed in pairing mode
 *      Button is pressed and held for 3 or more seconds
 *          System is turned off
 * 
 * Returns nothing
 */
static void button_handler(void)
{
    /* Make sure we are looking at the correct interrupt */
    uint32_t status = GPIOIntStatus(POWER_PAIR_BUTTON_BASE, true);
    
    /* Don't handle interrupt if not correct pin */
    if(!(status & POWER_PAIR_BUTTON_PIN))
        return;
    
    /* Clear the interrupt */
    GPIOIntClear(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN);     
    GPIOIntDisable(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN);   
    
    ECG_Power_Reset_Off_Timer();
    
    if(g_ssSystemState == System_State_Off)
    {
        if(button_int_level == GPIO_LOW_LEVEL)
        {
            /* Begin system initialization */
            g_ssSystemState = System_State_Running;
            
            /* Configure for high level interrupt
             * If this occurs before the timer times out, then we
             * don't need to go into pairing mode. */
            button_int_level = GPIO_HIGH_LEVEL;
            GPIOIntTypeSet(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN, button_int_level);
        }
        else
        {
            /* Reconfigure button for low level interrupt */
            button_int_level = GPIO_LOW_LEVEL;
            GPIOIntTypeSet(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN, button_int_level);
        }
    }
    else if(g_ssSystemState == System_State_Running)
    {
        if(button_int_level == GPIO_LOW_LEVEL)
        {
            //Display(("System State Running Button Int Low\r\n"));
            /* Button was pressed */
            /* Start paring timer if not in paring mode */
            if(g_bpsBTPairState == Bluetooth_Pair_State_Not_Discoverable)
            {
                TimerIntEnable(POWER_PAIR_TIMER_BASE, PAIR_TIMER_TIMEOUT);
                TimerEnable(POWER_PAIR_TIMER_BASE, PAIR_TIMER);
                pair_int_count = POWER_PAIR_INTS_PER_1_SEC;
            }
            
            /* Start power down timer */
            TimerIntEnable(POWER_PAIR_TIMER_BASE, POWER_TIMER_TIMEOUT);
            TimerEnable(POWER_PAIR_TIMER_BASE, POWER_TIMER);
            power_int_count = POWER_PAIR_INTS_PER_3_SEC;
            
            /* Reconfigure button interrupt to be the high level interrupt */
            button_int_level = GPIO_HIGH_LEVEL;
            GPIOIntTypeSet(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN, button_int_level);
        }
        else if(button_int_level == GPIO_HIGH_LEVEL)
        {
            /* Button was released - Disable power and pairing timer */
            TimerDisable(POWER_PAIR_TIMER_BASE, TIMER_BOTH);
            TimerIntDisable(POWER_PAIR_TIMER_BASE, PAIR_TIMER_TIMEOUT);
            TimerIntDisable(POWER_PAIR_TIMER_BASE, POWER_TIMER_TIMEOUT);
            
            button_int_level = GPIO_LOW_LEVEL;
            GPIOIntTypeSet(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN, button_int_level);
            if(power_int_occurred == 1)
            {
                ECG_Power_Off();
            }
        }
    }
    GPIOIntEnable(POWER_PAIR_BUTTON_BASE, POWER_PAIR_BUTTON_PIN);
    
    /* Set unit state in main loop?? */
} /* ecg_button_handler() */


/* Internal - Timer Timeout interrupt handler for the pairing mode timer.  Each time this interrupt occurs, the
 * interrupt count for the pairing timer is decremented.  If it reaches zero, discoverablity mode shall be enabled.
 * 
 * Returns nothing
 */
static void pair_timer_handler(void)
{
    TimerIntClear(POWER_PAIR_TIMER_BASE, PAIR_TIMER_TIMEOUT);
    TimerIntDisable(POWER_PAIR_TIMER_BASE, PAIR_TIMER_TIMEOUT);
    
    if(--pair_int_count == 0)
    {   
        if(g_bpsBTPairState == Bluetooth_Pair_State_Not_Discoverable &&
           g_POSTComplete == 1)
        {
            g_bpsBTPairState = Bluetooth_Pair_State_Discoverable;
            g_toggleBTDiscoverability = true;
        }
    }
    else
    {
        /* Reload Timer */
        TimerLoadSet(POWER_PAIR_TIMER_BASE, PAIR_TIMER, POWER_PAIR_TICKS_PER_INT);
        TimerIntEnable(POWER_PAIR_TIMER_BASE, PAIR_TIMER_TIMEOUT);
        TimerEnable(POWER_PAIR_TIMER_BASE, PAIR_TIMER);
    }
} /* pair_timer_handler() */


/* Internal - Timer Timeout interrupt handler for the power off timer.  Each time this handler is entered, the
 * interrupt count for the timer is decremented.  If it reaches zero, the system will be turned off.
 * 
 * Returns nothing
 */
static void power_timer_handler(void)
{
    TimerIntClear(POWER_PAIR_TIMER_BASE, POWER_TIMER_TIMEOUT);
    TimerIntDisable(POWER_PAIR_TIMER_BASE, POWER_TIMER_TIMEOUT);
    TimerDisable(POWER_PAIR_TIMER_BASE, POWER_TIMER);
    
    if(--power_int_count == 0)
    {
        /* Initiate power down sequence - Disable pairing timer*/
        TimerIntClear(POWER_PAIR_TIMER_BASE, PAIR_TIMER_TIMEOUT);
        TimerIntDisable(POWER_PAIR_TIMER_BASE, PAIR_TIMER_TIMEOUT);
        TimerDisable(POWER_PAIR_TIMER_BASE, PAIR_TIMER);
        //Display(("System powering down...\r\n"));
        /* Disable pairing mode */
        power_int_occurred = 1;
        ECG_Status_Set(ECG_Status_Off);
    }
    else
    {
        /* Reload Timer */
        TimerLoadSet(POWER_PAIR_TIMER_BASE, POWER_TIMER, POWER_PAIR_TICKS_PER_INT);
        TimerIntEnable(POWER_PAIR_TIMER_BASE, POWER_TIMER_TIMEOUT);
        TimerEnable(POWER_PAIR_TIMER_BASE, POWER_TIMER);
    }
} /* power_timer_handler() */
