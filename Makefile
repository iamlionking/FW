#******************************************************************************
#
# Makefile - Rules for building the SPP demo for Bluetopia Bluetooth stack.
#
# Copyright (c) 2011 Texas Instruments Incorporated.  All rights reserved.
# Software License Agreement
# 
# Texas Instruments (TI) is supplying this software for use solely and
# exclusively on TI's microcontroller products. The software is owned by
# TI and/or its suppliers, and is protected under applicable copyright
# laws. You may not combine this software with "viral" open-source
# software in order to form a larger program.
# 
# THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
# NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
# NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
# CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
# DAMAGES, FOR ANY REASON WHATSOEVER.
# 
# This is part of revision 7611 of the DK-LM3S9B96-EM2-CC2560-BLUETOPIA Firmware Package.
#
#******************************************************************************

#
# Defines the part type that this project uses.
#
PART=TM4C1292NCPDT
#
# The base directory for StellarisWare.
#
ROOT=../Tiva

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=${ROOT}/Bluetopia/btpsvend
VPATH+=${ROOT}/Bluetopia/btpskrnl/noos
VPATH+=${ROOT}/Bluetopia/hcitrans/noos
VPATH+=${ROOT}/usblib
VPATH+=${ROOT}/utils
#VPATH+=../DriverLib

#
# Where to find header files that do not live in the source directory.
#
IPATH=.
IPATH+=${ROOT}
IPATH+=${ROOT}/Bluetopia/btpsvend
IPATH+=${ROOT}/Bluetopia/include
IPATH+=${ROOT}/Bluetopia/btpskrnl/noos
IPATH+=${ROOT}/Bluetopia/hcitrans/noos
IPATH+=${ROOT}/driverlib

CFLAGSgcc=-DTARGET_IS_TM4C129_RA0 -D__NEWLIB__ -DUART_BUFFERED -DDEBUG_ENABLED -DBTPS_KERNEL_NON_THREADED -DDEBUG_ZONES=DBG_ZONE_ANY -D__SUPPORT_PAN1315ETU__ -DDEPRECATED

# The default rule, which causes the SPP demo for Bluetopia Bluetooth stack to be built.
#
all: CFLAGSgcc += -DECG_HARDWARE_USB -DECG_HARDWARE_BLUETOOTH -DECG_HARDWARE_BATTERY -DECG_HARDWARE_BUTTON -DECG_EXTERNAL_FLASH
all: ${COMPILER}/cardea_app.axf

usb: CFLAGSgcc += -DECG_HARDWARE_USB
usb: ${COMPILER}/cardea_app_usb.axf

bluetooth: CFLAGSgcc += -DECG_HARDWARE_BLUETOOTH -DECG_HARDWARE_BATTERY -DECG_HARDWARE_BUTTON -DECG_EXTERNAL_FLASH
bluetooth: ${COMPILER}/cardea_app_bluetooth.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}
	@mkdir ${COMPILER}

#
# The rule to build libusb
#
libusb:
	@$(MAKE) -C ${ROOT}/usblib

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir -p ${COMPILER}

#
# Rules for building the Cardea Application.
#
${COMPILER}/cardea_app.axf: ${COMPILER}/ADS1x9x_Device_Status.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ADS1x9x_Functions.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ADS1x9x_SPI_Functions.o
${COMPILER}/cardea_app.axf: ${COMPILER}/bluetooth.o
${COMPILER}/cardea_app.axf: ${COMPILER}/bt_transport.o
${COMPILER}/cardea_app.axf: ${COMPILER}/BTPSKRNL.o
${COMPILER}/cardea_app.axf: ${COMPILER}/BTPSVEND.o
${COMPILER}/cardea_app.axf: ${COMPILER}/cardea_app.o
${COMPILER}/cardea_app.axf: ${COMPILER}/command_proc.o
${COMPILER}/cardea_app.axf: ${COMPILER}/crc.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_adc.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_battery.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_button.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_data_capture.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_discover.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_flash.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_gpio.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_init.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_led.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_post.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_power_ctl.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_pwm.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_ssi.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_speaker.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_status.o
${COMPILER}/cardea_app.axf: ${COMPILER}/exception.o
${COMPILER}/cardea_app.axf: ${COMPILER}/hard_fault.o
${COMPILER}/cardea_app.axf: ${COMPILER}/firmware_download.o
${COMPILER}/cardea_app.axf: ${COMPILER}/HCITRANS.o
${COMPILER}/cardea_app.axf: ${COMPILER}/init_ADS1x9x.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ringbuf.o
${COMPILER}/cardea_app.axf: ${COMPILER}/serial_num.o
${COMPILER}/cardea_app.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ustdlib.o
${COMPILER}/cardea_app.axf: ${COMPILER}/ecg_usb.o

${COMPILER}/cardea_app.axf: ${COMPILER}/test_ecg_flash.o
${COMPILER}/cardea_app.axf: ${COMPILER}/test_transfer.o

${COMPILER}/cardea_app.axf: ${ROOT}/Bluetopia/${COMPILER}/libBluetopiaNoOS.a
${COMPILER}/cardea_app.axf: ${ROOT}/usblib/${COMPILER}/libusb.a
${COMPILER}/cardea_app.axf: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/cardea_app.axf: cardea_app.ld

SCATTERgcc_cardea_app=cardea_app.ld
ENTRY_cardea_app=ResetISR

#
# Rules for building the USB Cardea Application.
#
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ADS1x9x_Device_Status.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ADS1x9x_Functions.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ADS1x9x_SPI_Functions.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/BTPSKRNL.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/cardea_app.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/command_proc.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/crc.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_adc.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_data_capture.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_discover.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_internal_flash.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_gpio.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_init.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_led.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_post.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_power_ctl.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_pwm.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_ssi.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_speaker.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_status.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_unique_id.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/exception.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/hard_fault.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/firmware_download.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/init_ADS1x9x.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ringbuf.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/serial_num.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ustdlib.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/ecg_usb.o

${COMPILER}/cardea_app_usb.axf: ${COMPILER}/test_ecg_flash.o
${COMPILER}/cardea_app_usb.axf: ${COMPILER}/test_transfer.o

${COMPILER}/cardea_app_usb.axf: ${ROOT}/usblib/${COMPILER}/libusb.a
${COMPILER}/cardea_app_usb.axf: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/cardea_app_usb.axf: cardea_app.ld

SCATTERgcc_cardea_app_usb=cardea_app.ld
ENTRY_cardea_app_usb=ResetISR

#
# Rules for building the bluetooth Cardea Application.
#
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ADS1x9x_Device_Status.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ADS1x9x_Functions.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ADS1x9x_SPI_Functions.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/bluetooth.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/bt_transport.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/BTPSKRNL.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/BTPSVEND.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/cardea_app.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/command_proc.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/crc.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_adc.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_battery.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_button.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_data_capture.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_discover.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_flash.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_gpio.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_init.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_led.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_post.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_power_ctl.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_pwm.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_ssi.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_speaker.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ecg_status.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/exception.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/hard_fault.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/firmware_download.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/HCITRANS.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/init_ADS1x9x.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ringbuf.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/serial_num.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/ustdlib.o

${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/test_ecg_flash.o
${COMPILER}/cardea_app_bluetooth.axf: ${COMPILER}/test_transfer.o

${COMPILER}/cardea_app_bluetooth.axf: ${ROOT}/Bluetopia/${COMPILER}/libBluetopiaNoOS.a
${COMPILER}/cardea_app_bluetooth.axf: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/cardea_app_bluetooth.axf: cardea_app.ld

SCATTERgcc_cardea_app_bluetooth=cardea_app.ld
ENTRY_cardea_app_bluetooth=ResetISR

#
# Hardware flags
#
# ECG_HARDWARE_BLUETOOTH and ECG_HARDWARE_USB are mutually exclusive

#CFLAGSgcc+=-DECG_HARDWARE_BLUETOOTH
#CFLAGSgcc+=-DECG_HARDWARE_USB
#CFLAGSgcc+=-DECG_HARDWARE_BATTERY

#
# Debug flags
#

#CFLAGSgcc+=-DDEBUG -g -O0

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
