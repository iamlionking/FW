#include "sapphire_pub.h"

#include "adc.h"
#include "gpio.h"
#include "ecg_adc.h"
#include "ecg_battery.h"
#include "ecg_gpio.h"
#include "ecg_power_ctl.h"
#include "interrupt.h"
#include "sysctl.h"

#define ADC_CHANNEL_COUNT       8

static const ECG_Adc_t adc_config[] =
{
    {
        ECG_Adc_1V8,                //AN7
        GPIO_PIN_7,
        GPIO_PORTD_BASE,
        ADC_CTL_CH4,
        1,
        0
    },
    {
        ECG_Adc_5V0,                //AN6
        GPIO_PIN_4,
        GPIO_PORTE_BASE,
        ADC_CTL_CH9,
        2,
        0
    },
    {
        ECG_Adc_External_5V,        //AN3
        GPIO_PIN_0,
        GPIO_PORTD_BASE,
        ADC_CTL_CH15,
        2,
        0
    },
    {
        ECG_Adc_Battery_Voltage,    //AN10
        GPIO_PIN_4,
        GPIO_PORTB_BASE,
        ADC_CTL_CH10,
        2,
        0
    },
    {
        ECG_Adc_VCore,              //AN13
        GPIO_PIN_1,
        GPIO_PORTD_BASE,
        ADC_CTL_CH14,
        1,
        0
    }
};

static const ECG_Adc_t ext_5v0 =
{
    ECG_Adc_External_5V,        //AN3
    GPIO_PIN_4,
    GPIO_PORTE_BASE,
    ADC_CTL_CH3,
    2,
    0
};

static const char* adc_text_name[] =
{
    "1.8V Supply",
    "5.0V Supply",
    "External 5V",
    "Battery Voltage",
    "VCore"
};

static uint8_t int_done;
static uint32_t adc_data[ADC_CHANNEL_COUNT];
static uint32_t interrupt_5v0;

static void adc_sequence_done(void);
static void adc_start(uint8_t use_int);
static void external_voltage_int(void);


/* Public - Initialize ECG specific ADC channels
 * 
 * Examples
 * 
 *      ECG_ADC_Init();
 * 
 * Returns Nothing
 */
void ECG_ADC_Init(void)
{
    uint8_t i;
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    
    for(i = 0; i < ECG_Adc_Count; i++)
    {
        GPIOPinTypeADC(adc_config[i].base, adc_config[i].pin);
    }
    
    /* Set the Reference */
//    ADCReferenceSet(ECG_ADC, ADC_REF_INT);
    ADCReferenceSet(ECG_ADC, ADC_REF_EXT_3V);
    
    /* Disable Sequence */
    ADCSequenceDisable(ECG_ADC, ECG_ADC_SEQUENCE);
    
    /* Configure Sequence */
    ADCSequenceConfigure(ECG_ADC, ECG_ADC_SEQUENCE, ADC_TRIGGER_PROCESSOR, 0);
    
    /* Configure Sequence Steps */
    for(i = 0; i < ECG_Adc_Count - 1; i++)
    {
        ADCSequenceStepConfigure(ECG_ADC, ECG_ADC_SEQUENCE, i, adc_config[i].channel);
    }
    
    /* Configure final step to cause interrupt */
    ADCSequenceStepConfigure(ECG_ADC, ECG_ADC_SEQUENCE, i, (adc_config[i].channel | ADC_CTL_END | ADC_CTL_IE));
    
    ADCIntRegister(ECG_ADC, ECG_ADC_SEQUENCE, adc_sequence_done);
    GPIOPinWrite(GPIO_BATT_MON_PORT, GPIO_BATT_MON_PIN, GPIO_BATT_MON_PIN);
    ADCSequenceEnable(ECG_ADC, ECG_ADC_SEQUENCE);
    
    ADCSequenceDisable(ECG_ADC, ECG_ADC_5V0_SEQ);
    
    ADCSequenceConfigure(ECG_ADC, ECG_ADC_5V0_SEQ, ADC_TRIGGER_ALWAYS, 1);
    
    ADCSequenceStepConfigure(ECG_ADC, ECG_ADC_5V0_SEQ, 0, (ext_5v0.channel | ADC_CTL_END | ADC_CTL_CMP0));
    
    interrupt_5v0 = ADC_COMP_INT_HIGH_ONCE;
    ADCComparatorConfigure(ECG_ADC, 0, ADC_COMP_INT_HIGH_ALWAYS);
    ADCComparatorRegionSet(ECG_ADC, 0, 0x22, 0x2ee);
    ADCComparatorIntEnable(ECG_ADC, ECG_ADC_5V0_SEQ);
    ADCIntRegister(ECG_ADC, ECG_ADC_5V0_SEQ, external_voltage_int);
    ADCHardwareOversampleConfigure(ECG_ADC, 16);
    ADCSequenceEnable(ECG_ADC, ECG_ADC_5V0_SEQ);
} /* ECG_ADC_Init() */


/* Public - Enables ECG specific ADC conversions.  The conversions are executed continuously by using a timer
 * that is configured by this function.
 * 
 * Examples
 * 
 *      ECG_ADC_Enable();
 * 
 * Returs nothing.
 */
void ECG_ADC_Enable(uint8_t use_int)
{
    int_done = 0;
    adc_start(use_int);
} /* ECG_ADC_Enable() */


/* Public - Provides an interface to get the name of an ECG ADC channel as a string.
 * 
 * id - The ID of the ADC channel
 * name - A pointer to a uint8_t array that will contain the name of the ADC channel.
 * 
 * Examples
 * 
 *      uint8_t name[20];
 *      ECG_ADC_GetName(ECG_Adc_VCore, &name);
 *      Display("%s\r\n", name);
 * 
 * Returns nothing
 */
void ECG_ADC_GetName(ECG_Adc_ID_t8 id, uint8_t *name_buf)
{
    if(id >= ECG_Adc_Count)
    {
        return;
    }
    BTPS_StringCopy((char*)name_buf, adc_text_name[id]);
} /* ECG_ADC_GetName() */


/* Public - Retrieves the most recent conversion value for the specified channel.
 * 
 * id - The ADC channel
 * value - A uint16_t pointer for storing the ADC channel conversion
 * 
 * Examples
 * 
 *      uint16_t adc_val;
 *      ECG_ADC_GetValue(ECG_Adc_VCore, &adc_val);
 * 
 * Returns nothing
 */
void ECG_ADC_GetValue(ECG_Adc_ID_t8 id, uint16_t *value)
{
    bool int_status;
    
    if(id >= ECG_Adc_Count)
    {
        *value = 0xffff;
        return;
    }
    
    int_status = IntMasterDisable();
    *value = adc_data[id];
    if(!int_status)
        IntMasterEnable();
        
    *value = ((((*value & ECG_ADC_BITMASK) * 3000) / (ECG_ADC_BITMASK + 1)) * adc_config[id].step_size) + adc_config[id].offset;
    
    //*value = adc_data[id] & ECG_ADC_BITMASK;
} /* ECG_ADC_GetValue() */


void ECG_ADC_GetRaw(ECG_Adc_ID_t8 id, uint16_t *value)
{
    if(id >= ECG_Adc_Count)
    {
        *value = 0xffff;
        return;
    }
    
    *value = adc_data[id];
}


/* Internal - Handles the interrupt generated when the ADC conversion sequence finishes.
 * 
 * Returns nothing
 */
void adc_sequence_done(void)
{
    //volatile uint16_t i;
    ADCIntClear(ECG_ADC, ECG_ADC_SEQUENCE);
    ADCIntDisable(ECG_ADC, ECG_ADC_SEQUENCE);
    //ADCSequenceDisable(ECG_ADC, ECG_ADC_SEQUENCE);
    
    ADCSequenceDataGet(ECG_ADC, ECG_ADC_SEQUENCE, adc_data);
    //for(i = 0; i < 200; i++);
    //GPIOPinWrite(GPIO_BATT_MON_PORT, GPIO_BATT_MON_PIN, ~GPIO_BATT_MON_PIN);
} /* adc_sequence_done() */


/* Internal - Called as a scheduled function from the Bluetooth Stack scheduler.  
 * Handles starting a new ADC conversion sequence.
 * 
 * Returns nothing
 */
static void adc_start(uint8_t use_int)
{
    ADCIntEnable(ECG_ADC, ECG_ADC_SEQUENCE);
    //GPIOPinWrite(GPIO_BATT_MON_PORT, GPIO_BATT_MON_PIN, GPIO_BATT_MON_PIN);
    ADCProcessorTrigger(ECG_ADC, ECG_ADC_SEQUENCE);
} /* adc_start() */


static void external_voltage_int(void)
{
    uint32_t status = ADCComparatorIntStatus(ECG_ADC);
    ADCComparatorIntClear(ECG_ADC, status);
    ADCComparatorIntDisable(ECG_ADC, ECG_ADC_5V0_SEQ);
    
    if(interrupt_5v0 == ADC_COMP_INT_HIGH_ONCE)
    {
        interrupt_5v0 = ADC_COMP_INT_LOW_ONCE;
        ADCComparatorConfigure(ECG_ADC, 0, interrupt_5v0);
        ECG_Power_Set_External_Connection(1);

#ifdef ECG_HARDWARE_BATTERY
        ECG_Battery_Charger_Connect();
#endif
    }
    else
    {
        interrupt_5v0 = ADC_COMP_INT_HIGH_ONCE;
        ADCComparatorConfigure(ECG_ADC, 0, interrupt_5v0);
        ECG_Power_Set_External_Connection(0);

#ifdef ECG_HARDWARE_BATTERY
        ECG_Battery_Charger_Disconnect();
#endif
    }
    
    ADCComparatorIntEnable(ECG_ADC, ECG_ADC_5V0_SEQ);
} /* external_voltage_int() */
