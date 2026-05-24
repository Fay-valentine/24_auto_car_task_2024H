#include "bsp_motor.h"
#include "AllHeader.h"

//1.PWM启动
void Init_Motor_PWM()
{
    //开启定时器计数器，启动PWM输出
    DL_TimerA_startCounter(motor_PWM_INST);
}

//2.死区补偿
/** 
*@brief 将期望的PWM补偿上死区的PWM值，防止无法启动
*@param pulse 期望的PWM值
*@return 补偿后的PWM值
*/
static int16_t Motor_Ignore_Dead_Zone(int16_t pulse)
{
    if(pulse>0)
    {
        return pulse+MOTOR_DEAD_ZONE;//正向
    }
    if(pulse<0)
    {
        return pulse-MOTOR_DEAD_ZONE;//反向
    }
	return 0;
}
//3.速度限制
/**
 * @brief   将速度/占空比限制在 [min, max] 范围内
 */
int16_t speed_limit(int16_t speed,int16_t max,int16_t min)
{
    if(speed==0)
    {
        return 0;
    }
    else if(speed>max)
    {
        return max;
    }
    else if(speed<min)
    {
        return min;
    }
    return speed;
}

int myAbs(int value)
{
    if(value<0)
    {
        return -value;
    }
    return value;
}

//4.单电机控制
/**
 * @brief   控制左电机（使用CC2和CC3两个通道组成全桥驱动）
 * @param   motor_speed  PWM占空比值 (0 ~ MAX_SPEED)
 * @param   direction    方向：0为正转，1为反转
 * @note    正转：CC3输出PWM，CC2输出低电平。
 *          反转：CC2输出PWM，CC3输出低电平。
 */
void L1_Control(int16_t motor_speed,uint8_t direction)
{
    if(direction==0)//正转
    {
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,motor_speed,GPIO_motor_PWM_C3_IDX);
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,0,GPIO_motor_PWM_C2_IDX);
    }
    else//反转
    {
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,motor_speed,GPIO_motor_PWM_C2_IDX);
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,0,GPIO_motor_PWM_C3_IDX);
    }
}

/**
 * @brief   控制右电机（使用CC0和CC1两个通道组成全桥驱动）
 * @param   motor_speed  PWM占空比值 (0 ~ MAX_SPEED)
 * @param   direction    方向：0为正转，1为反转
 * @note    正转：CC0输出PWM，CC1输出低电平。
 *          反转：CC0输出低电平，CC1输出PWM。 
 */
void R1_Control(int16_t motor_speed,uint8_t direction)
{
    if(direction==0)//正转
    {
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,motor_speed,GPIO_motor_PWM_C0_IDX);
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,0,GPIO_motor_PWM_C1_IDX);
    }
    else//反转
    {
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,motor_speed,GPIO_motor_PWM_C1_IDX);
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,0,GPIO_motor_PWM_C0_IDX);
    }
}
//5.双轮控制，将期望速度转为PWM，控制小车运动
/**
 * @brief   小车底盘控制核心函数，根据左右轮速度设置PWM占空比,并设有速度限制和死区补偿
 * @param   L_motor_speed   左轮期望速度（-MAX_SPEED ~ MAX_SPEED）
 * @param   R_motor_speed   右轮期望速度（-MAX_SPEED ~ MAX_SPEED）
 */
void PWM_Control_Car(int16_t L_motor_speed,int16_t R_motor_speed)
{
    int16_t sLeft,sRight;//左右轮速度
    //1.速度限制
    sLeft=speed_limit(L_motor_speed,MAX_SPEED,-MAX_SPEED);
    sRight=speed_limit(R_motor_speed,MAX_SPEED,-MAX_SPEED);
    //2.死区补偿
    sLeft=Motor_Ignore_Dead_Zone(sLeft);
    sRight=Motor_Ignore_Dead_Zone(sRight);
    //3.左右轮控制
    //左轮：
    if(sLeft>0)//正转
    {
        L1_Control(sLeft,0);
    }
    else//反转
    {
        L1_Control(myAbs(sLeft),1);
    }

    //右轮：
    if(sRight>0)//正转
    {
        R1_Control(sRight,0);
    }
    else//反转
    {
        R1_Control(myAbs(sRight),1);
    }
}
//6.停止
/**
 * @brief   使电机停止
 * @param   brake   1: 刹车（将两路都置为高电平），0: 自由停止（输出0占空比）
 */
void Motor_Stop(uint8_t brake)
{
    if(brake==1)//刹车
    {
        //全部设置为高电平
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,MAX_SPEED,GPIO_motor_PWM_C0_IDX);
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,MAX_SPEED,GPIO_motor_PWM_C1_IDX);
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,MAX_SPEED,GPIO_motor_PWM_C2_IDX);
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,MAX_SPEED,GPIO_motor_PWM_C3_IDX);
    }
    else//自由滑行
    {
        //全部设置为低电平
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,0,GPIO_motor_PWM_C0_IDX);
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,0,GPIO_motor_PWM_C1_IDX);
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,0,GPIO_motor_PWM_C2_IDX);
        DL_TimerA_setCaptureCompareValue(motor_PWM_INST,0,GPIO_motor_PWM_C3_IDX);
    }
}

//7.简单的测试：前进，后退，左转，右转
void Motor_Run(float sLeft,float sRight)
{
    L1_Control(sLeft,0);
    R1_Control(sRight,0);
}

void Motor_Back(float sLeft,float sRight)
{
    L1_Control(sLeft,1);
    R1_Control(sRight,1);
}

void Motor_Left(float sLeft,float sRight)
{
    L1_Control(sLeft,1);
    R1_Control(sRight,0);
    delay_ms(500);
    Motor_Stop(1);
}

void Motor_Right(float sLeft,float sRight)
{
    L1_Control(sLeft,0);
    R1_Control(sRight,1);
    delay_ms(500);
    Motor_Stop(1);
}
