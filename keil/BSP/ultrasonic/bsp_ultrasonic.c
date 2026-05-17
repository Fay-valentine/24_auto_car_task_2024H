#include "bsp_ultrasonic.h"

volatile uint32_t msHcCount=0;//系统时基，1ms自增1
volatile uint32_t systick_counter = 0; // 另一个计数器（预留）

uint32_t Get_Time(void)
{
    return msHcCount;   // 返回系统运行毫秒数
}

void Ultrasonic_Init(void)
{
    NVIC_ClearPendingIRQ(TIMER_1ms_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_1ms_INST_INT_IRQN);
    DL_TimerA_startCounter(TIMER_1ms_INST);   // 启动定时器计数
}

void TIMA1_IRQHandler(void)
{
    switch ((DL_TimerA_getPendingInterrupt(TIMER_1ms_INST)))
    {
    case DL_TIMERA_IIDX_LOAD:
        msHcCount++;
        Global_Tick();
        DL_TimerA_clearInterruptStatus(TIMER_1ms_INST, DL_TIMERA_INTERRUPT_LOAD_EVENT);
        break;
    case DL_TIMERA_IIDX_ZERO:
        systick_counter++;//预留
        DL_TimerA_clearInterruptStatus(TIMER_1ms_INST, DL_TIMERA_INTERRUPT_ZERO_EVENT);
        break;

    default:
        break;
    }
}
