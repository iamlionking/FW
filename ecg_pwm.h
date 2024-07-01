#ifndef __ECG_PWM_H__
#define __ECG_PWM_H__               

typedef enum ECG_Pwm_ID_enum_t8
{
    ECG_Pwm_Spk_0,
    ECG_Pwm_Led_0,
    ECG_Pwm_Led_1,
    ECG_Pwm_Led_2,
    
    ECG_Pwm_Count
}ECG_Pwm_ID_t8;

typedef struct ECG_Pwm_struct_t
{
    ECG_Pwm_ID_t8   id;
    uint8_t         pin;
    uint32_t        base;
    uint16_t        generator;
    uint16_t        output;
    uint8_t         output_bit;
    uint32_t        pin_config;
    uint32_t        period;
    uint32_t        duty_cycle;
    uint8_t         output_state;
} ECG_Pwm_t;

void ECG_PWM_Init(void);
void ECG_PWM_Enable(ECG_Pwm_ID_t8 id);
void ECG_PWM_Disable(ECG_Pwm_ID_t8 id);
void ECG_PWM_Set_Period_Duty_Cycle(ECG_Pwm_ID_t8 id, uint32_t period, uint32_t duty_cycle);
void ECG_PWM_Get_Period_Duty_Cycle(ECG_Pwm_ID_t8 id, uint32_t *period, uint32_t *duty_cycle);

#endif