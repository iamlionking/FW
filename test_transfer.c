#include "sapphire_pub.h"

#include "BKRNLAPI.h"
#include "ecg_packet.h"
#include "bt_transport.h"
#include "test_transfer.h"

static uint8_t in_progress = 0;
static uint32_t data_rate;
static uint16_t packet_num;
static uint8_t transfer_tick;

static ECG_Packet_t packet;

void Test_Transfer_Start(uint32_t rate)
{
    uint8_t i;
    data_rate = rate;
    in_progress = 1;
    packet_num = 0;
    
    packet.status = 0x0fffffff;
    
    for(i = 0; i < 8; i++)
    {
        packet.msb[i] = i + 1;
        packet.lsw[i] = 0x0000;
    }
    
    if(data_rate == 250)
        transfer_tick = 4;
    else if(data_rate == 500)
        transfer_tick = 2;
    else if(data_rate == 1000 || data_rate == 2000)
        transfer_tick = 1;
}

void Test_Transfer_Stop(void)
{
    in_progress = 0;
}

uint8_t Test_Transfer_In_Progress(void)
{
    return in_progress;
}

void Test_Transfer_Queue_Data(void)
{
    if(--transfer_tick == 0)
    {
        ECG_PUT_STATUS_DATA(packet.status, packet_num, SEQUENCE_NUM_MASK, SEQUENCE_NUM_SHIFT);
        BTT_Queue_Packet(&packet);
        if(data_rate == 250)
            transfer_tick = 4;
        else if(data_rate == 500)
            transfer_tick = 2;
        else if(data_rate == 1000 || data_rate == 2000)
            transfer_tick = 1;
        packet_num++;
        if(packet_num > 4095)
            packet_num = 0;
            
        if(data_rate == 2000)
        {
            ECG_PUT_STATUS_DATA(packet.status, packet_num, SEQUENCE_NUM_MASK, SEQUENCE_NUM_SHIFT);
            BTT_Queue_Packet(&packet);
            packet_num++;
            if(packet_num > 4095)
                packet_num = 0;
        }
    }
}

void data_ready()
{
    
}
