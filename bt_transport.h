#ifndef __BT_TRANSPORT_H__
#define __BT_TRANSPORT_H__

#include "device_api.h"
#include "ecg_packet.h"

#define MAX_NUMBER_DATA_PACKETS			6
#define QUEUED_DATA_PACKETS_THRESHOLD	2

/* NOTE: The following condition must hold true for valid packet delivery:
 * ( SPP_TRANSMIT_BUFFER_SIZE / SPP_MAXIMUM_FRAME_SIZE ) <= ( MAX_NUMBER_DATA_PACKETS - QUEUED_DATA_PACKETS_THRESHOLD )
 */
#define SPP_MAXIMUM_FRAME_SIZE			1000
#define SPP_TRANSMIT_BUFFER_SIZE		4000
#define SPP_RECEIVE_BUFFER_SIZE			6000

#define SPP_RADIO_COMMUNICATION_SPEED	3000000	

#define SPP_PACKET_BUFFER_SIZE			16384
#define SPP_COMMAND_BUFFER_SIZE         80
#define BT_COMMAND_SIZE                 8


extern bool g_sppPortConnected;
extern int g_iTotalBytesSent;

void BTT_Init();
void BTT_Close_Server();
void BTT_Queue_Packet(ECG_Packet_t *packet);
void BTT_Transport_Packets();
/* Change to boolean later once you know what header file the type is in */
char BTT_Queue_Is_Full();
void BTT_Process_Commands();
void BTT_Clear_Buffer();
uint8_t BTT_Send_Command_Response(void *response_data, uint32_t response_size);
void BTT_Get_Firmware_Packet(void *packet_ptr);
void BTT_Get_Device_Info_Packet(CardeaCmdSetDeviceIdInfo_t * packet_ptr);

#endif
