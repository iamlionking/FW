#ifndef __ECG_PACKET_H__
#define __ECG_PACKET_H__

/* ECG Data Size Constants */
#define ADC_NUM_CHANNELS					8						
#define ADC_BITS_PER_CHANNEL_COMPRESSED		16
#define ADC_BITS_PER_CHANNEL_RAW		 	24
#define ADC_DATA_SIZE_COMPRESSED 			8  //( ADC_BITS_PER_CHANNEL_COMPRESSED / 8) * ADC_NUM_CHANNELS
#define ADC_DATA_SIZE_RAW		 			24 //( ADC_BITS_PER_CHANNEL_RAW / 8) * ADC_NUM_CHANNELS
#define ADC_STATUS_SIZE						4

#define COMPRESSED_DATA_SIZE				ADC_STATUS_SIZE + ADC_DATA_SIZE_COMPRESSED
#define RAW_DATA_SIZE						ADC_STATUS_SIZE + ADC_DATA_SIZE_RAW

// check the status bits
// status bits:
//     31 - 30     29      28       27 - 16         15 - 9            8 - 0
//     Reserved  Voltage  Pacer    Sequence #       Reserved        Lead-off  (bit 0 = lead I, 1 = II, etc)

// 31 30 29 28  27 26 25 24  23 22 21 20  19 18 17 16  15 14 13 12  11 10 9 8  7 6 5 4  3 2 1 0
// 0  0  0  0   0  0  0  0   0  0  0  0   0  0  0  0   0  0  0  0   0  0  0 0  0 0 0 0  0 0 0 0

#define TRACE_OVERLOAD_SHIFT        8
#define SEQUENCE_NUM_SHIFT          16 
#define PACER_SHIFT                 28
#define VOLTAGE_SHIFT				29

#define LEAD_OFF_STATUS_MASK        0x000001FF
#define SEQUENCE_NUM_MASK           0x0FFF0000
#define PACER_BIT_MASK              0x10000000
#define VOLTAGE_BIT_MASK			0x20000000

typedef struct ECG_Packet_Struct_t
{
    uint32_t            status;  //4
    uint16_t            lsw[8];  //16
    int8_t              msb[8];  //8
} ECG_Packet_t;

#define ECG_PUT_STATUS_DATA(status, data, mask, shift)      \
{                                                           \
status &= ~mask;                                            \
status |= (data << shift);                                  \
}                   

#define ECG_GET_STATUS_DATA(status, mask, shift)   (status & mask) >> shift

#define ECG_PACKET_COMPRESSED_SIZE 	SEQ_NUM_SIZE + ADC_STATUS_SIZE + ADC_DATA_SIZE_COMPRESSED
#define ECG_PACKET_RAW_SIZE			sizeof(ECG_Packet_t);

#define ECG_PACKET_GET_COMPRESSED_DATA( packet ) packet->Packet_Data
#define ECG_PACKET_GET_RAW_DATA( packet ) packet->Packet_Data

#define ECG_PACKET_PUT_COMPRESSED_DATA( packet, data, data_sz ) memcpy( packet->Packet_Data, data, data_size )
#define ECG_PACKET_PUT_RAW_DATA( packet, data, data_sz ) memcpy( packet->Packet_Data, data, data_size )

#endif
