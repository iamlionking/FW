#ifndef __ECG_USB_H__
#define __ECG_USB_H__

#define USB_COMMAND_SIZE 8

#define USB_VID_CARDEA		0x1CBE
#define USB_PID_RUBY		0x0208

#include <stdint.h>

#include "device_api.h"
#include "ecg_packet.h"

void ECG_USB_Init(void);
void USB_Reset(void);
void USB_Process_Commands(void);
void USB_Queue_Packet(ECG_Packet_t* packet);
uint8_t USB_Send_Command_Response(void *response_data, uint32_t response_size);
void USB_Get_Firmware_Packet(void *packet_ptr);
void USB_Get_Device_Info_Packet(CardeaCmdSetDeviceIdInfo_t * packet_ptr);

#endif
