#ifndef __ECG_BATTERY_H__
#define __ECG_BATTYER_H__

typedef enum Battery_Charge_Status_Enum_t8
{
    Battery_Charge_Low = 0x01,
    Battery_Charge_High = 0x02,
    Battery_Charge_Full = 0x04,
	Battery_Charge_Unknown = 0x08
} Battery_Charge_Status_t8;

typedef struct battery_eqn_struct
{
    int32_t m;
    int32_t b;
} battery_eqn_t;

void ECG_Battery_Init(void);
void ECG_Battery_Charger_Connect(void);
void ECG_Battery_Charger_Disconnect(void);
void ECG_Battery_Get_Level(uint32_t *level);
void ECG_Battery_Get_Status(uint8_t *status);
void ECG_Battery_Set_Charge_Status(Battery_Charge_Status_t8 status);
 
#endif