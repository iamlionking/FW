#ifndef __ADC_SSI_H__
#define __ADC_SSI_H__

#define ADS_SSI_UDMA_CHANNEL_RX				UDMA_CHANNEL_SSI0RX
#define ADS_SSI_UDMA_CHANNEL_TX             UDMA_CHANNEL_SSI0TX
#define ADS_SSI_UDMA_RX_CHANNEL_CONTROL		(UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 | UDMA_ARB_4)
#define ADS_SSI_UDMA_TX_CHANNEL_CONTROL     (UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_4)
#define ADS_SSI_BUS							SSI0_BASE

#define ADS_SSI_CLOCK_RATE		80000000 	//80 MHz System Clock
#define ADS_SSI_BIT_RATE		1000000	    //1 mbps
#define ADS_SSI_FRAME_SIZE      8

typedef enum
{
    ADS_SSI_DMA_Data_Capture,
    ADS_SSI_DMA_ADS_Command
} ADS_SSI_DMA_Transfer_Type_t8;

typedef struct
{
    uint8_t size;
    ADS_SSI_DMA_Transfer_Type_t8 type;
    uint8_t use_burst;
    uint8_t *tx_ptr;
    uint8_t *rx_ptr;
} ADS_SSI_DMA_Transfer_Settings_t;

void ADS_DMA_Init();
void ADS_SSI_DMAExecuteReadWrite(ADS_SSI_DMA_Transfer_Settings_t *settings);
uint8_t ADS_SSI_GetDMAStatus(void);
void ads_ssi_transfer_complete(void);

#endif