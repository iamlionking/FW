#ifndef __ECG_SPEAKER_H__
#define __ECG_SPEAKER_H__

#define SPEAKER_OFF             0
#define SPEAKER_ON              1

void ECG_Speaker_Init(void);
void ECG_Speaker_Enable(uint32_t duration);
void ECG_Speaker_Disable(void);
uint8_t ECG_Speaker_Get_State(void);
void ECG_Speaker_Off_Check(void);

#endif