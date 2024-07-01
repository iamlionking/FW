#ifndef __ECG_DATA_CAPTURE_H__
#define __ECG_DATA_CAPTURE_H__

#define MAX_PACKET_NUM              4096

typedef enum Data_Capture_Rate_Enum_t
{
    Data_Capture_Rate_2000_SPS = 0,         //.5ms per sample
    Data_Capture_Rate_1000_SPS = 1,         //1ms per sample
    Data_Capture_Rate_500_SPS = 2,          //2ms per sample
    Data_Capture_Rate_250_SPS = 4           //4ms per sample
} Data_Capture_Rate_t8;


uint8_t ECG_Data_Capture_Get_Queued_Data_Count(void);
Data_Capture_Rate_t8 ECG_Data_Capture_Get_Rate(void);
void ECG_Data_Capture_Init(void);
uint8_t ECG_Data_Capture_In_Progress(void);
void ECG_Data_Capture_Start(Data_Capture_Rate_t8 data_rate);
void ECG_Data_Capture_Stop(void);
void ECG_Data_Capture_Hard_Stop(void);
void ECG_Data_Capture_Queue_Data(void);
void ECG_Data_Capture_Restart(void);

#endif