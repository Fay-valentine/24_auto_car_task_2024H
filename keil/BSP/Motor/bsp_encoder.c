#include "bsp_encoder.h"

//思路：车轮在运动时电机的AB相霍尔编码器会发送信号，通过中断不断读取并更新计数值，
//定时调用更新函数更新计数值，提供给上层处理

//这两个暂时没有被使用
int Encoder_L;//提供给外部使用的左轮速度（如PID）
int Encoder_R;//提供给外部使用的右轮速度

//仅本文件使用（静态变量会自动初始化为0，意味着结构体里的变量都会自动赋值0）
static Encoder_Res motorL_Encoder;//左轮状态
static Encoder_Res motorR_Encoder;//右轮状态

//编码器初始化
void Encoder_Init(void)
{
    //先清除中断，防止后面一使能就触发
    NVIC_ClearPendingIRQ(GPIO_MULTIPLE_GPIOB_INT_IRQN);
    NVIC_ClearPendingIRQ(GPIO_ENCODER_R_GPIOA_INT_IRQN);
    //使能中断
    NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOB_INT_IRQN);
    NVIC_EnableIRQ(GPIO_ENCODER_R_GPIOA_INT_IRQN);
}

//获取左右轮方向
Encoder_Dir get_encoderL_dir(void)
{
    return motorL_Encoder.dir;
}
Encoder_Dir get_encoderR_dir(void)
{
    return motorR_Encoder.dir;
}

/** 
*@brief 获取总计数(AllCount)
*@note [0]为左,[1]为右
*/
void Encoder_Get_ALL(int *Encoder_all)
{
    Encoder_all[0] = motorL_Encoder.AllCount;
    Encoder_all[1] = motorR_Encoder.AllCount;
}

/** 
*@brief 获取临时计数(temp_count)
*@note [0]为左,[1]为右
*/
void Encoder_Get_Temp(int *Encoder_temp)
{
    Encoder_temp[0] = motorL_Encoder.temp_count;
    Encoder_temp[1] = motorR_Encoder.temp_count;
}

//定时调用
void Encoder_Update(void)
{

    //把 中断计数(temp_count) 更新到 定时计数(count)
    motorL_Encoder.count=motorL_Encoder.temp_count;
    motorR_Encoder.count=motorR_Encoder.temp_count;

    //更新外部速度
    Encoder_L=motorL_Encoder.count/2;
    Encoder_R=motorR_Encoder.count/2;

    //更新 总计数(AllCount)
    motorL_Encoder.AllCount+=motorL_Encoder.count;
    motorR_Encoder.AllCount+=motorR_Encoder.count;

    //判断方向
    motorL_Encoder.dir=(motorL_Encoder.count>=0)?Forward:Reversal;
    motorR_Encoder.dir=(motorR_Encoder.count>=0)?Forward:Reversal;
	
	//清零中断计数，为下一次调用做准备
    motorL_Encoder.temp_count=0;
    motorR_Encoder.temp_count=0;
}

//
#if Motor_Switch==1

volatile uint32_t g_encoder_irq_count = 0;
//用于处理左右车轮编码器的信号
void GROUP1_IRQHandler(void)
{
	g_encoder_irq_count++;
    //1.先读取整个中断组，判断是GPIOA还是GPIOB
    uint32_t pendingSourse=DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1);

    //左轮
    if(pendingSourse & GPIO_MULTIPLE_GPIOB_INT_IIDX)//GPIOB
    {
        //获取GPIOB全部使能中断位
        uint32_t gpio_status=DL_GPIO_getEnabledInterruptStatus(GPIOB,0xFFFFFFFF);
        //2.再判断是GPIOA或GPIOB的哪个具体pin
        //H1A
        if(gpio_status & GPIO_ENCODER_L_H1A_PIN)//A相上升沿触发，B相0则正转，1则反转
        {
            if(!DL_GPIO_readPins(GPIOB,GPIO_ENCODER_L_H1B_PIN))
            {
                motorL_Encoder.temp_count++;
            }
            else
            {
                motorL_Encoder.temp_count--;
            }
            //清除中断位，防止重复进入中断
            DL_GPIO_clearInterruptStatus(GPIOB,GPIO_ENCODER_L_H1A_PIN);
        }
        //H1B
        else if(gpio_status & GPIO_ENCODER_L_H1B_PIN)//B相上升沿触发，A相0则反转，1则正转
        {
            if(DL_GPIO_readPins(GPIOB,GPIO_ENCODER_L_H1A_PIN))
            {
                motorL_Encoder.temp_count++;
            }
            else
            {
                motorL_Encoder.temp_count--;
            }
            //清除中断位，防止重复进入中断
            DL_GPIO_clearInterruptStatus(GPIOB,GPIO_ENCODER_L_H1B_PIN);
        }
    }
    
    //右轮
    //H2A
    if(pendingSourse & GPIO_MULTIPLE_GPIOB_INT_IIDX)//GPIOB
    {
        uint32_t gpio_status=DL_GPIO_getEnabledInterruptStatus(GPIOB,0xFFFFFFFF);
        if(gpio_status & GPIO_ENCODER_R_H2A_PIN)//A相上升沿触发，B相0则反转，1则正转
        {
            if(DL_GPIO_readPins(GPIOA,GPIO_ENCODER_R_H2B_PIN))
            {
                motorR_Encoder.temp_count++;
            }
            else
            {
                motorR_Encoder.temp_count--;
            }
            //清除中断位，防止重复进入中断
            DL_GPIO_clearInterruptStatus(GPIOB,GPIO_ENCODER_R_H2A_PIN);
        }
    }

    //H2B
    if(pendingSourse & GPIO_ENCODER_R_GPIOA_INT_IIDX)//GPIOA
    {
        uint32_t gpio_status=DL_GPIO_getEnabledInterruptStatus(GPIOA,GPIO_ENCODER_R_H2B_PIN);
        if(gpio_status & GPIO_ENCODER_R_H2B_PIN)//B相上升沿触发，A相0则正转，1则反转
        {
            if(!DL_GPIO_readPins(GPIOB,GPIO_ENCODER_R_H2A_PIN))
            {
                motorR_Encoder.temp_count++;
            }
            else
            {
                motorR_Encoder.temp_count--;
            }
            //清除中断位，防止重复进入中断
            DL_GPIO_clearInterruptStatus(GPIOA,GPIO_ENCODER_R_H2B_PIN);
        }
    }
    
}




#endif