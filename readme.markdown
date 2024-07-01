Sapphire Embedded Firmware Design
=================================

General Architecture
--------------------

The Sapphire Embedded Firmware is organized into six major design components:

*	Stellaris Driver Library
*	ECG Specific Software Drivers
*	ADS 1x9x Interface
*	Bluetopia Bluetooth Stack Interface
*	Embedded Application
*	Bootload

The following sections describe each of the major design components.

### Stellaris Driver Library

The Stellaris Driver Library is provided by TI and provides easy perepherial access on TI Steallaris microcontrollers.  The library is organized into C header and source files for each major perepherial.  The function based interface provides an abstraction layer from the low level register access.  
The Sapphire project compiles this library directly due to some minor changes that were necessary to achieve the project objectives.  Additionally, the plan is to remove unneeded perepherials from the library to reduce code size requirements.  Since the library is provided from TI, the removed code can always be added back later as necessary.

### ECG Specific Software Drivers

The ECG Specific Software Drivers provide a generic level of access to many of the perepherials on the microcontroller but implement the specific objectives the ECG software requires to operate.  The drivers interface mainly with the Stellaris Software library and each other as necessary.  This architecture allows the application software to focus on performing the tasks necessary in an generic manner and not worry about the details of directly accessing a perepherial.

The following drivers are implemented in this component:

* ECG_ADC
* ECG_Battery
* ECG_Button
* ECG_Data_Capture
* ECG_Discover
* ECG_Flash
* ECG_GPIO
* ECG_Init
* ECG_LED
* ECG_Power_Ctl
* ECG_PWM
* ECG_Speaker
* ECG_SSI	

### ADS1x9x Interface

The ADS1x9x Interface provides a generic interface to the TI ADS1298 24-bit ADC.  The interface makes use of the ECG Specific Drivers to interface with the perepherials needed to provide data acquisition.  The application talks to the ADS though this interface.

This interface handles reset and configuartion of the ADS as well as the starting and stopping of data acquisition.  The interface does not handle the data packaging required for Sapphire data transmission.  The data packaging is handled by software in the ECG Specific Software Drivers.

### Bluetopia Bluetooth Stack Interface

The Bluetopia Bluetooth Stack is provided to TI customers for use on TI Stellairs microcontrollers with the PAN 1315 radio module.

Full documentation on this API is provided by Stonestreet One.

### Embedded Application

The Embedded Application is responsible for orchestrating all action on the microcontroller.  The application interfaces with the ECG Specific Software Drivers, Bluetooth Interface, and ADS1x9x Interface to provide the functionality necessary for an operational Sapphire device.

### Bootloader

The Bootloader software loads the application and performs firmware updates if necessary.  Software updates are only performed if new firmware exists on the external flash part.  If new firmware exists on the flash, the Bootloader will load the new firmware from the external flash and then continue by executing the firmware.

The existance of new firmware is determined by reading a firmware information stored on the flash and verifying the CRC.  If either the firmware information or actually firmware data CRCs fail, the firmware already programmed on the device will not be overwritten with the firmware on the external flash.

Detailed Design
---------------

### ECG Specific Software Drivers

#### ECG ADC

This driver provides access to the ADCs detailed on the Sapphire schematic.  

#### ECG Battery

This driver provides access to information about the battery status and charge level.

#### ECG Button

This driver provides interrupts for handling button presses and the timing required for entering discoverable mode and powering off the device.

#### ECG Data Capature

This driver performs the necessary actions to initialize the ADS at the requested data rate and capture the data as it become available.  Additionally, this driver is responsible for writing the data to the Bluetooth stack for transmission.

#### ECG Discover

This driver is responsible for calling the proper Bluetooth stack functions to make the device discoverable.  Additionally, this driver provides an interface to exit discoverable mode when the device becomes paired with a PC Application.

#### ECG Flash

This driver provides an inteface to the external flash part.  The interface is not generic access to the flash part, but is specific to the needs of the ECG project.  As such, the functionality provides allows the writting of firmware information (firmware size and CRC) and the actual firmware data.

A suggested improvement to this would be to add a more generic interface to the flash part.

#### ECG GPIO

This driver configures each of the GPIO for the project as detailed on the schematic.

The intent of this driver was to provide generic GPIO access through pin names, but this portion has yet to be implemented.  Currently, GPIO access uses the constants defined in this driver, but the GPIO are interfaced directly through the Stellaris Driver Library.

#### ECG Init

This driver provides and interface to initialize the drivers and perform other necessary hardware initialization (system clock speed, etc).

#### ECG LED

This driver provides an interface to turn the LEDs on and off as well as flash then at some period and duty cycle.

#### ECG Power Ctl

This driver provides an interface to enable and disable power supplies on the hardware as needed by the application.

#### ECG PWM

This driver provides generic access to the PWM controlled pins detailed on the schematic (Speaker and LEDs).

#### ECG Speaker

This driver provides and interface to turn the speaker on and off.  Additionally, it is responsible for handling the timing of a BEEP command (currently 1.5 seconds).

#### ECG SSI

This driver simply provides an interface to configure the SSI bus for either ADS access or External Flash access.  The two parts use different clock idle states and clock phase.
