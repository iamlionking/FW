#ifndef CARDEA_DEVICEAPI_H
#define CARDEA_DEVICEAPI_H

/*! @file DeviceAPI.h
	Describes the command and data formats that are transmitted between the
	Cardea ECG device and the PC.
*/

/*! The base value for the commands sent to the device.
	The commands are one-byte values or'd onto the MSB of this value.
	The values are chosen so that dropped bytes can be detected and resync'd
	up.  The command will come through as a series of bytes: 
	@code
		0xAB 0xAC 0xAB [1 byte command] [4 byte parameter]
	@endcode

	Some commands repond with an associated reponse packet.
	@see CardeaCmdGetRevisionResponse
	@see CardeaCmdGetBatteryStatusResponse

*/
#define CARDEA_COMMAND_BASE			0x00ABACAB

//! Macro to generate a command
#define CARDEA_MAKECMD(cmd)	(((cmd) << 24) | CARDEA_COMMAND_BASE)

/*! Command to control the LED

	The param value indicates the control.
	- 0 -> turns the LED off
	- 1 -> turns the LED on solid
	- 2 -> sets a 1Hz 50% duty cycle

	@todo Define what the LED flashing patters are.
*/
#define CARDEA_CMD_LED_CONTROL			    CARDEA_MAKECMD(0x00)

/*! Command to sound the chime

	The param value indicates the chime pattern.

	@todo Define what the chime patters are.
*/
#define CARDEA_CMD_BEEP					    CARDEA_MAKECMD(0x01)

/*! Start data transmission

	The param value represents the sample rate and can be one of
	{ 250, 500, 1000, 2000 }.

*/
#define CARDEA_CMD_START_DATA			    CARDEA_MAKECMD(0x02)

/*! Stop the data transmission
*/
#define CARDEA_CMD_STOP_DATA			    CARDEA_MAKECMD(0x03)

/*! Save current pairing information
*/
#define CARDEA_CMD_SAVE_PAIRING			    CARDEA_MAKECMD(0x04)

/*! Return the device firmware revision

	The response to this command is a CardeaCmdGetRevisionResponse.
*/
#define CARDEA_CMD_GET_REVISION			    CARDEA_MAKECMD(0x05)

/*! Return the battery status

	The response to this command is a CardeaGetBatteryStatusResponse
*/
#define CARDEA_CMD_GET_BATTERY_STATUS	    CARDEA_MAKECMD(0x06)


/*! Return the result of the power-on self-test

	The response to this command is a CardeaGetPOSTResult
*/
#define CARDEA_CMD_GET_POST_RESULT		    CARDEA_MAKECMD(0x07)

/*! Starts the firmware update process

	The param value indicates the number of bytes in the software update.

	The response to this command is a CardeaCmdUpdateFirmwareResponse
*/
#define CARDEA_CMD_UPDATE_FIRMWARE		    CARDEA_MAKECMD(0x08)

/*! Ends the firmware update process

	The response to this command is a CardeaCmdUpdateFirmwareResponse
*/
#define CARDEA_CMD_UPDATE_FIRMWARE_DONE	    CARDEA_MAKECMD(0x09)

/*! Enables/Disables test mode on the device.

	The param value indicates 0=disable, non-0=enable test mode.
*/
#define CARDEA_CMD_ENABLE_TEST_MODE	        CARDEA_MAKECMD(0x0A)

/*! Sets the transmission power on the device
 
       The param value is the power (see device TRS for details).
*/
#define CARDEA_CMD_SET_XMIT_POWER           CARDEA_MAKECMD(0x0B)
 
/*! Requests the link quality
 
       The response to this command is a CardeaCmdGetLinkQualityResponse
*/
#define CARDEA_CMD_GET_LINK_QUALITY         CARDEA_MAKECMD(0x0C)

/*!    Sets the device identification information
 
       The response to this command is a CardeaCmdSetDeviceIdInfoResponse
*/
#define CARDEA_CMD_SET_DEVICE_ID_INFO    CARDEA_MAKECMD(0x0D)

/*! Gets the device secret code
 
       The reponse to this command is a CardeaCmdGetSecretCodeResponse
*/
#define CARDEA_CMD_GET_SECRET_CODE       CARDEA_MAKECMD(0x0E)

/*! Gets the device unique id
 
       The reponse to this command is a CardeaCmdGetUniqueIdResponse
*/
#define CARDEA_CMD_GET_UNIQUE_ID       CARDEA_MAKECMD(0x0F)

/*! Signals the device should restart if it doesn't receive keepalives
 
*/
#define CARDEA_CMD_BEGIN_KEEPALIVE       CARDEA_MAKECMD(0x10)

/*! Resets the keepalive counter to keep the device from restarting
 
       The reponse to this command is a CardeaCmdKeepaliveResponse
*/
#define CARDEA_CMD_KEEPALIVE       CARDEA_MAKECMD(0x11)

/*! Signals the device should not restart if it doesn't receive keepalives
 
*/
#define CARDEA_CMD_END_KEEPALIVE       CARDEA_MAKECMD(0x12)

#define MAX_COMMAND_VALUE                   0x13

/*! Sends 4 bytes of debuggingly useful information
*/
#define CARDEA_CMD_DEBUG				    CARDEA_MAKECMD(0xBB)

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

/*! Format of the command sent to the device
	
	A command sent to the ECG device has this format.  Many commands
	don't have parameters, in which case the param value will be 0.
*/
typedef struct CardeaDeviceCommand_struct {
	uint32_t	command;	//*!< command
	uint32_t	param;		//*!< command parameter
} CardeaDeviceCommand_t;

//! Maximum length of the revision string
#define CARDEA_MAX_REVISION		28

//! Response to the CARDEA_CMD_GET_REVISION
typedef struct CardeaCmdGetRevisionResponse_struct {
	uint32_t	command;						    //!< The command (CARDEA_CMD_GET_REVISION)
	char		app_version[CARDEA_MAX_REVISION];	//!< NULL terminated revision string
    char        boot_version[CARDEA_MAX_REVISION];
    char        bt_version[CARDEA_MAX_REVISION];
    char        hw_version[CARDEA_MAX_REVISION];
} CardeaCmdGetRevisionResponse_t;

/*! Response to the CARDEA_CMD_GET_BATTERY_STATUS

	@todo Define the battery status
*/
typedef struct CardeaCmdGetBatteryStatusResponse_struct {
	uint32_t	cmd;		//!< the command (CARDEA_CMD_GET_BATTERY_STATUS)
	uint32_t	status;		//!< the status
} CardeaCmdGetBatteryStatusResponse_t;

/*! Response to the CARDEA_CMD_GET_POST_RESULT

	@todo Define non-successful status values if necessary
*/
typedef struct CardeaCmdGetPOSTResult_struct {
	uint32_t	cmd;	//!< the command (CARDEA_CMD_GET_POST_RESULT)
	uint32_t	status;	//!< 0 on a successful POST.  
} CardeaCmdGetPOSTResult_t;

/*! Response to the CARDEA_CMD_UPDATE_FIRMWARE command.

	This command also serves as the acknowledgement to the data blocks
*/
typedef struct CardeaCmdUpdateFirmwareResponse_struct {
	uint32_t	cmd;	//!< the command CARDEA_CMD_UPDATE_FIRMWARE
	uint32_t	status;	//!< 0 on a successful transmission, non-zero failure.
} CardeaCmdUpdateFirmwareResponse_t;

/*! The format of the data blocks sent while updating firmware

	After receiving a block, the device will send a CardeaCmdUpdateFirmwareResponse
	structure.
*/
typedef struct CardeaFirmwareUpdateBlock_struct {
	uint32_t	block_number;	//!< The block of data being sent, starting from 0
	uint8_t		data[256];		//!< The firmware data, padded with 0xFF on the last block
	uint32_t	crc;			//!< The crc of the previous 260 bytes.
} CardeaFirmwareUpdateBlock_t;

//! The format of the ECG sample sent
struct CardeaRawEcgSample {
	uint32_t	status;		//!< status bits
	uint16_t	lsw[8];		//!< least significant 16 bits
	int8_t		msb[8];		//!< most significant 8 bits
};
 
/*! The format of the response to the CARDEA_CMD_GET_LINK_QUALITY command
*/
typedef struct CardeaCmdGetLinkQualityResponse_struct {
   uint32_t cmd;           //!< set to CARDEA_CMD_GET_LINK_QUALITY
   uint32_t rssi;          //!< RSSI value
   uint32_t link_quaility; //!< Link quality
} CardeaCmdGetLinkQualityResponse_t;

//! The max length of the serial number
#define CARDEA_MAX_SERIAL_NUMBER	16

//! The length of the secret code
#define CARDEA_MAX_SECRET_CODE		16

/*! The command to set the device ID information
*/
typedef struct CardeaCmdSetDeviceIdInfo_struct {
       uint32_t      cmd;                                       //!< CARDEA_CMD_SET_DEVICE_ID_INFO
       uint32_t      param;                                     //!< Number of bytes to follow [CARDEA_MAX_SERIAL_NUMBER + CARDEA_MAX_SECRET_CODE + 4]
       char          serial_number[CARDEA_MAX_SERIAL_NUMBER];   //!< The serial number.
       uint8_t       secret_code[CARDEA_MAX_SECRET_CODE];       //!< The secret code.
       uint32_t      crc;                                       //! the CRC of the serial number and secret code.
} CardeaCmdSetDeviceIdInfo_t;


/*! The response to the CARDEA_CMD_SET_DEVICE_ID_INFO command
*/
typedef struct CardeaCmdSetDeviceIdInfoResponse_struct {
       uint32_t      cmd;          //!< CARDEA_CMD_SET_DEVICE_ID_INFO
       uint32_t      status;              //!< 0 = success, non-0 = failure
} CardeaCmdSetDeviceIdInfoResponse_t;

/*! The response to the CARDEA_CMD_GET_SECRET_CODE command
*/
typedef struct CardeaCmdGetSecretCodeResponse_struct {
       uint32_t      cmd;                                     //!< CARDEA_CMD_GET_SECRET_CODE
       uint8_t       secret_code[CARDEA_MAX_SECRET_CODE];     //!< The secret code
} CardeaCmdGetSecretCodeResponse_t;

 
/*! The format of the response to the CARDEA_CMD_GET_UNIQUE_ID command
*/
typedef struct CardeaCmdGetUniqueIdResponse_struct {
   uint32_t cmd;           //!< set to CARDEA_CMD_GET_UNIQUE_ID
   uint8_t id[16];         //!< RSSI value
} CardeaCmdGetUniqueIdResponse_t;

/*! The format of the response to the CARDEA_CMD_KEEPALIVE command
*/
typedef struct CardeaCmdKeepaliveResponse_struct {
   uint32_t cmd;           //!< set to CARDEA_CMD_GET_UNIQUE_ID
} CardeaCmdKeepaliveResponse_t;
#endif

