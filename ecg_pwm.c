#include "sapphire_pub.h"

#include "pin_map.h"
#include "BKRNLAPI.h"
#include "ecg_gpio.h"
#include "ecg_pwm.h"
#include "gpio.h"
#include "driverlib/pwm.h"
#include "sysctl.h"

#define SPEAKER_PWM_DEFAULT_PERIOD          8000
#define SPEAKER_PWM_DEFAULT_DUTY_CYCLE      4000
#define LED_DEFAULT_PERIOD                  8000
#define LED_DEFAULT_DUTY_CYCLE              7999

static const ECG_Pwm_t pwm_default_config[] =
{
    {
        ECG_Pwm_Spk_0,
        GPIO_PIN_1,
        GPIO_PORTG_BASE,
        PWM_GEN_2,
        PWM_OUT_5,
        PWM_OUT_5_BIT,
        GPIO_PG1_M0PWM5,
        SPEAKER_PWM_DEFAULT_PERIOD,
        SPEAKER_PWM_DEFAULT_DUTY_CYCLE,
        false
    },
    {
        ECG_Pwm_Led_0,
        GPIO_PIN_0,
        GPIO_PORTF_BASE,
        PWM_GEN_0,
        PWM_OUT_0,
        PWM_OUT_0_BIT,
        //GPIO_PF0_PWM0,
        GPIO_PF0_M0PWM0,
        LED_DEFAULT_PERIOD,
        LED_DEFAULT_DUTY_CYCLE,
        false
    },
    {
        ECG_Pwm_Led_1,
        GPIO_PIN_1,
        GPIO_PORTF_BASE,
        PWM_GEN_0,
        PWM_OUT_1,
        PWM_OUT_1_BIT,
//        GPIO_PA7_PWM1,
		GPIO_PF1_M0PWM1,
        LED_DEFAULT_PERIOD,
        LED_DEFAULT_DUTY_CYCLE,
        false
    },
    {
        ECG_Pwm_Led_2,
        GPIO_PIN_2,
        GPIO_PORTF_BASE,
        PWM_GEN_1,
        PWM_OUT_2,
        PWM_OUT_2_BIT,
        //GPIO_PG1_PWM5,
        GPIO_PF2_M0PWM2,
        LED_DEFAULT_PERIOD,
        LED_DEFAULT_DUTY_CYCLE,
        false
    }
};

ECG_Pwm_t pwm_config[ECG_Pwm_Count];


/* Public - Initializes the PWM
 * 
 * Examples
 * 
 *      ECG_PWM_Init();
 * 
 * Returns nothing
 */
void ECG_PWM_Init(void)
{
    uint8_t i;
    
    /* Copy default configuration to runtime configuration */
    BTPS_MemCopy(pwm_config, pwm_default_config, sizeof(ECG_Pwm_t) * ECG_Pwm_Count);
    
    /* Enable PWM */
//    PWMClockSet(PWM0_BASE, SYSCTL_PWMDIV_8);
    //SysCtlPeripheralDisable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    
    /* Set PWM Pins */
    for(i = 0; i < ECG_Pwm_Count; i++)
    {
        GPIOPinConfigure(pwm_config[i].pin_config);
        GPIOPinTypePWM(pwm_config[i].base, pwm_config[i].pin);
        PWMGenConfigure(PWM0_BASE, pwm_config[i].generator, ( PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC ));
        PWMGenPeriodSet(PWM0_BASE, pwm_config[i].generator, pwm_config[i].period);
        PWMPulseWidthSet(PWM0_BASE, pwm_config[i].output, pwm_config[i].duty_cycle);
    }

    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_8);
} /* ECG_PWM_Init() */


/* Public - Enables the pwm for the given id
 * 
 * Examples
 * 
 *      ECG_PWM_Enable(ECG_Pwm_Spk_0);
 * 
 * Returns nothing
 */
void ECG_PWM_Enable(ECG_Pwm_ID_t8 id)
{
    if(id >= ECG_Pwm_Count)
        return;
    
    PWMOutputState(PWM0_BASE, pwm_config[id].output_bit, true);
    PWMGenEnable(PWM0_BASE, pwm_config[id].generator);
} /* ECG_PWM_Enable() */


/* Public - Disables the pwm for the given id
 * 
 * Exmaples
 * 
 *      ECG_PWM_Disable(ECG_Pwm_Spk_0);
 * 
 * Returns nothing
 */
void ECG_PWM_Disable(ECG_Pwm_ID_t8 id)
{
    uint8_t i;
    uint8_t disable = 1;
    
    if(id >= ECG_Pwm_Count)
        return;
    
    PWMOutputState(PWM0_BASE, pwm_config[id].output_bit, false);
    for(i = 0; i < ECG_Pwm_Count; i++)
    {
        if(i == id)
            continue;
        
        /* If another PWM Output is sharing this generator, and it is off,
         * then turn off the generator */
        if(pwm_config[i].generator == pwm_config[id].generator &&
            pwm_config[i].output_state == true)
            disable = 0;
    }
    
    if(disable == 1)
        PWMGenDisable(PWM0_BASE, pwm_config[id].generator);
} /* ECG_PWM_Disable() */


/* Public - Retrieves the period and duty cycle of the pwm matching the provided id.
 * 
 * Examples
 * 
 *      uint32_t period;
 *      uint32_t duty;
 * 
 *      ECG_PWM_Get_Period_Duty_Cycle(ECG_Pwm_Spk_0, &period, &duty);
 * 
 * Returns the period and duty cycle for the given pwm.
 */
void ECG_PWM_Get_Period_Duty_Cycle(ECG_Pwm_ID_t8 id, uint32_t *period, uint32_t *duty_cycle)
{
    if(id >= ECG_Pwm_Count ||
        period == 0 ||
        duty_cycle == 0)
        return;
        
    *period = pwm_config[id].period;
    *duty_cycle = pwm_config[id].duty_cycle;
} /* ECG_PWM_Get_Period_Duty_Cycle() */


/* Public - Sets the period and duty cycle for the pwm matching the provided id.
 * 
 * Examples
 * 
 *      uint32_t period = 20000;
 *      uint32_t duty = 50;
 * 
 *      ECG_PWM_Set_Period_Duty_Cycle(ECG_Pwm_Spk_0, period, duty);
 * 
 * Returns nothing
 */
void ECG_PWM_Set_Period_Duty_Cycle(ECG_Pwm_ID_t8 id, uint32_t period, uint32_t duty_cycle)
{
    if(id >= ECG_Pwm_Count)
        return;
        
    pwm_config[id].period = period;
    pwm_config[id].duty_cycle = duty_cycle;
    
    PWMGenPeriodSet(PWM0_BASE, pwm_config[id].generator, pwm_config[id].period);
    PWMPulseWidthSet(PWM0_BASE, pwm_config[id].output, pwm_config[id].duty_cycle);
    PWMGenEnable(PWM0_BASE, pwm_config[id].generator);
} /* ECG_PWM_Set_Period_Duty_Cycle() */
