#ifndef __BSP_IR_EIGHT_H__
#define __BSP_IR_EIGHT_H__

#include "AllHeader.h"

extern volatile uint8_t  IR_recv0_complete_flag;
#define IR_NUM  8//红外探头数量
extern volatile uint8_t IR_Data_Number[];
extern volatile u8 oledbuf[];

void IR_UART_Init(void);
void IR_DataAnalysis(void);

#endif