#ifndef __ECG_SSI_H__
#define __ECG_SSI_H__

#define SSI_BUS             SSI0_BASE
#define SSI_BIT_RATE		2000000	    //2 mbps
//#define SSS_ADS_BIT_RATE    5000000    //10 mbps
#define SSS_ADS_BIT_RATE    1000000    //2 mbps
//#define SSS_ADS_BIT_RATE    600000

#define SSI_FRAME_SIZE      8

typedef enum
{
    SSI_Unknown_Mode,
    SSI_ADS_Mode,
    SSI_Flash_Mode
} ECG_SSI_Mode_t8;

ECG_SSI_Mode_t8 ECG_SSI_Get_Mode(void);
void ECG_SSI_Init(void);
void ECG_SSI_Set_Mode(ECG_SSI_Mode_t8 mode);

#endif
