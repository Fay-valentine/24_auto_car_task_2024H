#include "uart0.h"
#include "AllHeader.h"



void UART_Init(void)
{
	//����ж������־
	NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
	//ʹ���ж�
	NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
}

//���ڷ��͵����ַ�
void uart0_send_char(char ch)
{
	//��æ
	while(DL_UART_isBusy(UART_0_INST)==true);
	//���͵����ַ�
	DL_UART_Main_transmitData(UART_0_INST,ch);
}

//���ڷ����ַ���   
void uart0_send_string(char* str)
{
	//�ַ�����Ϊ
	while(str!=NULL&&*str!='\0')
	{
		uart0_send_char(*str++);
	}
}

#if !defined(__MICROLIB)
//��ʹ��΢��Ļ�����Ҫ��������ĺ���
//If you don't use the micro library, you need to add the following function
#if (__ARMCLIB_VERSION <= 6000000)
//�����������AC5  �Ͷ�����������ṹ��
//If the compiler is AC5, define the following structure
struct __FILE
{
	int handle;
};
#endif

FILE __stdout;

//����_sys_exit()�Ա���ʹ�ð�����ģʽ
//Define _sys_exit() to avoid using semihosting mode
void _sys_exit(int x)
{
	x = x;
}
#endif

//�ض�����

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



