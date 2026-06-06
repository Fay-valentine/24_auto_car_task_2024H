#include "PID_motor.h"
#include "AllHeader.h"

#define G_SPEED 250     //全局速度
int g_speed=G_SPEED;

//车轮pid声明
Motor_PID motor_pid[MOTOR_NUM];//0:左轮，1：右轮

/**
 * @brief pid重置，重置了输出值，误差值
 * 
 * @param pid 车轮pid结构体
 */
void Motor_PID_Reset(Motor_PID *pid)
{
    pid->target = 0.0f;
    pid->actual = 0.0f;

    pid->enable = 0;

    pid->pid_out=0.0f;

    pid->err=0.0f;
    pid->err_last_1=0.0f;
    pid->err_last_2=0.0f;
}


/**
 * @brief 初始化电机PID控制器
 * @param pid   PID结构体指针
 * @param kp    比例系数
 * @param ki    积分系数
 * @param kd    微分系数
 * @param max_out  输出上限
 * @param min_out  输出下限
 */
void Motor_PID_Init(Motor_PID *pid, float kp, float ki, float kd,
                    float max_out, float min_out)
{
    // 1. 设置PID参数
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;

    // 2. 清零所有历史误差和输出分量
    pid->err=0.0f;
    pid->err_last_1 = 0.0f;
    pid->err_last_2 = 0.0f;
    pid->p_out      = 0.0f;
    pid->i_out      = 0.0f;
    pid->d_out      = 0.0f;
    pid->pid_out    = 0.0f;

    // 3. 清零目标与实际速度
    pid->target = 0.0f;
    pid->actual = 0.0f;

    // 4. 设置输出限幅
    pid->max_out = max_out;
    pid->min_out = min_out;

    // 5. 初始化为使能状态 (也可以默认禁用，让用户手动使能)
    pid->enable = 1;
}



//公式：
//Δu(k) = Kp * [e(k) - e(k-1)] + Ki * e(k) + Kd * [e(k) - 2e(k-1) + e(k-2)]
/**
 * @brief 增量式pid计算函数
 * 
 * @param pid 车轮pid结构体
 * @return float 增量式pid计算结果
 */
float Motor_PID_Cal(Motor_PID *pid)
{
    if(!pid->enable)//未使能
    {
        Motor_PID_Reset(pid);
        return 0.0f;//返回0
    }

    float err=pid->target - pid->actual;//误差

    pid->p_out=pid->Kp * (err - pid->err_last_1);//p分量
    pid->i_out=pid->Ki * err;//i分量
    pid->d_out=pid->Kd * (err - 2.0f*pid->err_last_1 + pid->err_last_2);//d分量

    float delta_u=pid->p_out + pid->i_out + pid->d_out;//总分量

    pid->pid_out+=delta_u;//累加误差

    //输出限幅
    if(pid->pid_out>pid->max_out)
    {
        pid->pid_out=pid->max_out;
    }
    if(pid->pid_out<pid->min_out)
    {
        pid->pid_out=pid->min_out;
    }

    //更新误差
    pid->err_last_2=pid->err_last_1;//先更新err_last2,防止err_last_1先被err覆盖
    pid->err_last_1=err;

    return pid->pid_out;
}


void Motor_Set_TargetSpeed(Motor_PID *pid, float speed)
{
    pid->target = speed;
    pid->enable = 1;
}

void Motor_SetActualSpeed(Motor_PID *pid, float speed)
{
    pid->actual = speed;
}




void Motor_Disable(Motor_PID *pid)
{
    pid->enable = 0;
}

void Motor_Enable(Motor_PID *pid)
{
    pid->enable = 1;
}