#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

void __bus_fault(void);
void __hard_fault(void);
void __usage_fault(void);

/*
extern unsigned int  xxxLockCount;
extern unsigned int  xxxHCICOMM;
extern unsigned int  xxxDebug_HCI;
extern unsigned int  xxxDebug_Line;
extern unsigned int  xxxDebug_Func;*/

#endif