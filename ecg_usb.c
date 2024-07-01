#include <string.h>

#include "sapphire_pub.h"

#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"

#include "usb.h"
#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"

#include "ecg_usb.h"
#include "serial_num.h"
#include "device_api.h"
#include "command_proc.h"
#include "ecg_packet.h"

#define USB_BUFFER_SIZE 4096

/* Firmware Download Packet */
static CardeaFirmwareUpdateBlock_t firmware_packet;
static CardeaCmdSetDeviceIdInfo_t device_info_packet;

static tLineCoding g_sLineCoding =
{
    115200, USB_CDC_STOP_BITS_1_5, USB_CDC_PARITY_NONE, 8
};

const unsigned char g_pLangDescriptor[] =
{
	4,
	USB_DTYPE_STRING,
	USBShort(USB_LANG_EN_US)
};

const unsigned char g_pManufacturerString[] =
{
	2 + (23 * 2),
	USB_DTYPE_STRING,
	'C', 0, 'a', 0, 'r', 0, 'd', 0, 'e', 0, 'a', 0, ' ', 0,
	'A', 0 ,'s', 0, 's', 0, 'o', 0, 'c', 0, 'i', 0, 'a', 0, 't', 0, 'e', 0, 's', 0, ',', 0, ' ', 0,
	'I', 0, 'n', 0, 'c', 0, '.', 0
};

const unsigned char g_pProductString[] =
{
	2 + (13 * 2),
	USB_DTYPE_STRING,
	'C', 0, 'a', 0, 'r', 0, 'd', 0, 'e', 0, 'a', 0, ' ', 0,
	'S', 0, 'c', 0, 'r', 0, 'e', 0, 'e', 0, 'n', 0
};

const unsigned char g_pControlInterfaceString[] =
{
	2 + (23 * 2),
	USB_DTYPE_STRING,
	'C', 0, 'a', 0, 'r', 0, 'd', 0, 'e', 0, 'a', 0, ' ', 0, 'S', 0, 'c', 0, 'r', 0, 'e', 0, 'e', 0, 'n', 0, ' ', 0,
	'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0
};

const unsigned char g_pConfigString[] =
{
	2 + (27 * 2),
	USB_DTYPE_STRING,
	'C', 0, 'a', 0, 'r', 0, 'd', 0, 'e', 0, 'a', 0, ' ', 0,
	'S', 0, 'c', 0, 'r', 0, 'e', 0, 'e', 0, 'n', 0, ' ', 0,
	'C', 0, 'o', 0, 'n', 0, 'f', 0, 'i', 0, 'g', 0, 'u', 0, 'r', 0, 'a', 0, 't', 0, 'i', 0, 'o', 0, 'n', 0
};

//unsigned char g_pSerialNumberString[2 + (SERIAL_NUM_LEN * 2)];
unsigned char g_pSerialNumberString[] =
{
	2 + (8 * 2),
	USB_DTYPE_STRING,
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

const unsigned char* const g_pStringDescriptors[] =
{
	g_pLangDescriptor,
	g_pManufacturerString,
	g_pProductString,
	g_pSerialNumberString,
	g_pControlInterfaceString,
	g_pConfigString
};

tCDCSerInstance g_sCDCInstance;

#define NUM_STRING_DESCRIPTORS (sizeof(g_pStringDescriptors) / sizeof(unsigned char *))

uint8_t g_bConnected = 0;
tUSBDCDCDevice* g_pUSBDevice = NULL;

// Callbacks

unsigned long RxHandler(void* pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void* pvMsgData);
unsigned long TxHandler(void* pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void* pvMsgData);
unsigned long ControlHandler(void* pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void* pvMsgData);

extern const tUSBBuffer g_sTxBuffer;
extern const tUSBBuffer g_sRxBuffer;

tUSBDCDCDevice g_sCDCDevice =
{
	USB_VID_CARDEA,
	USB_PID_RUBY,
	100,
	USB_CONF_ATTR_BUS_PWR,
	ControlHandler,
	(void*)&g_sCDCDevice,
	USBBufferEventCallback,
	(void*)&g_sRxBuffer,
	USBBufferEventCallback,
	(void*)&g_sTxBuffer,
	g_pStringDescriptors,
	NUM_STRING_DESCRIPTORS,
};

unsigned char g_pcUSBRxBuffer[USB_BUFFER_SIZE];
unsigned char g_pucRxBufferWorkspace[USB_BUFFER_WORKSPACE_SIZE];

const tUSBBuffer g_sRxBuffer =
{
	false,
	RxHandler,
	(void*)&g_sCDCDevice,
	USBDCDCPacketRead,
	USBDCDCRxPacketAvailable,
	(void*)&g_sCDCDevice,
	g_pcUSBRxBuffer,
	USB_BUFFER_SIZE,
	g_pucRxBufferWorkspace
};

unsigned char g_pcUSBTxBuffer[USB_BUFFER_SIZE];
unsigned char g_pucTxBufferWorkspace[USB_BUFFER_WORKSPACE_SIZE];

const tUSBBuffer g_sTxBuffer =
{
	true,
	TxHandler,
	(void*)&g_sCDCDevice,
	USBDCDCPacketWrite,
	USBDCDCTxPacketAvailable,
	(void*)&g_sCDCDevice,
	g_pcUSBTxBuffer,
	USB_BUFFER_SIZE,
	g_pucTxBufferWorkspace
};

static void SetLineCoding(tLineCoding *psLineCoding)
{
    memcpy(&g_sLineCoding, psLineCoding, sizeof(tLineCoding));
}

static void GetLineCoding(tLineCoding *psLineCoding)
{
    memcpy(psLineCoding, &g_sLineCoding, sizeof(tLineCoding));
}

unsigned long ControlHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData)
{
	switch(ulEvent)
	{
		case USB_EVENT_CONNECTED:
			Display(("USB Connected"));
			g_bConnected = 1;
			g_bSleepRequested = false;

			USBBufferFlush(&g_sTxBuffer);
			USBBufferFlush(&g_sRxBuffer);
			break;

		case USB_EVENT_DISCONNECTED:
			Display(("USB Disconnected"));
			g_bConnected = 0;
			break;

		case USB_EVENT_SUSPEND:
			Display(("USB Suspend"));
			if(g_bConnected)
			{
				g_bSleepRequested = true;
			}
			else
			{
				Display(("Not connected, ignoring suspend."));
			}

			break;

		case USB_EVENT_RESUME:
			// This interrupt should wake us from deep sleep
			g_bSleepRequested = false;
			Display(("USB Resume"));
			break;

        case USBD_CDC_EVENT_GET_LINE_CODING:
            GetLineCoding(pvMsgData);
            break;

        case USBD_CDC_EVENT_SET_LINE_CODING:
            SetLineCoding(pvMsgData);
            break;

        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
        case USBD_CDC_EVENT_SEND_BREAK:
        case USBD_CDC_EVENT_CLEAR_BREAK:
			// Not applicable as we are not using a UART
            break;

		default:
			Display(("Unhandled control event %d", ulEvent));
			break;
	}

	return 0;
}

unsigned long TxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData)
{
	switch(ulEvent)
	{
		case USB_EVENT_TX_COMPLETE:
			//Display(("USB Tx Complete"));
			g_KeepaliveCounter = KEEPALIVE_INTERVAL;
			break;
	}

	return 0;
}

unsigned long RxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData)
{
	switch(ulEvent)
	{
		case USB_EVENT_RX_AVAILABLE:
			//Display(("USB Rx Available"));
			break;

		case USB_EVENT_DATA_REMAINING:
			//Display(("USB Data Remaining"));
			break;
	}

	return 0;
}

static void InitializeSerialNumber()
{
	Serial_Code_t serial_code;
	uint8_t length = 0;
	//length = sizeof(g_pSerialNumberString);

	Read_Serial_Code(&serial_code);

	for(int index = 0;index < SERIAL_NUM_LEN && serial_code.serial_num[index] != '\0'; index++) {
		g_pSerialNumberString[(index * 2) + 2] = serial_code.serial_num[index];
		g_pSerialNumberString[(index * 2) + 3] = 0;
		length++;
	}

	g_pSerialNumberString[0] = 2 + (length * 2);
}

uint8_t DeviceCommandBuffered()
{
	uint8_t result = 0xFF;
	uint32_t buffer_index;

	tUSBRingBufObject ring_buffer;

	if(USBBufferDataAvailable(&g_sRxBuffer) >= USB_COMMAND_SIZE)
	{
		// Check for a Cardea command prefix
		USBBufferInfoGet(&g_sRxBuffer, &ring_buffer);
		buffer_index = ring_buffer.ui32ReadIndex;

		if(ring_buffer.pui8Buf[buffer_index] == (CARDEA_COMMAND_BASE & 0x00FF0000) >> 16)
		{
			buffer_index++;

			if(buffer_index >= USB_BUFFER_SIZE)
			{
				buffer_index -= USB_BUFFER_SIZE;
			}

			if(ring_buffer.pui8Buf[buffer_index] == (CARDEA_COMMAND_BASE & 0x0000FF00) >> 8)
			{
				buffer_index++;

				if(buffer_index >= USB_BUFFER_SIZE)
				{
					buffer_index -= USB_BUFFER_SIZE;
				}

				if(ring_buffer.pui8Buf[buffer_index] == (CARDEA_COMMAND_BASE & 0x000000FF))
				{
					buffer_index++;

					if(buffer_index >= USB_BUFFER_SIZE)
					{
						buffer_index -= USB_BUFFER_SIZE;
					}

					result = ring_buffer.pui8Buf[buffer_index];
				}
			}
		}
	}

	return result;
}

void USB_Process_Commands()
{
	uint8_t next_command_id;
    CardeaDeviceCommand_t command;

	if(USBBufferDataAvailable(&g_sRxBuffer) < USB_COMMAND_SIZE)
	{
		return;
	}

	next_command_id = DeviceCommandBuffered();

	if(next_command_id != 0xFF)
	{
		if(next_command_id == ((CARDEA_CMD_SET_DEVICE_ID_INFO & 0xFF000000) >> 24))
		{
			if(USBBufferDataAvailable(&g_sRxBuffer) >= sizeof(CardeaCmdSetDeviceIdInfo_t))
			{
				USBBufferRead(&g_sRxBuffer, (uint8_t*)&device_info_packet, sizeof(CardeaCmdSetDeviceIdInfo_t));

				command.command = device_info_packet.cmd;
				command.param = device_info_packet.param;

				Process_Command(&command);
			}
		}
		else
		{
			USBBufferRead(&g_sRxBuffer, (uint8_t*)&command, USB_COMMAND_SIZE);

			Process_Command(&command);
		}
	}
	else
	{
		// The data is either a firmware update packet or the buffer is corrupt.
		// We will assume firmware update as it will verify CRCs.
		if(USBBufferDataAvailable(&g_sRxBuffer) >= sizeof(CardeaFirmwareUpdateBlock_t))
		{
			USBBufferRead(&g_sRxBuffer, (uint8_t*)&firmware_packet, sizeof(CardeaFirmwareUpdateBlock_t));

			command.command = CARDEA_CMD_UPDATE_FIRMWARE;
			command.param = 0;
			Process_Command(&command);
		}
	}
}

void USB_Queue_Packet(ECG_Packet_t* packet)
{
	if(packet == NULL)
	{
		Display(("Cannot add NULL packet!\r\n"));
		return;
	}

	USBBufferWrite(&g_sTxBuffer, (uint8_t*)packet, sizeof(ECG_Packet_t));
}

uint8_t USB_Send_Command_Response(void *response_data, uint32_t response_size)
{
    uint32_t bytes_sent;

	bytes_sent = USBBufferWrite(&g_sTxBuffer, response_data, response_size);

    if(bytes_sent != response_size)
        return 0;
    else
        return 1;
}

void USB_Get_Firmware_Packet(void *packet_ptr)
{
    BTPS_MemCopy(packet_ptr, &firmware_packet, sizeof(CardeaFirmwareUpdateBlock_t));
}

void USB_Get_Device_Info_Packet(CardeaCmdSetDeviceIdInfo_t * packet_ptr)
{
    BTPS_MemCopy(packet_ptr, &device_info_packet, sizeof(CardeaCmdSetDeviceIdInfo_t));
}

static void USB_GPIO_Init()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_USB0);
	GPIOPinTypeUSBAnalog(GPIO_PORTL_BASE, GPIO_PIN_6 | GPIO_PIN_7);
//	GPIOPinTypeGPIOInput(GPIO_PORTQ_BASE, GPIO_PIN_4);

	GPIOIntTypeSet(GPIO_PORTQ_BASE, GPIO_INT_PIN_6, GPIO_LOW_LEVEL | GPIO_DISCRETE_INT);
	GPIOIntDisable(GPIO_PORTQ_BASE, GPIO_INT_PIN_6);
}

void USBActivityIntHandler(void)
{
	// This should be called when activity is detected while we are suspended
	GPIOIntClear(GPIO_PORTQ_BASE, GPIO_INT_PIN_6);
	g_bSleepRequested = false;
	IntDisable(INT_GPIOQ6);
	GPIOIntDisable(GPIO_PORTQ_BASE, GPIO_INT_PIN_6);
}

void USB_Reset()
{
	g_bConnected = 0;
	USBDCDTerm(0);
	ECG_USB_Init();
}

void ECG_USB_Init()
{
	uint32_t ui32PLLRate = 480000000;

	USB_GPIO_Init();

	InitializeSerialNumber();

	// Initialize USB Buffers
	USBBufferInit((tUSBBuffer*)&g_sTxBuffer);
	USBBufferInit((tUSBBuffer*)&g_sRxBuffer);

	// Initialize USB Device
	USBStackModeSet(0, eUSBModeForceDevice, NULL);

	USBDCDFeatureSet(0, USBLIB_FEATURE_CPUCLK, &g_ui32ClockFrequency);
	USBDCDFeatureSet(0, USBLIB_FEATURE_USBPLL, &ui32PLLRate);
	
	g_pUSBDevice = USBDCDCInit(0, &g_sCDCDevice);

	if(g_pUSBDevice == NULL)
	{
		Display(("Error initializing USB interface"));
	}
}
