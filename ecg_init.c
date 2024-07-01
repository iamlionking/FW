#include "sapphire_pub.h"

#include "interrupt.h"
#include "sysctl.h"
#include "watchdog.h"
#include "inc/hw_watchdog.h"
#include "ADS1x9x.h"

#include "ecg_adc.h"
#include "ecg_battery.h"
#include "ecg_button.h"
#include "ecg_data_capture.h"
#include "ecg_discover.h"
#include "ecg_gpio.h"
#include "ecg_led.h"
#include "ecg_pwm.h"
#include "ecg_power_ctl.h"
#include "ecg_speaker.h"
#include "ecg_ssi.h"
#include "ecg_status.h"
#include "ecg_usb.h"
#include "firmware_download.h"
#include "interrupt.h"
#include "rom.h"
#include "timer.h"

/* System variables */
System_State_t8 g_ssSystemState;

#ifdef ECG_HARDWARE_BLUETOOTH
Bluetooth_Pair_State_t8 g_bpsBTPairState;
#endif

uint8_t g_iResetAfterUpdate;
uint32_t g_ui32ClockFrequency;
bool g_bSleepRequested;

/* Local Function Prototpyes */
static void init_watchdog(void);
static void watchdog_int_handler(void);


/* Public - Initializes the drivers and subsystems required by the ECG hardware.
 * 
 * Examples
 * 
 *      ECG_Init()
 * 
 * Returns nothing.
 */
void ECG_Init()
{
    SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);

    //
    // Run from the PLL at 120 MHz.
    //
    //g_ui32ClockFrequency = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    
	// Evaluation Kit Board
	//g_ui32ClockFrequency = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 80000000);
	
	// Rev 1 Board
	g_ui32ClockFrequency = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 80000000);

/*
#ifdef ECG_HARDWARE_USB
	// USB requires an external oscillator
    SysCtlClockSet((SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | 
						SYSCTL_XTAL_16MHZ));
#else
    SysCtlClockSet((SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | 
                        SYSCTL_OSC_INT));
#endif
*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    TimerConfigure(TIMER2_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | TIMER_CFG_B_PERIODIC));

    ECG_Gpio_Poweron();
    
    ECG_Power_Poweron();
    
    ECG_Gpio_Init();
    
    ECG_LED_Init();
    
    ECG_SSI_Init();
    
    ECG_ADC_Init();
    
    ECG_PWM_Init();
    
    ECG_Data_Capture_Init();
    
    ECG_Firmware_Download_Init();
    
#ifdef ECG_HARDWARE_BLUETOOTH
    ECG_Discover_Init();
#endif

    ECG_Speaker_Init();
    
    ECG_Status_Init();
    
#ifdef ECG_HARDWARE_BATTERY
    ECG_Battery_Init();
#endif

#ifdef ECG_HARDWARE_USB
	ECG_USB_Init();
#endif

    IntEnable(FAULT_HARD);
    IntEnable(FAULT_BUS);
    //IntEnable(FAULT_USAGE);
    
    init_watchdog();
    
#ifdef ECG_HARDWARE_BLUETOOTH
    g_bpsBTPairState = Bluetooth_Pair_State_Not_Discoverable;
#endif

    g_iResetAfterUpdate = 0;
	g_bSleepRequested = false;
    
    /* Set state to running for now */
    g_ssSystemState = System_State_Off;
} /* ECG_Init() */


/* Public - Disables the watchdog and forces a system reset.
 * 
 * Examples
 * 
 *      ECG_Force_System_Reset();
 *
 * Returns nothing.
 */
void ECG_Force_System_Reset()
{
    WatchdogIntUnregister(WATCHDOG0_BASE);
} /* ECG_Force_System_Reset() */


/* Public - Perform some further initialization before turning on the system.
 * 
 * Examples
 * 
 *      ECG_Pre_Init();
 * 
 * Returns nothing
 */
void ECG_Pre_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    
    TimerLoadSet(TIMER0_BASE, TIMER_A, g_ui32ClockFrequency * 10);
    
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);

	// Configure sleep timer to wake us up to check for a USB connection
//	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);
//	TimerDisable(TIMER4_BASE, TIMER_A);
//	TimerConfigure(TIMER4_BASE, TIMER_CFG_PERIODIC);

//	TimerLoadSet(TIMER4_BASE, TIMER_A, g_ui32ClockFrequency * 1000);
//	IntEnable(INT_TIMER4A);
//	TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    
    IntMasterEnable();
    
	SysCtlPeripheralClockGating(true);
//	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_USB0);
//	SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOQ);

	// Deep sleep configuration
//	SysCtlPeripheralClockGating(true);
//	SysCtlDeepSleepClockConfigSet(SYSCTL_DSLP_DIV_1, SYSCTL_DSLP_OSC_INT30 | SYSCTL_DSLP_PIOSC_PD);

//	SysCtlDeepSleepClockConfigSet(1, SYSCTL_DSLP_OSC_MAIN);
	SysCtlDeepSleepClockConfigSet(1, SYSCTL_DSLP_OSC_INT30 | SYSCTL_DSLP_PIOSC_PD | SYSCTL_DSLP_MOSC_PD);
	SysCtlDeepSleepPowerSet(SYSCTL_FLASH_LOW_POWER | SYSCTL_SRAM_LOW_POWER);

	SysCtlPeripheralDeepSleepEnable(SYSCTL_PERIPH_GPIOQ);
} /* ECG_Pre_Init() */


/* Internal - Intializes the watchdog timer.
 * 
 * Returns nothing.
 */
void init_watchdog(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    WatchdogIntRegister(WATCHDOG0_BASE, &watchdog_int_handler);
    WatchdogReloadSet(WATCHDOG0_BASE, g_ui32ClockFrequency);
    WatchdogResetEnable(WATCHDOG0_BASE);
    WatchdogEnable(WATCHDOG0_BASE);

	// Enable watchdog stall on CPU_HALT from debugger
	HWREG(WATCHDOG0_BASE + WDT_O_TEST) |= (0x1 << 0x8);
} /* init_watchdog() */


/* Internal - Interrupt handler for the watchdog timer.
 * 
 * Returns nothing.
 */
void watchdog_int_handler(void)
{
    WatchdogIntClear(WATCHDOG0_BASE);
} /* watchdog_int_handler() */
