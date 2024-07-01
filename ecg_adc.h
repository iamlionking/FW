#ifndef __ECG_ADC_H__
#define __ECG_ADC_H__

#define ECG_ADC             ADC0_BASE
#define ECG_ADC_SEQUENCE    0
#define ECG_ADC_5V0_SEQ     3
#define ECG_ADC_OVERSAMPLE  16
#define ECG_ADC_BITMASK     0x0fff
//#define ECG_ADC_BITMASK     0x03ff

typedef enum ECG_Adc_ID_enum_t8
{
    ECG_Adc_1V8,                //AN7
    ECG_Adc_5V0,                //AN6
    ECG_Adc_External_5V,        //AN3
    ECG_Adc_Battery_Voltage,    //AN10
    ECG_Adc_VCore,              //AN13
    
    ECG_Adc_Count
} ECG_Adc_ID_t8;

typedef struct ECG_Adc_struct_t
{
    ECG_Adc_ID_t8   id;
    uint8_t         pin;
    uint32_t        base;
    uint8_t         channel;
    uint16_t        step_size;
    int16_t         offset;
} ECG_Adc_t;

void ECG_ADC_Init(void);
void ECG_ADC_Enable(uint8_t use_int);
void ECG_ADC_GetName(ECG_Adc_ID_t8 id, uint8_t *name_buf);
void ECG_ADC_GetValue(ECG_Adc_ID_t8 id, uint16_t *value);
void ECG_ADC_GetRaw(ECG_Adc_ID_t8 id, uint16_t *value);

#endif
