.syntax unified
.cpu cortex-m3
.thumb_func

.global _bus_fault_handler_asm
.extern bus_fault_handler_c
 
_bus_fault_handler_asm:
  TST LR, #4
  ITE EQ
  MRSEQ R0, MSP
  MRSNE R0, PSP
  B bus_fault_handler_c