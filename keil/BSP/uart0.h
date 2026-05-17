#ifndef __UART0_H__
#define __UART0_H__

#include "AllHeader.h"

void UART_Init(void);
void uart0_send_char(char ch);
void uart0_send_string(char* str);


#endif
