#include "sapphire_pub.h"

#include "ecg_gpio.h"
#include "ecg_led.h"
#include "ecg_pwm.h"
#include "gpio.h"
#include "sysctl.h"
#include "timer.h"

#define PWM_MS_PERIOD               10000

//#define LED_TIMER_BASE              TIMER2_BASE
//#define TIMER_PRESCALE              0x0a
//#define TIMER_TICKS_PER_USEC        8        
//#define TIMER_USEC_PER_INT          500
//#define TIMER_INTS_PER_MSEC         1000 / TIMER_USEC_PER_INT
//#define TIMER_INTS_PER_SEC          1000 * TIMER_INTS_PER_MSEC
//#define TIMER_TICKS_PER_INT         TIMER_TICKS_PER_USEC * TIMER_USEC_PER_INT

static const ECG_LED_t led_default_config[] =
{
    {
        ECG_LED_0,
        ECG_Pwm_Led_0,
        0,
        GPIO_LED0_PIN,
        GPIO_LED0_PORT,
    },
    {
        ECG_LED_1,
        ECG_Pwm_Led_1,
        0,
        GPIO_LED1_PIN,
        GPIO_LED1_PORT,
    },
    {
        ECG_LED_2,
        ECG_Pwm_Led_2,
        0,
        GPIO_LED2_PIN,
        GPIO_LED2_PORT,
    }
};

ECG_LED_t led_config[ECG_LED_COUNT];

//static void led0_timer_handler(void);

/* Public - Initializes the LEDs on the device
 * 
 * Examples
 * 
 *      ECG_LED_Init();
 * 
 * Returns nothing
 */
void ECG_LED_Init(void)
{
    //uint8_t i;
    
    /* Enable LED Peripherals */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    
    BTPS_MemCopy(led_config, led_default_config, sizeof(ECG_LED_t) * ECG_LED_COUNT);
    
    //for(i = 0; i < ECG_LED_COUNT; i++)
    //{
        //GPIOPinTypeGPIOOutput(led_config[i].gpio_base, led_config[i].pin);
    //}
    
    /* Register interrupt handlers for Timer A and B respectively */
    //TimerIntRegister(LED_TIMER_BASE, led_config[ECG_LED_0].timeout, &led0_timer_handler);
} /* ECG_LED_Init() */


/* Public - Enables the specified LED.
 * 
 * id - The ID of the LED to enable.
 * 
 * Examples
 * 
 *      ECG_LED_Enable(ECG_LED_0);
 * 
 * Returns nothing
 */
void ECG_LED_Enable(ECG_LED_ID_t8 id)
{
    if(!VALID_ID(id))
        return;

    ECG_PWM_Enable(led_config[id].pwm);
    //GPIOPinWrite(led_config[id].gpio_base, led_config[id].pin, led_config[id].pin);
} /* ECG_LED_Enable() */


/* Public - Disables the specified LED.
 * 
 * id - The ID of the LED to disable.
 * 
 * Examples
 * 
 *      ECG_LED_Disable(ECG_LED_1);
 * 
 * Returns nothing
 */
void ECG_LED_Disable(ECG_LED_ID_t8 id)
{
    if(!VALID_ID(id))
        return;
    
    ECG_PWM_Disable(led_config[id].pwm);
    //GPIOPinWrite(led_config[id].gpio_base, led_config[id].pin, ~led_config[id].pin);
} /* ECG_LED_Disable() */


/* Public - Flashes an LED at the specified period with the specified duty cycle.
 * 
 * id - The ID of the LED to control.
 * period - The frequency of the flash. 
 * duty_cycle - The duty cycle of the flash. (amount of time LED is on)
 * 
 * Exmaples
 *      
 *      ECG_LED_Flash(ECG_LED_0, 1000, 50);
 * 
 * Returns nothing
 */
//void ECG_LED_Flash(ECG_LED_ID_t8 id, uint32_t period, uint32_t duty_cycle)
//{
//    if(!VALID_ID(id))
//        return;
    
//    ECG_LED_Enable(id);
    
//    led_config[id].int_count = ((period * TIMER_INTS_PER_MSEC) * duty_cycle) / 100;
//    led_config[id].period = period;
//    led_config[id].duty_cycle = duty_cycle;
    
//    TimerIntEnable(LED_TIMER_BASE, led_config[id].timeout);
//    TimerEnable(LED_TIMER_BASE, led_config[id].timer);
    
//} /* ECG_LED_Flash() */


/* Public - Sets an LED to a solid glow
 * 
 * id - The ID of the LED to set
 * 
 * Examples
 * 
 *      ECG_LED_Solid(ECG_LED_1);
 * 
 * Returns nothing
 */
//void ECG_LED_Solid(ECG_LED_ID_t8 id)
//{
//    if(!VALID_ID(id))
//        return;
    
//    ECG_LED_Enable(id);
//    TimerIntDisable(LED_TIMER_BASE, led_config[id].timeout);
//    TimerDisable(LED_TIMER_BASE, led_config[id].timer);
//} /* ECG_LED_Solid() */


/* Internal - Timer timeout interrupt for handling the flash frequency of LED 0.
 * 
 * Returns nothing
 */
//static void led0_timer_handler(void)
//{
//    /* Clear the interrupt */
//    TimerIntClear(LED_TIMER_BASE, led_config[ECG_LED_0].timeout);

//    if(--(led_config[ECG_LED_0].int_count) == 0)
//    {
//        if(led_config[ECG_LED_0].enabled == 1)
//        {
//            GPIOPinWrite(led_config[ECG_LED_0].gpio_base, led_config[ECG_LED_0].pin, ~led_config[ECG_LED_0].pin);
//            led_config[ECG_LED_0].enabled = 0;
//            led_config[ECG_LED_0].int_count = (led_config[ECG_LED_0].period * TIMER_INTS_PER_MSEC * (100 - led_config[ECG_LED_0].duty_cycle)) / 100;
//        }
//        else
//        {
//            GPIOPinWrite(led_config[ECG_LED_0].gpio_base, led_config[ECG_LED_0].pin, led_config[ECG_LED_0].pin);
//            led_config[ECG_LED_0].enabled = 1;
//            led_config[ECG_LED_0].int_count = (led_config[ECG_LED_0].period * TIMER_INTS_PER_MSEC * led_config[ECG_LED_0].duty_cycle) / 100;
//        }
//    }
//} /* led0_timer_handler() */