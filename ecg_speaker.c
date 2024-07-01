#include "sapphire_pub.h"

#include "BKRNLAPI.h"
#include "ecg_gpio.h"
#include "ecg_pwm.h"
#include "ecg_speaker.h"
#include "gpio.h"

#define PWM_SPEAKER_COUNT       3
#define SPEAKER_ON_TICKS        1500
#define SPEAKER_AUDIBLE_OFFSET  0

static const ECG_Pwm_ID_t8 speaker_pwm_config[] =
{
    ECG_Pwm_Spk_0
};

static uint32_t duration;
static uint8_t speaker_state;
static uint32_t start_tick;


/* Public - Initializes the speaker state variables
 * 
 * Examples
 * 
 *      ECG_Speaker_Init();
 * 
 * Returns nothing
 */
void ECG_Speaker_Init(void)
{
    duration = 0;
    speaker_state = SPEAKER_OFF;
    start_tick = 0;
} /* ECG_Speaker_Init() */


/* Public - Enables the speaker for the sepcified duration
 * 
 * Examples
 * 
 *      ECG_Speaker(3000);  //Enable speaker for 3 seconds
 * 
 * Returns nothing.
 */
void ECG_Speaker_Enable(uint32_t duration_ms)
{
    uint8_t i;
    
    GPIOPinWrite(GPIO_BUZZ_AMP_PORT, GPIO_BUZZ_AMP_PIN, GPIO_BUZZ_AMP_PIN);
    
    for(i = 0; i < PWM_SPEAKER_COUNT; i++)
    {
        ECG_PWM_Enable(speaker_pwm_config[i]);
    }
    
    speaker_state = SPEAKER_ON;
    start_tick = BTPS_GetTickCount();
    duration = duration_ms + SPEAKER_AUDIBLE_OFFSET;
} /* ECG_Speaker_Enable() */


/* Public - Disable the speaker
 * 
 * Examples
 * 
 *      ECG_Speaker_Disable();  //Turn off the speaker
 * 
 * Returns nothing.
 */
void ECG_Speaker_Disable(void)
{
    uint8_t i;
    
    for(i = 0; i < PWM_SPEAKER_COUNT; i++)
    {
        ECG_PWM_Disable(speaker_pwm_config[i]);
    }
   
    GPIOPinWrite(GPIO_BUZZ_AMP_PORT, GPIO_BUZZ_AMP_PIN, ~GPIO_BUZZ_AMP_PIN);
    
    speaker_state = SPEAKER_OFF;
} /* ECG_Speaker_Disable() */


/* Public - Gets the current state of the speaker
 * 
 * Examples
 * 
 *      uint8_t speaker_state;
 *      speaker_state = ECG_Speaker_Get_State();
 *      if(speaker_state == SPEAKER_OFF)
 *          //do something about it
 * 
 * Returns the current state of the speaker.
 */
uint8_t ECG_Speaker_Get_State(void)
{
     return speaker_state;
} /* ECG_Speaker_Get_State() */


/* Public - Checks to see if the speaker on duration has passed and disables
 * the speaker if necessary.
 * 
 * Examples
 * 
 *      while(ECG_Get_Speaker_State() == SPEAKER_ON)
 *          ECG_Speaker_Off_Check(); //Check if duration has passed
 * 
 * Returns nothing
 */
void ECG_Speaker_Off_Check(void)
{
    uint32_t current_tick;
    uint32_t check_val;
    
    current_tick = BTPS_GetTickCount();
    
    /* Check for overflow */
    if(current_tick < start_tick)
    {
        check_val = (UINT32_MAX - start_tick) + current_tick;
    }
    else
        check_val = current_tick - start_tick;
    
    if(check_val < duration)
        return;
    
    ECG_Speaker_Disable();
} /* ECG_Speaker_Off_Check() */