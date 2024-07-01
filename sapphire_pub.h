#ifndef __SAPPHIRE_PUB_H__
#define __SAPPHIRE_PUB_H__

//#define DEV_KIT

#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_types.h"
//#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c1292ncpdt.h"
#include "BKRNLAPI.h"

#define __ECG_HARDWARE__
#define __ECG_DEBUG__

#ifndef NULL
    #define NULL ((void *)0)
#endif

#if defined(__ECG_DEBUG__)
    #define Display(_Message)       DBG_MSG(DBG_ZONE_DEVELOPMENT, _Message)
#else
    #define Display(_Message)
#endif

// Moved out of inc/hw_ints.h to avoid a conflict with inc/tm4c1294kcpdt.h
#define FAULT_HARD              3           // Hard fault
#define FAULT_BUS               5           // Bus fault

typedef enum System_State_enum_t8
{
    System_State_Off,
    System_State_Power_On,
    System_State_Running
} System_State_t8;

#ifdef ECG_HARDWARE_BLUETOOTH
typedef enum Bluetooth_Pair_State_enum_t8
{
    Bluetooth_Pair_State_Not_Discoverable = 0x00,
    Bluetooth_Pair_State_Discoverable = 0x01,
    Bluetooth_Pair_State_Paired_Not_Discoverable = 0x02,
    Bluetooth_Pair_State_Paired_Discoverable = 0x03
} Bluetooth_Pair_State_t8;

extern Bluetooth_Pair_State_t8 g_bpsBTPairState;
#endif

extern unsigned char ucControlTable[1024];
extern System_State_t8 g_ssSystemState;
extern int g_iLEDState;
extern uint8_t g_iResetAfterUpdate;
extern uint8_t g_DataRateError;
extern uint8_t g_POSTComplete;
extern uint8_t g_AutoTurnOn;

#if defined(__ECG_DEBUG__)
extern uint8_t g_TestTransfer;
extern uint32_t g_PowerOnTick;
#endif

extern uint32_t g_ui32ClockFrequency;
extern bool g_bSleepRequested;

extern bool g_KeepaliveEnabled;
extern uint32_t g_KeepaliveCounter;

#define KEEPALIVE_INTERVAL	2500
#endif
