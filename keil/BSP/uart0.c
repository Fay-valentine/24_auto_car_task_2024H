#include "uart0.h"
#include "AllHeader.h"

void UART_Init(void)
{
	//清除中断请求标志
	NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
	//使能中断
	NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
}

//串口发送单个字符
void uart0_send_char(char ch)
{
	//等忙
	while(DL_UART_isBusy(UART_0_INST)==true);
	//发送单个字符
	DL_UART_Main_transmitData(UART_0_INST,ch);
}

//串口发送字符串   
void uart0_send_string(char* str)
{
	//字符串不为
	while(str!=NULL&&*str!='\0')
	{
		uart0_send_char(*str++);
	}
}

#if !defined(__MICROLIB)
//不使用微库的话就需要添加下面的函数
//If you don't use the micro library, you need to add the following function
#if (__ARMCLIB_VERSION <= 6000000)
//如果编译器是AC5  就定义下面这个结构体
//If the compiler is AC5, define the following structure
struct __FILE
{
	int handle;
};
#endif

FILE __stdout;

//定义_sys_exit()以避免使用半主机模式
//Define _sys_exit() to avoid using semihosting mode
void _sys_exit(int x)
{
	x = x;
}
#endif

//重定向函数

int fputc(int ch,FILE* stream)
{
	uart0_send_char(ch);
	return ch;
}

int fputs(const char* restrict s, FILE* restrict stream)
{
	uint16_t i,len;
	len=strlen(s);
	for(i=0;i<len;i++)
	{
		uart0_send_char(s[i]);
	}
	return len;
}

int puts(const char *_ptr)
{
	int count=fputs(_ptr,stdout);
	count+=fputs("\n",stdout);
	return count;
}
