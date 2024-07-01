//*****************************************************************************
//
// bt_ucfg.h - Header file containing configuration user macros/constants for:
//                Bluetooth OS Interface (memory)
//                Bluetooth HCI Transport interface.
//
// Copyright (c) 2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 7611 of the DK-LM3S9B96-EM2-CC2560-BLUETOPIA Firmware Package.
//
//*****************************************************************************

#ifndef _BT_UCFG_H_
#define _BT_UCFG_H_

#include "sapphire_pub.h"
#include "pin_map.h"

// TODO: Set these correctly based on the Tiva schematics when they are completed.

//*****************************************************************************
//
// This section of the file contains macros that configure the Bluetopia
// OS Abstraction layer at build time.  It defines constants that can be
// used to control memory usage and operating parameters.
//
//*****************************************************************************

//*****************************************************************************
//
// Define the maximum number of scheduled functions that can be added to the
// Bluetopia Scheduling loop.
//
//*****************************************************************************
#define MAX_NUMBER_SCHEDULE_FUNCTIONS  8

//*****************************************************************************
//
// Define the total amount of memory that is available to the Bluetooth stack
// at run time.  This buffer is used as a pool which services all memory
// requests and allocations.
//
//*****************************************************************************
#define MEMORY_BUFFER_SIZE             (30*1024)

//*****************************************************************************
//
// This section of the file contains macros that configure the Bluetopia
// HCI Vendor Specific Bluetooth chipset initialization layer at build time.
// It defines the configuration information used to initialize the Bluetooth
// chipset itself.
//
//*****************************************************************************

//*****************************************************************************
//
// Define the baud rate that will be used for communication with the
// Bluetooth chip.  The value of this rate will be configured in the
// Bluetooth transport.
//
//*****************************************************************************
#define VENDOR_BAUD_RATE               921600L

//*****************************************************************************
//
// This section of the file contains macros that configure the Bluetopia
// HCI transport layer at build time.  It defines features of the hardware
// that is used to communicate with the Bluetooth hardware, and can be
// changed to accommodate a custom board.
//
//*****************************************************************************

//*****************************************************************************
//
// Define the UART buffer sizes that will be used to buffer incoming/outgoing
// UART data.  The value are:
//
//    DEFAULT_INPUT_BUFFER_SIZE  - UART input buffer size (in bytes)
//    DEFAULT_OUTPUT_BUFFER_SIZE - UART output buffer size (in bytes)
//    DEFAULT_XOFF_LIMIT         - UART H/W Flow Off lower limit
//    DEFAULT_XON_LIMIT          - UART H/W Flow On (re-enable)
//
//*****************************************************************************
#define DEFAULT_INPUT_BUFFER_SIZE       512
#define DEFAULT_OUTPUT_BUFFER_SIZE      1024

#define DEFAULT_XOFF_LIMIT              128
#define DEFAULT_XON_LIMIT               256

//*****************************************************************************
//
// Here is the default allocation of pins to connect to the Bluetooth device,
// using UART1.
//
// Function     Port/Pin
// --------     --------
//   RX           PC6
//   TX           PC7
//   RTS          PJ6git st
//   CTS          PJ3
//  RESET         PC4
//
//*****************************************************************************

#ifndef __ECG_HARDWARE__
//*****************************************************************************
//
// Define the UART peripheral that is used for the Bluetooth interface.
//
// Note: at this time, only UART1 can be used because it supports hardware
// flow control.
//
//*****************************************************************************
#define HCI_UART_BASE               UART1_BASE
#define HCI_UART_INT                INT_UART1
#define HCI_UART_PERIPH             SYSCTL_PERIPH_UART1

//*****************************************************************************
//
// Define the GPIO ports and pins that are used for the UART RX/TX signals.
//
// Note: see gpio.h for possible values for HCI_PIN_CONFIGURE_ macros.
//
//*****************************************************************************
#define HCI_UART_GPIO_PERIPH        SYSCTL_PERIPH_GPIOC
#define HCI_UART_GPIO_BASE          GPIO_PORTC_BASE
#define HCI_UART_PIN_RX             GPIO_PIN_6
#define HCI_UART_PIN_TX             GPIO_PIN_7
#define HCI_PIN_CONFIGURE_UART_RX   GPIO_PC6_U1RX
#define HCI_PIN_CONFIGURE_UART_TX   GPIO_PC7_U1TX

//*****************************************************************************
//
// Define the GPIO ports and pins that are used for the UART RTS signal.
//
//*****************************************************************************
#define HCI_UART_RTS_GPIO_PERIPH    SYSCTL_PERIPH_GPIOJ
#define HCI_UART_RTS_GPIO_BASE      GPIO_PORTJ_BASE
#define HCI_UART_PIN_RTS            GPIO_PIN_6
#define HCI_PIN_CONFIGURE_UART_RTS  GPIO_PJ6_U1RTS

//*****************************************************************************
//
// Define the GPIO ports and pins that are used for the UART CTS signal.
//
//*****************************************************************************
#define HCI_UART_CTS_GPIO_PERIPH    SYSCTL_PERIPH_GPIOJ
#define HCI_UART_CTS_GPIO_BASE      GPIO_PORTJ_BASE
#define HCI_UART_PIN_CTS            GPIO_PIN_3
#define HCI_PIN_CONFIGURE_UART_CTS  GPIO_PJ3_U1CTS

//*****************************************************************************
//
// Define the GPIO ports and pins that are used for the Bluetooth RESET(nSHDN)
// signal.
//
//*****************************************************************************
#define HCI_RESET_PERIPH            SYSCTL_PERIPH_GPIOC
#define HCI_RESET_BASE              GPIO_PORTC_BASE
#define HCI_RESET_PIN               GPIO_PIN_4

#else

//*****************************************************************************
//
// Here is the default allocation of pins to connect to the Bluetooth device,
// using UART1 on the ECG Hardware.
//
// Function     Port/Pin
// --------     --------
//   RX           PC6
//   TX           PC7
//   RTS          PF1
//   CTS          PD0
//  RESET         PC4
//
//*****************************************************************************
    
//*****************************************************************************
//
// Define the UART peripheral that is used for the Bluetooth interface.
//
// Note: at this time, only UART1 can be used because it supports hardware
// flow control.
//
//*****************************************************************************
#define HCI_UART_BASE               UART1_BASE
#define HCI_UART_INT                INT_UART1
#define HCI_UART_PERIPH             SYSCTL_PERIPH_UART1

//*****************************************************************************
//
// Define the GPIO ports and pins that are used for the UART RX/TX signals.
//
// Note: see gpio.h for possible values for HCI_PIN_CONFIGURE_ macros.
//
//*****************************************************************************
#define HCI_UART_GPIO_PERIPH        SYSCTL_PERIPH_GPIOC
#define HCI_UART_GPIO_BASE          GPIO_PORTC_BASE
#define HCI_UART_PIN_RX             GPIO_PIN_6
#define HCI_UART_PIN_TX             GPIO_PIN_7
//#define HCI_PIN_CONFIGURE_UART_RX   GPIO_PC6_U1RX
#define HCI_PIN_CONFIGURE_UART_RX   GPIO_PB0_U1RX
//#define HCI_PIN_CONFIGURE_UART_TX   GPIO_PC7_U1TX
#define HCI_PIN_CONFIGURE_UART_TX   GPIO_PB1_U1TX

//*****************************************************************************
//
// Define the GPIO ports and pins that are used for the UART RTS signal.
//
//*****************************************************************************
#define HCI_UART_RTS_GPIO_PERIPH    SYSCTL_PERIPH_GPIOF
#define HCI_UART_RTS_GPIO_BASE      GPIO_PORTF_BASE
#define HCI_UART_PIN_RTS            GPIO_PIN_1
//#define HCI_PIN_CONFIGURE_UART_RTS  GPIO_PF1_U1RTS
#define HCI_PIN_CONFIGURE_UART_RTS  GPIO_PE0_U1RTS

//*****************************************************************************
//
// Define the GPIO ports and pins that are used for the UART CTS signal.
//
//*****************************************************************************
#define HCI_UART_CTS_GPIO_PERIPH    SYSCTL_PERIPH_GPIOD
#define HCI_UART_CTS_GPIO_BASE      GPIO_PORTD_BASE
#define HCI_UART_PIN_CTS            GPIO_PIN_0
//#define HCI_PIN_CONFIGURE_UART_CTS  GPIO_PD0_U1CTS
#define HCI_PIN_CONFIGURE_UART_CTS  GPIO_PN1_U1CTS

//*****************************************************************************
//
// Define the GPIO ports and pins that are used for the Bluetooth RESET(nSHDN) 
// signal.
//
//*****************************************************************************
#define HCI_RESET_PERIPH            SYSCTL_PERIPH_GPIOC
#define HCI_RESET_BASE              GPIO_PORTC_BASE
#define HCI_RESET_PIN               GPIO_PIN_4
#endif /* __ECG_HARDWARE__ */

#endif /* _BT_UCFG_H_ */
