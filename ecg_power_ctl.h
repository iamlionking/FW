#ifndef __ECG_POWER_CTL_H__
#define __ECG_POWER_CTL_H__

typedef enum ECG_Power_Supply_enum_t8
{
    ECG_Power_Supply_1V8,
    ECG_Power_Supply_3V3,
    ECG_Power_Supply_5V0,
    
    ECG_Power_Supply_Count
} ECG_Power_Supply_ID_t8;

typedef struct
{
    ECG_Power_Supply_ID_t8  supply;
    uint32_t                base;
    uint8_t                 pin;
} ECG_Power_Supply_t;

#define POWER_SUPPLY_ENABLE_BASE          GPIO_PORTF_BASE

void ECG_Power_Check_Low_Battery_Power_Down(void);
void ECG_Power_Init();
void ECG_Power_Poweron();
void ECG_Power_Supply_Enable(ECG_Power_Supply_ID_t8 power_supply);
void ECG_Power_Supply_Disable(ECG_Power_Supply_ID_t8 power_supply);
void ECG_Power_Off();
void ECG_Power_Off_Check();
void ECG_Power_Reset_Off_Timer();
void ECG_Power_Update_Charge_LED();
void ECG_Power_Set_External_Connection(uint8_t state);
uint8_t ECG_Power_Get_External_Connection_State(void);

#endif