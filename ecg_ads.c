#include "sapphire_pub.h"

#include "inc/hw_gpio.h"
#include "inc/hw_ssi.h"
#include "gpio.h"
#include "ssi.h"
#include "sysctl.h"
#include "udma.h"
#include "interrupt.h"

#include "ecg_ads.h"
#include "ecg_data_capture.h"
#include "ecg_ssi.h"

unsigned char ucControlTable[1024] __attribute__ ((aligned(1024)));

static uint8_t dma_status;
static uint8_t dma_transfer_type;
static uint8_t dma_tx_finished;
static uint8_t dma_rx_finished;

void ads_SSI_DMARead(uint8_t *data_ptr, uint8_t transfer_size);
void ads_SSI_DMAWrite(uint8_t *data_ptr, uint8_t transfer_size);
void ads_SSI_DMAStart(void);

void ADS_DMA_Init()
{
    /* Enable uDMA Peripheral */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    
    /* Enable uDMA */
	uDMAEnable();
	uDMAControlBaseSet(ucControlTable);
	
	/* Set SSI uDMA channel to High Priority */
	uDMAChannelAttributeDisable(ADS_SSI_UDMA_CHANNEL_RX, (UDMA_ATTR_USEBURST| UDMA_ATTR_ALTSELECT | UDMA_ATTR_REQMASK | UDMA_ATTR_HIGH_PRIORITY));
    uDMAChannelAttributeDisable(ADS_SSI_UDMA_CHANNEL_TX, (UDMA_ATTR_USEBURST| UDMA_ATTR_ALTSELECT | UDMA_ATTR_REQMASK | UDMA_ATTR_HIGH_PRIORITY));
	
	/* Set Control parameters for SSI uDMA */
	uDMAChannelControlSet((ADS_SSI_UDMA_CHANNEL_RX | UDMA_PRI_SELECT), ADS_SSI_UDMA_RX_CHANNEL_CONTROL);
    uDMAChannelControlSet((ADS_SSI_UDMA_CHANNEL_TX | UDMA_PRI_SELECT), ADS_SSI_UDMA_TX_CHANNEL_CONTROL);
} /* ADC_SPI_Init() */


void ADS_SSI_DMAExecuteReadWrite(ADS_SSI_DMA_Transfer_Settings_t *settings)
{
    dma_transfer_type = settings->type;
    if(settings->use_burst)
    {
       /* Enable Burst Mode */
        uDMAChannelAttributeEnable(ADS_SSI_UDMA_CHANNEL_RX, UDMA_ATTR_USEBURST);
        uDMAChannelAttributeEnable(ADS_SSI_UDMA_CHANNEL_TX, UDMA_ATTR_USEBURST); 
    }
    else
    {
        /* Disable Burst Mode */
        uDMAChannelAttributeDisable(ADS_SSI_UDMA_CHANNEL_RX, UDMA_ATTR_USEBURST);
        uDMAChannelAttributeDisable(ADS_SSI_UDMA_CHANNEL_TX, UDMA_ATTR_USEBURST);
    }
    /* Set up transaction */
    ads_SSI_DMARead(settings->rx_ptr, settings->size);
    ads_SSI_DMAWrite(settings->tx_ptr, settings->size);
    
    /* Start DMA Transaction */
    ads_SSI_DMAStart();
} /* ADS_SSI_DMAExecuteReadWrite() */


uint8_t ADS_SSI_GetDMAStatus(void)
{
    return (dma_tx_finished && dma_rx_finished);
} /* ADS_SSI_GetDMAStatus() */


void ads_SSI_DMARead(uint8_t *data_ptr, uint8_t transfer_size)
{
	uDMAChannelTransferSet((ADS_SSI_UDMA_CHANNEL_RX | UDMA_PRI_SELECT), UDMA_MODE_BASIC, (void *)(SSI0_BASE + SSI_O_DR), data_ptr, transfer_size);
} /* ads_SSI_DMARead() */


void ads_SSI_DMAWrite(uint8_t *data_ptr, uint8_t transfer_size)
{
    uDMAChannelTransferSet((ADS_SSI_UDMA_CHANNEL_TX | UDMA_PRI_SELECT), UDMA_MODE_BASIC, data_ptr, (void *)(SSI0_BASE + SSI_O_DR), transfer_size);
} /* ads_SSI_DMAWrite() */


void ads_SSI_DMAStart()
{
    dma_status = 2;
    dma_tx_finished = 0;
    dma_rx_finished = 0;
    
    /* Enable DMA interface to SSI Bus */
    IntEnable(INT_SSI0);
	SSIDMAEnable(ADS_SSI_BUS, (SSI_DMA_TX | SSI_DMA_RX));
    
    /* Enable uDMA SSI Channels */
    uDMAChannelEnable(ADS_SSI_UDMA_CHANNEL_TX);
	uDMAChannelEnable(ADS_SSI_UDMA_CHANNEL_RX);
} /* ads_SSI_DMAStart() */


void ads_ssi_transfer_complete()
{
    uint32_t int_status;
    uint32_t dma_mode;
    
    int_status = SSIIntStatus(ADS_SSI_BUS, 1);
    SSIIntClear(ADS_SSI_BUS, int_status);
    
    dma_mode = uDMAChannelModeGet(ADS_SSI_UDMA_CHANNEL_RX);
    if(dma_mode == UDMA_MODE_STOP)
        dma_rx_finished = 1;
    
    dma_mode = uDMAChannelModeGet(ADS_SSI_UDMA_CHANNEL_TX);
    if(dma_mode == UDMA_MODE_STOP)
        dma_tx_finished = 1;
        
    
    if(dma_transfer_type == ADS_SSI_DMA_Data_Capture && dma_rx_finished && dma_tx_finished)
    {
        /* Call ECG Data Capture function to put data into transport queue */
        ECG_Data_Capture_Queue_Data();
    }
    
    if(dma_tx_finished && dma_rx_finished)
    {
        /* Disable DMA interface to SSI Bus */
        IntDisable(INT_SSI0);
        SSIDMADisable(ADS_SSI_BUS, (SSI_DMA_TX | SSI_DMA_RX));
    }
}