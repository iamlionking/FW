#ifndef __ECG_LED_H__
#define __ECG_LED_H__

#include "ecg_pwm.h"

/* Hardware configuration for the LEDs */
/* Note: At this time, they may not need to be PWM'd */
typedef enum ECG_LED_ID_enum_t8
{
    ECG_LED_0,
    ECG_LED_1,
    ECG_LED_2,
    
    ECG_LED_COUNT
} ECG_LED_ID_t8;

typedef struct ECG_LED_struct_t
{
    ECG_LED_ID_t8   id;
    ECG_Pwm_ID_t8   pwm;
    uint8_t         enabled;
    uint8_t         pin;
    uint32_t        gpio_base;
} ECG_LED_t;

#define VALID_ID(id)    (id < ECG_LED_COUNT)

void ECG_LED_Init(void);
void ECG_LED_Enable(ECG_LED_ID_t8 id);
void ECG_LED_Disable(ECG_LED_ID_t8 id);
uint8_t ECG_LED_Get_State(ECG_LED_ID_t8 id);
void ECG_LED_Flash(ECG_LED_ID_t8 id, uint32_t period, uint32_t duty_cycle);
void ECG_LED_Solid(ECG_LED_ID_t8 id);

#endif