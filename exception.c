#include <string.h>

#include "sapphire_pub.h"

#include "exception.h"
#include "BKRNLAPI.h"
#include "ecg_status.h"
#include "sysctl.h"
#include "uart.h"
#include "watchdog.h"

/* Internal - Used to pring information about the fault that occurred.
 * 
 * Returns nothing
 */
void print_msg(char *msg, unsigned char len)
{
    unsigned char i;
    
    for(i = 0; i < len; i++)
        UARTCharPut(UART0_BASE, msg[i]);
} /* print_msg */


/* Internal - Handles a Bus fault and prints useful information.
 * 
 * Returns nothing
 */
void __bus_fault(void)
{
    unsigned char bfsr;
	unsigned long bfar;
	//unsigned int spr;
    char buffer[100];

	memset(buffer, 0x00, 100);
	
	WatchdogReloadSet(WATCHDOG0_BASE, g_ui32ClockFrequency);
	
	//__asm("MOV spr, __current_lr()");
	
    bfsr = *((uint32_t*)0xE000ED29);
	bfar = *((uint32_t*)0xE000ED38);
    
    BTPS_SprintF(buffer, "Bus Fault, BFSR: %xxxx  BFAR: %x\r\n", bfsr, bfar);
    
    /* On data abort exception the LR points to PC+8 */
    print_msg(buffer, 100);
    for(;;);
} /* __bus_fault() */


/* Internal - Handles and hard fault and currently just prints Hard Fault
 * 
 * Returns nothing
 */
void __hard_fault(void)
{
    char buffer[100];

    /* On data abort exception the LR points to PC+8 */
    BTPS_SprintF(buffer, "Hard Fault\r\n");
    print_msg(buffer, 100);
    for(;;);
} /* __hard_fault() */


/* Internal - Handles a usage fault and currently just prints Usage Fault
 * 
 * Returns nothing
 */
void __usage_fault(void)
{
    char buffer[100];

    /* On data abort exception the LR points to PC+8 */
    BTPS_SprintF(buffer, "Usage Fault\r\n");
    print_msg(buffer, 100);
    for(;;);
} /* __usage_fault() */

void bus_fault_handler_c(unsigned int * hardfault_args)
{
unsigned int stacked_r0;
unsigned int stacked_r1;
unsigned int stacked_r2;
unsigned int stacked_r3;
unsigned int stacked_r12;
unsigned int stacked_lr;
unsigned int stacked_pc;
unsigned int stacked_psr;
unsigned int rBFAR;
unsigned int rCFSR;
unsigned int rHFSR;
unsigned int rDFSR;
unsigned int rAFSR;
char buffer[200];

stacked_r0 = ((unsigned long) hardfault_args[0]);
stacked_r1 = ((unsigned long) hardfault_args[1]);
stacked_r2 = ((unsigned long) hardfault_args[2]);
stacked_r3 = ((unsigned long) hardfault_args[3]);

stacked_r12 = ((unsigned long) hardfault_args[4]);
stacked_lr = ((unsigned long) hardfault_args[5]);
stacked_pc = ((unsigned long) hardfault_args[6]);
stacked_psr = ((unsigned long) hardfault_args[7]);

rBFAR = (*((volatile unsigned long *)(0xE000ED38)));
rCFSR = (*((volatile unsigned long *)(0xE000ED28)));
rHFSR = (*((volatile unsigned long *)(0xE000ED2C)));
rDFSR = (*((volatile unsigned long *)(0xE000ED30)));
rAFSR = (*((volatile unsigned long *)(0xE000ED3C)));

memset(buffer, 0x00, sizeof(buffer));

BTPS_SprintF(buffer, "Bus Fault Handler\r\n"
					 "R0 = %x\r\n"
					 "R1 = %x\r\n"
					 "R2 = %x\r\n"
					 "R3 = %x\r\n"
					 "R12 = %x\r\n"
					 "PC = %x\r\n"
					 "LR = %x\r\n"
					 "PSR = %x\r\n",
					 stacked_r0, stacked_r1, stacked_r2, stacked_r3, stacked_r12, stacked_pc, stacked_lr, stacked_psr);

print_msg (buffer, strlen(buffer));
/*memset(buffer, 0x00, sizeof(buffer));

BTPS_SprintF(buffer, "\r\nDebug Variables\r\n"
					  "Debug HCI: %d\r\n"
					  "Debug Line: %d\r\n"
					  "Debug Func: %d\r\n"
					  "Lock Count: %d\r\n"
					  "HCI COMM: %d\r\n",
					  xxxDebug_HCI, xxxDebug_Line, xxxDebug_Func, xxxLockCount, xxxHCICOMM);

print_msg(buffer, strlen(buffer));*/

ECG_Status_Set(ECG_Status_Error);

while(1);
}
