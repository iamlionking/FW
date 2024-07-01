#include <string.h>

#include "sapphire_pub.h"

//#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "rom.h"
#include "sysctl.h"
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include "interrupt.h"
#include "udma.h"
#include "utils/ustdlib.h"
#include "driverlib/pin_map.h"
#include "usblib/usblib.h"
#include "usblib/device/usbdevice.h"

#include "ADS1x9x.h"

#ifdef ECG_HARDWARE_BLUETOOTH
#include "bluetooth.h"
#include "ecg_discover.h"
#include "BTVSAPI.h"
#include "RFCOMAPI.h"
#include "SPPAPI.h"
#endif

#include "BSCAPI.h"
#include "ecg_adc.h"

#ifdef ECG_HARDWARE_BUTTON
#include "ecg_button.h"
#endif

#include "ecg_data_capture.h"
#include "ecg_gpio.h"
#include "ecg_init.h"
#include "ecg_pwm.h"
#include "ecg_led.h"
#include "ecg_packet.h"
#include "ecg_post.h"
#include "ecg_power_ctl.h"
#include "ecg_speaker.h"
#include "ecg_status.h"

#ifdef ECG_HARDWARE_USB
#include "ecg_usb.h"
#endif

#include "ecg_version.h"
#include "../Bootloader/ecg_boot_version.h"

#include "ecg_ssi.h"
#include "device_api.h"
#include "bt_transport.h"
#include "serial_num.h"
#include "test_transfer.h"

#include "BTPSKRNL.h"

#define MAIN_LOOP_SPEED_MS          1
#define ONE_MS_INIT        		    1 / MAIN_LOOP_SPEED_MS
#define FIVE_MS_INIT                5 / MAIN_LOOP_SPEED_MS
#define TEN_MS_INIT   	            10 / MAIN_LOOP_SPEED_MS
#define ONE_SEC_INIT           	    1000 / MAIN_LOOP_SPEED_MS
#define MAX_RADIO_POWER				12

#ifdef ECG_HARDWARE_BLUETOOTH
static char g_cBoardAddress[(SIZE_OF_BD_ADDR << 1) + 2 + 1];

bool g_toggleBTDiscoverability = false;
static bool discoverability_state = false;
#endif

static volatile unsigned long g_ulTickCount;

uint8_t g_DataRateError = 0;
uint8_t g_POSTComplete = 0;
uint8_t g_AutoTurnOn = 0;

bool g_KeepaliveEnabled = false;
uint32_t g_KeepaliveCounter = KEEPALIVE_INTERVAL;

#if defined(__ECG_DEBUG__)
uint8_t g_TestTransfer = 0;
uint32_t g_PowerOnTick;
#endif

int g_iPOSTRun = 2 * ONE_SEC_INIT;
int g_run_post = 1;
static uint8_t init_button = 1;

char *g_pcHCIVersionStrings[8] =
{
    "1.0b",
    "1.1",
    "1.2",
    "2.0",
    "2.1",
    "3.0",
    "4.0",
    "Unknown (greater 4.0)"
} ;

static uint8_t data_rate_error_cnt;

#define NUM_SUPPORTED_HCI_VERSIONS                                            \
        (sizeof(g_pcHCIVersionStrings)/sizeof(char *) - 1)

static void DisplayVersionInformation(void);

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
    Display(("Assert occured in %s at line %d\r\n", pcFilename, ulLine));
}
#endif


#ifdef ECG_HARDWARE_BLUETOOTH
//*****************************************************************************
//
// Function to format the BD_ADDR as a string.
//
//*****************************************************************************
static void
BD_ADDRToStr(unsigned char *pucBoard_Address, char *pcBoardStr)
{
    unsigned int uIdx;

    usprintf(pcBoardStr, "0x");
    for(uIdx = 0; uIdx < 6; uIdx++)
    {
        pcBoardStr += 2;
        usprintf(pcBoardStr, "%02X", pucBoard_Address[uIdx]);
    }
}

//*****************************************************************************
//
// The following function is called when various Bluetooth Events occur.  The
// function passes a Callback Event Data Structure and a Callback Parameter.
// The Callback parameter is a user definable value that was passed to the
// InitializeBluetooth function.  For this application, this value is not used.
//
//*****************************************************************************
static void
BluetoothCallbackFunction(tCallbackEventData *pCallbackData,
                          void *pvCallbackParameter)
{
    Serial_Code_t serial_code;
    //
    // Verify that the parameters pass in appear valid.
    //
    if(pCallbackData)
    {
        //
        // Process each Callback Event.
        //
        switch(pCallbackData->sEvent)
        {
            //
            // Process each Callback Event.
            //
            case cePinCodeRequest:
            {
                Display(("cePinCodeRequest\r\n"));
                Read_Serial_Code(&serial_code);
                PinCodeResponse(pCallbackData->ucRemoteDevice, BTPS_StringLength((char*)serial_code.serial_num),
                                (char*)serial_code.serial_num);
                break;
            }

            //
            // Handle completion of authentication
            //
            case ceAuthenticationComplete:
            {
                Display(("ceAuthenticationComplete\r\n"));
                break;
            }

            //
            // Handle failure of authentication
            //
            case ceAuthenticationFailure:
            {
                Display(("ceAuthenticationFailure\r\n"));
                break;
            }
            
            default:
                break;
        }
    }
}
#endif


//*****************************************************************************
//
// The following function is the function that is registered with the
// BTPS abstraction layer to support the retrieval of the current
// millisecond tick count. This function is registered with the
// system by putting the value of this function in the
// GetTickCountCallback member of the BTPS_Initialization_t structure
// (and passing this structure to the BTPS_Init() function.
//
//*****************************************************************************
static unsigned long
GetTickCountCallback(void)
{
    //
    // Simply return the current Tick Count.
    //
    return(g_ulTickCount);
}

static void DisplayVersionInformation(void)
{
	char boot_version[15];
	char app_version[15];
	
	BTPS_MemInitialize(boot_version, 0x00, sizeof(boot_version));
	BTPS_MemInitialize(app_version, 0x00, sizeof(app_version));
	
	BTPS_SprintF(boot_version, "%d.%d.%d", ECG_BOOT_VERSION_MAJOR, ECG_BOOT_VERSION_MINOR, ECG_BOOT_VERSION_BUILD);
	BTPS_SprintF(app_version, "%d.%d.%d", ECG_VERSION_MAJOR, ECG_VERSION_MINOR, ECG_VERSION_BUILD);
	
	Display(("\r\nBoot Version: %s\r\n", boot_version));
	Display(("Application Version: %s\r\n\r\n", app_version));
}


#ifdef __ECG_DEBUG__
//*****************************************************************************
//
// Function that is registered with the Bluetooth system (via call to
// BTPS_Init()) for debugging.  This function will be called back for
// each character that is to be output to the debug terminal.
//
//*****************************************************************************
static void BTPSAPI
MessageOutputCallback(char cDebugCharacter)
{
    //
    // Simply output the debug character.
    //
    UARTCharPut(UART0_BASE, cDebugCharacter);
}
#endif

int bt_init(void)
{
    int iRetVal = 0;
    BTPS_Initialization_t sBTPSInitialization;

#ifdef ECG_HARDWARE_BLUETOOTH
    tDeviceInfo sDeviceInfo;
#endif

    //
    // Specify the function that will be responsible for querying the
    // current millisecond Tick Count.
    // * NOTE * This function *MUST* be specified
    //
    sBTPSInitialization.GetTickCountCallback  = GetTickCountCallback;
    sBTPSInitialization.MessageOutputCallback = NULL;

    //
    // Set the callback function the stack can use for printing to the
    // console.
    //
#ifdef DEBUG_ENABLED
    sBTPSInitialization.MessageOutputCallback = MessageOutputCallback;
#else
    sBTPSInitialization.MessageOutputCallback = NULL;
#endif

	BTPS_Init((void *)&sBTPSInitialization);

#ifdef ECG_HARDWARE_BLUETOOTH
    //
    // Initialize the Bluetooth stack, using no callback parameters (NULL).
    //
    iRetVal = InitializeBluetooth(BluetoothCallbackFunction, NULL);
//                                  &sBTPSInitialization);
	
    //
    // Proceed with application if there was no Bluetooth init error.
    //
    if(!iRetVal)
    {
        /* Initialize Transport Layer */
		BTT_Init();
		
        //
        // Make the device Connectable and Discoverable and enabled
        // Secured Simple Pairing.
        //
        SetLocalDeviceMode(CONNECTABLE_MODE | PAIRABLE_SSP_MODE);

        //
        // Get information about our local device.
        //
        iRetVal = GetLocalDeviceInformation(&sDeviceInfo);
        if(!iRetVal)
        {
            //
            // Format the board address into a string, and display it on
            // the console.
            //
            BD_ADDRToStr(sDeviceInfo.ucBDAddr, g_cBoardAddress);
            Display(("Local BD_ADDR: %s\r\n", g_cBoardAddress));

            //
            // Display additional info about the device to the console
            //
            Display(("HCI Version  : %s\r\n",
                     g_pcHCIVersionStrings[sDeviceInfo.ucHCIVersion]));
            Display(("Connectable  : %s\r\n",
                     ((sDeviceInfo.sMode & CONNECTABLE_MODE) ? "Yes" : "No")));
            Display(("Discoverable : %s\r\n",
                     ((sDeviceInfo.sMode & DISCOVERABLE_MODE) ? "Yes" : "No")));
            if(sDeviceInfo.sMode & (PAIRABLE_NON_SSP_MODE | PAIRABLE_SSP_MODE))
            {
                Display(("Pairable     : Yes\r\n"));
                Display(("SSP Enabled  : %s\r\n",
                        ((sDeviceInfo.sMode & PAIRABLE_SSP_MODE) ?
                        "Yes" : "No")));
            }
            else
            {
                Display(("Pairable     : No\r\n"));
            }
        
		VS_Set_Max_Output_Power(g_uiBluetoothStackID, MAX_RADIO_POWER);
		
		}
    }
#endif
	
	return iRetVal;
} /* bt_init() */

static void
UpdateKeepalive()
{
	if(g_KeepaliveEnabled)
	{
		if(g_KeepaliveCounter <= 0)
		{
			Display(("Failed to receive keepalive in time."));
			//g_iResetAfterUpdate = 1;
			g_KeepaliveEnabled = false;
			g_KeepaliveCounter = KEEPALIVE_INTERVAL;
			USB_Reset();
		}
		else
		{
			g_KeepaliveCounter--;
		}
	}
}

static void
MainApp(void *ThreadParameter)
{
	volatile uint32_t counter;
    uint8_t one_ms_timer;
    uint8_t exit_loop;
    uint16_t one_second_timer;
    uint16_t two_second_timer;

#ifdef ECG_HARDWARE_BLUETOOTH
	int bt_init_state;
    uint8_t five_ms_timer;
	uint32_t timer_300_ms;

    five_ms_timer = 1;
#endif

#ifdef ECG_HARDWARE_BATTERY
    uint8_t check_charger;

    check_charger = 0;
#endif
    
    one_ms_timer = 1;
    one_second_timer = 1;
    two_second_timer = 2 * ONE_SEC_INIT;
    exit_loop = 0;
	data_rate_error_cnt = 0;
    
#ifdef ECG_HARDWARE_BLUETOOTH
	GPIOPinWrite(GPIO_BT_N_SHDN_PORT, GPIO_BT_N_SHDN_PIN, ~GPIO_BT_N_SHDN_PIN);
	timer_300_ms = g_ulTickCount + 300;
	while(g_ulTickCount < timer_300_ms);
	GPIOPinWrite(GPIO_BT_N_SHDN_PORT, GPIO_BT_N_SHDN_PIN, GPIO_BT_N_SHDN_PIN);

    bt_init_state = bt_init();

	DisplayVersionInformation();
    
	if(bt_init_state != 0)
	{
		g_ssSystemState = System_State_Running;
		ECG_Status_Set(ECG_Status_Error);
		g_run_post = 0;
		Display(("Bluetooth Initialization Failed!\r\n"));
		
		while(1)
		{
			if(g_ssSystemState == System_State_Off && g_run_post == 0)
			{
				ECG_Data_Capture_Stop();
				BSC_Shutdown(g_uiBluetoothStackID);
				GPIOPinWrite(GPIO_BT_N_SHDN_PORT, GPIO_BT_N_SHDN_PIN, ~GPIO_BT_N_SHDN_PIN);
				ECG_Power_Off();
				exit_loop = 1;
				g_run_post = 1;
			}
		}
	}
#else
    bt_init();
	DisplayVersionInformation();
#endif

#ifdef ECG_HARDWARE_BATTERY
    ECG_Power_Reset_Off_Timer();
#endif
    
    while(exit_loop == 0)
    {
        BTPS_ProcessScheduler();

#ifdef ECG_HARDWARE_BLUETOOTH        
        if(g_toggleBTDiscoverability)
        {
			if(ECG_Data_Capture_In_Progress() == 0)
			{
				ECG_Discover_Start(60);
				discoverability_state = true;
			}
			else
			{
				g_bpsBTPairState = Bluetooth_Pair_State_Not_Discoverable;
			}
            g_toggleBTDiscoverability = false;
        }
        
        if(g_bpsBTPairState == Bluetooth_Pair_State_Discoverable)
        {
            GAP_Discoverability_Mode_t discover_mode;
            uint32_t duration;
            
            GAP_Query_Discoverability_Mode(g_uiBluetoothStackID, &discover_mode, &duration);
            if(discover_mode == dmNonDiscoverableMode)
            {
                ECG_Discover_Stop();
                discoverability_state = false;
                g_bpsBTPairState = Bluetooth_Pair_State_Not_Discoverable;
            }
        }
#endif
        
        if(ECG_Speaker_Get_State() == SPEAKER_ON)
        {
            ECG_Speaker_Off_Check();
        }
        
#ifdef ECG_HARDWARE_BATTERY
        ECG_Power_Off_Check();
		
		/* If the post has been run and external power is not connected,
		 * check to see if the device should shut down due to low battery
		 * voltage.
		 */
		if(!g_run_post && ECG_Power_Get_External_Connection_State() == 0)
			ECG_Power_Check_Low_Battery_Power_Down();
#endif
        
        if(--one_second_timer == 0)
        {
            ECG_ADC_Enable(1);
            
            if(g_iResetAfterUpdate == 1)
            {
                /* Jump to boot code! */
                Display(("Restarting...\r\n"));
                ECG_Force_System_Reset();
            }

#ifdef ECG_HARDWARE_USB
			if(g_bSleepRequested)
			{
				// Wait for bus to quite down
//				for(counter = 0;counter < 30000000;counter++);

//				Display(("Going to sleep..."));
				// Power down unneeded peripherals
				ECG_LED_Disable(ECG_LED_1);
				ECG_Power_Supply_Disable(ECG_Power_Supply_5V0);

				GPIOPinWrite(GPIO_ADS_N_CS_PORT, GPIO_ADS_N_CS_PIN, ~GPIO_ADS_N_CS_PIN);

				// Enable the USB activity interrupt
				GPIOIntEnable(GPIO_PORTQ_BASE, GPIO_INT_PIN_6);
				IntEnable(INT_GPIOQ6);

				while(g_bSleepRequested)
				{
					SysCtlDeepSleep();

					//SysCtlSleep();
				}

//                Display(("Waking up..."));

				// Restart to recover from Sleep
				//g_iResetAfterUpdate = 1;
				GPIOPinWrite(GPIO_ADS_N_CS_PORT, GPIO_ADS_N_CS_PIN, GPIO_ADS_N_CS_PIN);

				ECG_LED_Enable(ECG_LED_1);
				ECG_Power_Supply_Enable(ECG_Power_Supply_5V0);


				for(counter = 0; counter < 480000; counter++);

				init_ADS1x9x();
			}
#endif

#ifdef ECG_HARDWARE_BATTERY
            if(check_charger == 1)
                ECG_Power_Update_Charge_LED();
#endif

            one_second_timer = ONE_SEC_INIT;
        }
        
#ifdef ECG_HARDWARE_USB
		USB_Process_Commands();
#elif ECG_HARDWARE_BLUETOOTH
        BTT_Process_Commands();
#endif
        
        if(ECG_Data_Capture_In_Progress() == 1)
        {
            if(--one_ms_timer == 0)
            {
                ECG_Data_Capture_Queue_Data();
                
                one_ms_timer = ONE_MS_INIT;
            }
            
#ifdef ECG_HARDWARE_BLUETOOTH
            if(--five_ms_timer == 0)
            {
                BTT_Transport_Packets();
                
                ECG_Power_Reset_Off_Timer();
                
                five_ms_timer = FIVE_MS_INIT;
            }
#endif
        }
        
        if(g_run_post == 1)
        {
            if(--two_second_timer == 0)
            {
                ECG_POST_Run();
                g_run_post = 0;

#ifdef ECG_HARDWARE_BATTERY
                ECG_Power_Reset_Off_Timer();
                check_charger = 1;
#endif

                if(!(ECG_Status_Check(ECG_Status_Error) & ECG_Status_Error))
                    ECG_Status_Set(ECG_Status_Normal);
                g_ssSystemState = System_State_Running;
                g_POSTComplete = 1;
            }
        }
        
        if(g_ssSystemState == System_State_Off && g_run_post == 0)
        {
            ECG_Data_Capture_Stop();

#ifdef ECG_HARDWARE_BLUETOOTH
            BSC_Shutdown(g_uiBluetoothStackID);
#endif

            ECG_Power_Off();
            exit_loop = 1;
            g_run_post = 1;
        }
        
        if(g_DataRateError == 1)
        {
            g_DataRateError = 0;

			Display(("Restarting Data Capture!\r\n"));
			ECG_Data_Capture_Restart();
        }

		UpdateKeepalive();
        
        /* Delay 1 millisecond to control loop execution rate */
        BTPS_Delay(1);
    }
}



//*****************************************************************************
//
// The following function is used to configure the hardware platform for the
// intended use.
//
//*****************************************************************************
static void
ConfigureHardware(void)
{
    //
    // Set the current Output debug port (if debugging enabled).
    //
#ifdef __ECG_DEBUG__
    //
    // Configure UART 0 to be used as the debug console port.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
    UARTConfigSetExpClk(UART0_BASE, g_ui32ClockFrequency, 9600,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE));
#endif

    //
    // Setup a 1 ms Timer to implement Tick Count required for Bluetooth
    // Stack.
    //
    g_ulTickCount = 0;
    g_PowerOnTick = 0;
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 0);

    //
    // Configure timer for 1 mS tick rate
    //
    TimerLoadSet(TIMER0_BASE, TIMER_A, g_ui32ClockFrequency / 1000);

    //
    // Enable timer interrupts for the 1ms timer
    //
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);

    //
    // Turn on all interrupts in the system.
    //
    IntMasterEnable();
} /* ConfigureHardware() */


//*****************************************************************************
//
// Timer Interrupt handler that is registered to process Timer Tick Interrupt
// used to keep current Tick Count required for the Bluetooth Stack.
//
//*****************************************************************************
void
TimerTick(void)
{
    //
    // Clear the interrupt and update the tick count.
    //
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    g_ulTickCount++;
}

int main(void)
{
    uint8_t timer_5_sec = 2;
    init_button = 1;
    uint16_t adc_ext_5v_val = 0;
	volatile uint32_t counter;
    uint8_t ext_connected = 0;
    
    while(1)
    {
        ECG_Init();
        
#ifdef ECG_HARDWARE_BLUETOOTH
        if(init_button == 1)
        {
            ECG_Button_Init();
            init_button = 0;
        }
#endif

        ECG_Pre_Init();

#ifndef ECG_HARDWARE_BATTERY 
		g_AutoTurnOn = 1;
#endif

        while(g_ssSystemState == System_State_Off && g_AutoTurnOn == 0)
        {
            if(--timer_5_sec == 0)
            {
                ECG_ADC_Enable(1);
                
#ifdef ECG_HARDWARE_BATTERY
                ECG_Power_Update_Charge_LED();
#endif
                
                ECG_ADC_GetValue(ECG_Adc_External_5V, &adc_ext_5v_val);
                if(adc_ext_5v_val > 4000)
                {
                    TimerLoadSet(TIMER0_BASE, TIMER_A, g_ui32ClockFrequency / 1000);
                    ext_connected = 1;
                }
                else if(ext_connected == 1)
                {
                    ECG_Power_Off();
                }
                else
                {
                    TimerLoadSet(TIMER0_BASE, TIMER_A, g_ui32ClockFrequency / 1000);
                }
                    
                timer_5_sec = 3;
            }
            SysCtlSleep();
        }
        
		g_AutoTurnOn = 1;
		
        /* Set state back to off until POST is complete */
        g_ssSystemState = System_State_Off;
        
        IntMasterDisable();

        TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
        TimerDisable(TIMER0_BASE, TIMER_A);

        for(counter = 0; counter < 480000; counter++);

        ECG_Power_Init();
        
        for(counter = 0; counter < 480000; counter++);
        
        ConfigureHardware();
        
        ECG_Status_Set(ECG_Status_POST);
        
        //
        // Call the application main loop (above).  This function will not return.
        //
        MainApp(NULL);
    }
    
    return(0);
} /* main() */
