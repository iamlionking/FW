#ifndef __TEST_TRANSFER_H__
#define __TEST_TRANSFER_H__

void Test_Transfer_Start(uint32_t rate);
void Test_Transfer_Stop();
uint8_t Test_Transfer_In_Progress();
void Test_Transfer_Queue_Data();

#endif