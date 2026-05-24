#include "ti_msp_dl_config.h"
#include "AllHeader.h"

#define DELAY_SELECT 1
volatile unsigned int delay_times = 0;

#if DELAY_SELECT==0
void delay_ms(unsigned int ms)
{
	unsigned int i,j;
	for(i=0;i<ms;i++)
	{
		for(j=0;j<8000;j++)
		{
			__asm__("nop");
		}
	}
}

#elif DELAY_SELECT==1
//ͨ��systick��ѯ��ȡ��ʵ����ʱ 
void delay_us(unsigned long __us) 
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;

    ticks = __us * (80000000 / 1000000);  // 80MHz ʱ��  
    told = SysTick->VAL;

    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow;
            else
                tcnt += (SysTick->LOAD + 1) - tnow + told;

            told = tnow;
            if (tcnt >= ticks)
                break;
        }
    }
}
//����δ�ʱ��ʵ�ֵľ�ȷms��ʱ 
void delay_ms(unsigned long ms) 
{
    while (ms--)
    {
        delay_us(1000);  // ÿ����ʱ 1ms
    }
}
#else
//ͨ��systick�ж�ʵ����ʱ ��Ҫ��sysconfig�д�systick�ж�  
void delay_ms(unsigned int ms)
{
    delay_times=ms;
    while(delay_times!=0);
}


void SysTick_Handler(void)
{
        if( delay_times != 0 )
        {
                delay_times--;
        }
}
#endif
