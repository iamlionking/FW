#include "sapphire_pub.h"

#include "pin_map.h"
#include "ecg_ssi.h"
#include "gpio.h"
#include "interrupt.h"
#include "ssi.h"
#include "sysctl.h"

static ECG_SSI_Mode_t8 current_mode;

static void set_ads_mode(void);
static void set_flash_mode(void);


/* Public - Gets the current SSI mode.
 * 
 * Examples
 * 
 *      ECG_SSI_Mode_t8 ssi_mode;
 * 
 *      ssi_mode = ECG_SSI_Get_Mode();
 *      if(ssi_mode == SSI_Flash_Mode)
 *          ECG_SSI_Set_Mode(SSI_ADS_Mode);
 * 
 * Returns the current SSI mode.
 */
ECG_SSI_Mode_t8 ECG_SSI_Get_Mode(void)
{
    return current_mode;
} /* ECG_SSI_Get_Mode() */


/* Public - Initializes the SSI configuration driver.
 * 
 * Examples
 * 
 *      ECG_SSI_Init();
 * 
 * Returns nothing.
 */
void ECG_SSI_Init(void)
{
    current_mode = SSI_Unknown_Mode;
    
    //Enable SSI0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
	
    /* Enable SSI0 Pins */
	//NOTE: The slave select pin is controlled by the communication with the ADS and is left not enabled for SSI
    //This is also true of the external flash chip.
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA4_SSI0XDAT0);
	GPIOPinConfigure(GPIO_PA5_SSI0XDAT1);
	
	/* Configure GPIO pins for SSI, gives control of pins to SSI hardware */
	GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_2);
} /* ECG_SSI_Init() */


/* Public - Sets the current SSI mode.
 * 
 * Examples
 * 
 *      //Configure to Flash mode
 *      if(ECG_SSI_Get_Mode() == SSI_ADS_Mode)
 *          ECG_SSI_Set_Mode(SSI_Flash_Mode);
 * 
 * Returns nothing
 */
void ECG_SSI_Set_Mode(ECG_SSI_Mode_t8 mode)
{
    SSIDisable(SSI_BUS);
    
    if(mode == SSI_ADS_Mode)
    {
        set_ads_mode();
    }
    else if(mode == SSI_Flash_Mode)
    {
        set_flash_mode();
    }
    
    SSIEnable(SSI_BUS);
} /* ECG_SSI_Set_Mode() */


/* Internal - Sets up the SSI interface to talk to the ADS chip.
 * 
 * Returns nothing
 */
static void set_ads_mode(void)
{
	/* Set SSI Parameters */
	SSIConfigSetExpClk(SSI_BUS, g_ui32ClockFrequency, SSI_FRF_MOTO_MODE_1, SSI_MODE_MASTER, SSS_ADS_BIT_RATE, SSI_FRAME_SIZE);
    
    current_mode = SSI_ADS_Mode;
} /* set_ads_mode() */


/* Internal - Sets up the SSI interface to talk to the external flash chip.
 * 
 * Returns nothing
 */
static void set_flash_mode(void)
{
	/* Set SSI Parameters */
	SSIConfigSetExpClk(SSI_BUS, g_ui32ClockFrequency, SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, SSI_BIT_RATE, SSI_FRAME_SIZE);
    
    current_mode = SSI_Flash_Mode;
} /* set_flash_mode() */
