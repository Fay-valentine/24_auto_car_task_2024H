#include "bsp_timer.h"

#if Timer_20ms_Switch

//20ms定时器
void Timer_20ms_Init(void)
{
    //打开20ms定时器
    NVIC_ClearPendingIRQ(TIMER_20ms_INST_INT_IRQN);
	  NVIC_EnableIRQ(TIMER_20ms_INST_INT_IRQN);
		DL_TimerG_startCounter(TIMER_20ms_INST);
}

u8 gled_cnt = 0;
//20ms定时器中断
void TIMER_20ms_INST_IRQHandler(void)
{
    //20ms归零中断触发
	if( DL_TimerG_getPendingInterrupt(TIMER_20ms_INST) == DL_TIMER_IIDX_ZERO )
	{
		IR_DataAnalysis();//八路红外模块数据处理
		Encoder_Update();//编码器更新
        Motion_Handle(); //小车驱动
        gled_cnt++;
        if(gled_cnt>=20)//400ms
        {
            gled_cnt=0;
            DL_GPIO_togglePins(LED_PORT,LED_MCU_PIN);
        }
        DL_TimerG_clearInterruptStatus(TIMER_20ms_INST, DL_TIMERG_INTERRUPT_ZERO_EVENT); // 新增
	}

}

#endif

#if Timer_1ms_Switch

volatile uint32_t msHcCount=0;//系统时基，1ms自增1
volatile uint32_t systick_counter = 0; // 另一个计数器（预留）

uint32_t Get_Time(void)
{
    return msHcCount;   // 返回系统运行毫秒数
}

void Timer_1ms_Init(void)
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
        Key_Tick();//按键检测
        RGB_Tick();//RGB亮灭
        DL_TimerA_clearInterruptStatus(TIMER_1ms_INST, DL_TIMERA_INTERRUPT_LOAD_EVENT);
        break;
    // case DL_TIMERA_IIDX_ZERO:
    //     systick_counter++;//预留
    //     DL_TimerA_clearInterruptStatus(TIMER_1ms_INST, DL_TIMERA_INTERRUPT_ZERO_EVENT);
    //     break;

    default:
        break;
    }
}

#endif
