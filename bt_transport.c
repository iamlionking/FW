#include <math.h>
#include <string.h>

#include "sapphire_pub.h"

#include "inc/hw_types.h"
#include "utils/ringbuf.h"
#include "bluetooth.h"
#include "device_api.h"
#include "bt_transport.h"
#include "command_proc.h"
#include "ecg_adc.h"
#include "ecg_data_capture.h"
#include "ecg_gpio.h"
#include "ecg_init.h"
#include "ecg_packet.h"
#include "ecg_status.h"
#include "firmware_download.h"
#include "gpio.h"
#include "interrupt.h"

#include "ADS1x9x.h"
#include "BTVSAPI.h"
#include "RFCOMAPI.h"
#include "SPPAPI.h"
#include "HCIAPI.h"

int g_iTotalBytesSent;
static bool spp_tx_busy;

/* Bluetooth Transport SPP Server ID */
static uint32_t iBTTServerID;

/* Packet Queue */
static uint8_t packet_buf[SPP_PACKET_BUFFER_SIZE];
static tRingBufObject packet_buffer;

/* Command Queue */
static uint8_t command_buf[SPP_COMMAND_BUFFER_SIZE];
static tRingBufObject command_buffer;

/* Firmware Download Packet */
static CardeaFirmwareUpdateBlock_t firmware_packet;

/* Device ID Packet */
static CardeaCmdSetDeviceIdInfo_t device_info_packet;

/* Packet buffer */
static uint8_t packets[SPP_PACKET_BUFFER_SIZE];

bool g_sppPortConnected;
static uint32_t data_len;

static void init_ring_buffers();
static void init_spp();
static void spp_event_callback(unsigned int BluetoothStackID, SPP_Event_Data_t *SPP_Event_Data, unsigned long CallbackParameter);
static void hci_event_callback(unsigned int BluetoothStackID, HCI_Event_Data_t *HCI_Event_Data, unsigned long CallbackParameter);
static void start_spp_server();


/* Public - Initialize the Bluetooth Transport layer
 * 
 * Examples
 * 
 *      BTT_Init();
 * 
 * Returns nothing
 */
void BTT_Init()
{
	g_sppPortConnected = false;
	spp_tx_busy = false;
    data_len = 0;
	
    /* Initialize Bluetooth Transport */
    init_ring_buffers();
	
	init_spp();
    
    start_spp_server();
    
    BTPS_MemInitialize(packets, 0x00, sizeof(packets));
} /* BTT_Init() */


void BTT_Close_Server()
{
    SPP_Close_Port(g_uiBluetoothStackID, iBTTServerID);
    SPP_Close_Server_Port(g_uiBluetoothStackID, iBTTServerID);
    BTT_Init();
}


/* Public - Queues an ecg data packet for later transmission.
 * 
 * Examples
 * 
 *      ECG_Packet_t packet;
 * 
 *      //Fill packet data here
 *      BTT_Queue_Packet(&packet);
 * 
 * Returns nothing
 */
void BTT_Queue_Packet(ECG_Packet_t* packet)
{
	if(packet == NULL)
	{
		Display(("Cannot add NULL packet!\r\n"));
		return;
	}	
	
	RingBufWrite(&packet_buffer, (uint8_t*)&packet->status, 4);
    RingBufWrite(&packet_buffer, (uint8_t*)&packet->lsw, 16);
    RingBufWrite(&packet_buffer, (uint8_t*)&packet->msb, 8);
    
} /* BTT_Queue_Packet() */


/* Public - Clears the packet buffer
 * 
 * Examples
 * 
 *      BTT_Clear_Buffer();
 * 
 * Returns nothing
 */
void BTT_Clear_Buffer()
{
    RingBufFlush(&packet_buffer);
    spp_tx_busy = false;
} /* BTT_Clear_Buffer() */


/* Public - Transports the packets in the packet queue over the Bluetooth SPP data connection.
 * This function is typically called at some slower rater than BTT_Queue_Packet.
 * 
 * Examples
 *
 *      BTT_Transport_Packets();
 * 
 * Returns nothing
 */
void BTT_Transport_Packets()
{
	int bytes_txd = 0;
    DWord_t iTotalBytes;
    
	/* TX is busy until we either write all bytes or the Transmit Buffer Empty event occurs */
	if(spp_tx_busy)
	{
		return;
	}
	
    iTotalBytes = RingBufUsed(&packet_buffer);
    if(iTotalBytes == 0)
    {
        return;
    }
    
	RingBufRead(&packet_buffer, packets, iTotalBytes);
    
    spp_tx_busy = true;
	
	/* Transmit data */
	bytes_txd = /*iTotalBytes;*/SPP_Data_Write(g_uiBluetoothStackID, iBTTServerID, iTotalBytes, packets);
	if(bytes_txd > 0)
    {
        if(bytes_txd == iTotalBytes)
		{
			spp_tx_busy = false;
		}
        else
        {
            /* Write Data not sent back to ring buffer */
            RingBufWrite(&packet_buffer, &packets[bytes_txd], iTotalBytes - bytes_txd);
        }
    }
	else
	{
        Display(("SPP Write Error!\r\n"));
        ECG_Data_Capture_Hard_Stop();
	}
} /* BTT_Transport_Packets() */


/* Public - Processes commands in the command queue sent from the PC application
 * This function is typically called at some interval.
 * 
 * Examples
 * 
 *      //Inside main loop...
 *      BTT_Process_Commands();
 * 
 * Returns nothing
 */
void BTT_Process_Commands()
{
    uint8_t i;
    uint8_t num_commands;
    CardeaDeviceCommand_t command;
    
    num_commands = RingBufUsed(&command_buffer) / BT_COMMAND_SIZE;
    if(num_commands == 0)
        return;
        
    for(i = 0; i < num_commands; i++)
    {
        RingBufRead(&command_buffer, (uint8_t*)&command, BT_COMMAND_SIZE);
        Process_Command(&command);
    }
} /* BTT_Process_Command() */


/* Public - Sends a response to a command from the PC application.  This is called
 * during the processing of a command that requires a response be sent to the PC
 * application.
 * 
 * Examples
 * 
 *      //Inside a command that requires a response
 *      CardeaCmdGetRevisionResponse_t response;
 *      //Fill response data
 *      BTT_Send_Command_Response(&response, sizeof(CardeaCmdGetRevisionResponse_t));
 * 
 * Returns 1 if the response was successfully written, 0 otherwise
 */
uint8_t BTT_Send_Command_Response(void *response_data, uint32_t response_size)
{
    uint32_t bytes_sent;
    
    bytes_sent = SPP_Data_Write(g_uiBluetoothStackID, iBTTServerID, response_size, response_data);
    if(bytes_sent != response_size)
        return 0;
    else
        return 1;
} /* BTT_Send_Command_Response() */


/* Public - Retrieves the current firmware packet available for writting to the
 * external flash.
 * 
 * Examples
 * 
 *      CardeaFirmwareUpdateBlock_t packet_ptr;
 * 
 *      BTT_Get_Firmware_Packet(&packet_ptr);
 *      //Write to flash
 * 
 * Returns nothing
 */
void BTT_Get_Firmware_Packet(void *packet_ptr)
{
    BTPS_MemCopy(packet_ptr, &firmware_packet, sizeof(CardeaFirmwareUpdateBlock_t));
} /* BTT_Get_Firmware_Packet() */


/* Public - Retrieves the current device info packet
 * 
 * Examples
 * 
 *      CardeaCmdSetDeviceIdInfo_t dev_info;
 *      BTT_Get_Device_Info_Packet(&dev_info);
 * 
 * Returns nothing
 */
void BTT_Get_Device_Info_Packet(CardeaCmdSetDeviceIdInfo_t * packet_ptr)
{
    BTPS_MemCopy(packet_ptr, &device_info_packet, sizeof(CardeaCmdSetDeviceIdInfo_t));
} /* BTT_Get_Device_Info_Packet() */


/* Internal - Handles the HCI events generated by the Bluetooth stack
 * 
 * Returns nothing
 */
static void hci_event_callback(unsigned int BluetoothStackID, HCI_Event_Data_t *HCI_Event_Data, unsigned long CallbackParameter)
{
    switch(HCI_Event_Data->Event_Data_Type)
    {
        case etConnection_Complete_Event:
            Set_HCI_Connection_Handle(HCI_Event_Data->Event_Data.HCI_Connection_Complete_Event_Data->Connection_Handle);
            break;
        default:
            break;
    }
} /* hci_event_callback() */


/* Internal - Initializes the ring buffers used for queueing ecg data packets and commands
 * 
 * Returns nothing
 */
static void init_ring_buffers()
{
    /* Initialize Packet Buffer */
    BTPS_MemInitialize(packet_buf, 0x00, SPP_PACKET_BUFFER_SIZE);
	RingBufInit(&packet_buffer, packet_buf, SPP_PACKET_BUFFER_SIZE);
    
    /* Initialize Command Buffer */
    BTPS_MemInitialize(command_buf, 0x00, SPP_COMMAND_BUFFER_SIZE);
    RingBufInit(&command_buffer, command_buf, SPP_COMMAND_BUFFER_SIZE);
} /* init_ring_buffers() */


/* Internal - Initializes the SPP communication interface
 * 
 * Returns nothing
 */
static void init_spp()
{
    SPP_Configuration_Params_t SPPConfigParams;
    
    SPPConfigParams.MaximumFrameSize = SPP_MAXIMUM_FRAME_SIZE;
	SPPConfigParams.TransmitBufferSize = SPP_TRANSMIT_BUFFER_SIZE;
	SPPConfigParams.ReceiveBufferSize = SPP_RECEIVE_BUFFER_SIZE;
	
	SPP_Set_Configuration_Parameters(g_uiBluetoothStackID, &SPPConfigParams);
	SPP_Set_Queuing_Parameters(g_uiBluetoothStackID, MAX_NUMBER_DATA_PACKETS, QUEUED_DATA_PACKETS_THRESHOLD);
	
	/* Initialize Bluetooth Radio Communication Speed */
	VS_Update_UART_Baud_Rate(g_uiBluetoothStackID, SPP_RADIO_COMMUNICATION_SPEED);
} /* init_spp() */


/* Internal - Handles SPP events generated by the Bluetooth stack
 * 
 * Returns nothing
 */
static void spp_event_callback(unsigned int BluetoothStackID, SPP_Event_Data_t *SPP_Event_Data, unsigned long CallbackParameter)
{
	static Byte_t temp[512];
	int data_rx;
	
	switch(SPP_Event_Data->Event_Data_Type)
	{
	case etPort_Open_Indication:
		Display(("Port Connection made\r\n"));
		SPP_Open_Port_Request_Response(1, iBTTServerID, TRUE);
        g_sppPortConnected = 1;
		break;
	case etPort_Close_Port_Indication:
		Display(("Port Connection close\r\n"));
		SPP_Close_Port(g_uiBluetoothStackID, iBTTServerID);
        g_sppPortConnected = 0;
        if(ECG_Data_Capture_In_Progress() == 1)
            ECG_Data_Capture_Hard_Stop();
		break;
	case etPort_Data_Indication:
		data_rx = SPP_Event_Data->Event_Data.SPP_Data_Indication_Data->DataLength;
		SPP_Data_Read(g_uiBluetoothStackID, iBTTServerID, data_rx, &temp[data_len]);
        data_len += data_rx;
        
        if(data_len == sizeof(CardeaDeviceCommand_t))
        {
            Display(("Received data: %x%x%x%x%x%x%x%x\r\n", temp[0],temp[1],temp[2],temp[3],temp[4],temp[5],temp[6],temp[7]));
            RingBufWrite(&command_buffer, temp, data_len);
            data_len = 0;
        }
        else if(data_len == sizeof(CardeaFirmwareUpdateBlock_t))
        {
            CardeaDeviceCommand_t command;
            command.command = CARDEA_CMD_UPDATE_FIRMWARE;
            command.param = 0;
            
            RingBufWrite(&command_buffer, (uint8_t*)&command, sizeof(CardeaDeviceCommand_t));
            BTPS_MemCopy(&firmware_packet, temp, data_len);
            data_len = 0;
		}
        else if(data_len == sizeof(CardeaCmdSetDeviceIdInfo_t))
        {
            CardeaDeviceCommand_t command;
            
            BTPS_MemCopy(&device_info_packet, temp, data_len);
            
            command.command = device_info_packet.cmd;
            command.param = device_info_packet.param;
            
            RingBufWrite(&command_buffer, (uint8_t*)&command, sizeof(CardeaDeviceCommand_t));
            
            data_len = 0;
        }
		break;
	case etPort_Transmit_Buffer_Empty_Indication:
		spp_tx_busy = false;
		break;
	default:
		break;
	}
} /* spp_event_callback() */


/* Internal - Starts the SPP server necessary for receiving SPP data connections 
 * from the PC application.
 * 
 * Returns nothing
 */
static void start_spp_server()
{
    /* Local Variables */
    int iRetVal;
	DWord_t sdp_hndl;
    
    HCI_Register_Event_Callback(g_uiBluetoothStackID, hci_event_callback, 0);
    iBTTServerID = SPP_Open_Server_Port(g_uiBluetoothStackID, RFCOMM_MINIMUM_SERVER_CHANNEL_ID, spp_event_callback, 0);
	if(iBTTServerID == 0)
	{
        Display(("Failed to open SPP Server Port"));
        return;
    }
		
    iRetVal = SPP_Register_SDP_Record(g_uiBluetoothStackID, iBTTServerID, NULL, "ECG Connection", &sdp_hndl);
    if(iRetVal > 0)
    {
        Display(("SDP Record Register Failed...\r\n"));
        return;
    }
} /* start_spp_server() */
