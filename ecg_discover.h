#ifndef __ECG_DISCOVER_H__
#define __ECG_DISCOVER_H__

#define ECG_NON_DISCOVERABLE_STATE      0
#define ECG_DISCOVERABLE_STATE          1

void ECG_Discover_Init(void);
void ECG_Discover_Start(uint32_t duration_ms);
void ECG_Discover_Stop(void);
uint8_t ECG_Discover_Get_State(void);
uint32_t ECG_Discover_Get_Stop_Tick(void);

#endif