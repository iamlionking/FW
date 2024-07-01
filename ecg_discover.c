#include "sapphire_pub.h"

#include "BKRNLAPI.h"
#include "bluetooth.h"
#include "ecg_discover.h"
#include "ecg_status.h"
#include "GAPAPI.h"

static uint8_t discover_state;
static uint32_t discover_stop_tick;
static uint8_t error_before_discoverable;


/* Public - Initializes the dicoverability driver.
 * 
 * Examples
 * 
 *      ECG_Discover_Init();
 * 
 * Returns nothing.
 */
void ECG_Discover_Init(void)
{
    discover_state = ECG_NON_DISCOVERABLE_STATE;
    discover_stop_tick = 0;
	error_before_discoverable = 0;
}


/* Public - Places the device in discoverable mode for the specified duration in seconds.
 * 
 * duration_sec - The number of seconds for the device to be discoverable.
 * 
 * Examples
 * 
 *      //Make the device discoverable for 60 seconds
 *      ECG_Discover_Start(60);
 * 
 * Returns nothing
 */
void ECG_Discover_Start(uint32_t duration_sec)
{
    discover_stop_tick = BTPS_GetTickCount() + duration_sec;
    GAP_Set_Discoverability_Mode(g_uiBluetoothStackID, dmGeneralDiscoverableMode, duration_sec);
    
	error_before_discoverable = ECG_Status_Check(ECG_Status_Error);
    ECG_Status_Set(ECG_Status_Discoverable);
    discover_state = ECG_DISCOVERABLE_STATE;
} /* ECG_Discover_Start() */


/* Public - places the device in non-discoverable mode.
 * 
 * Examples
 * 
 *      ECG_Discover_Stop();
 * 
 * Returns nothing
 */
void ECG_Discover_Stop(void)
{
    discover_stop_tick = 0;
    GAP_Set_Discoverability_Mode(g_uiBluetoothStackID, dmNonDiscoverableMode, 0);

	if(error_before_discoverable)
	{
		ECG_Status_Set(ECG_Status_Error);
	}
	else
	{
		ECG_Status_Set(ECG_Status_Normal);
	}

	error_before_discoverable = 0;

    discover_state = ECG_NON_DISCOVERABLE_STATE;
} /* ECG_Discover_Stop() */
