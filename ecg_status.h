#ifndef __ECG_STATUS_H__
#define __ECG_STATUS_H__

typedef enum ECG_Status_ID_Enum_t8
{
    /* Mutually Exclusive Status Bits */
    ECG_Status_Off = 0x01,
    ECG_Status_Normal = 0x02,
    ECG_Status_Error = 0x04,
    ECG_Status_POST = 0x08,
    ECG_Status_Discoverable = 0x10,
    ECG_Status_Data_Capture = 0x20,
    ECG_Status_Flash = 0x40,
    
    /* Should always be highest status */
    ECG_Status_Low_Battery = 0x80
} ECG_Status_ID_t8;

void ECG_Status_Init(void);
void ECG_Status_Set(ECG_Status_ID_t8 id);
void ECG_Status_Clear(ECG_Status_ID_t8 id);
uint8_t ECG_Status_Check(ECG_Status_ID_t8 id);

#endif