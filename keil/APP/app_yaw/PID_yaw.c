#include "PID_yaw.h"
#include "get_mpu6050.h"
#include "unwrap_yaw.h"


void YawPID_Init(YawPID_t *pid, float kp, float ki, float kd,
                 float integral_limit, float output_limit)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;

    pid->p_out = 0.0f;
    pid->i_out = 0.0f;
    pid->d_out = 0.0f;
    pid->pid_out = 0.0f;
    pid->err = 0.0f;
    pid->prev_error = 0.0f;

    pid->integral = 0.0f;
    
    pid->integral_limit = integral_limit;
    pid->output_limit = output_limit;

    pid->target = 0.0f;
    pid->actual = 0.0f;

    pid->locked = 0;
}

void YawPID_Reset(YawPID_t* pid)
{
    pid->p_out = 0.0f;
    pid->i_out = 0.0f;
    pid->d_out = 0.0f;
    pid->pid_out = 0.0f;
    pid->err = 0.0f;
    pid->prev_error = 0.0f;

    pid->integral = 0.0f;
   
    pid->target = 0.0f;
    pid->actual = 0.0f;

    pid->locked = 0;
}

void YawPID_Lock(YawPID_t* pid)
{
    pid->locked = 1;
}

void YawPID_Unlock(YawPID_t* pid)
{
    pid->locked = 0;
}

float YawPID_GetTarget(YawPID_t* pid)
{
    return pid->target;
}
void YawPID_SetTarget(YawPID_t* pid, float target)
{
    pid->target = target;
}

float YawPID_Compute(YawPID_t*pid,float error)
{
    pid->prev_error = pid->err;
    pid->err = error;
    //积分:
    //积分分离，在误差小的时候才进行积分
    if(fabs(error)<10.0f)
    {
        pid->integral+=error;
    }
    else//大误差时清零积分，防止超调
    {
        pid->integral=0.0f;
    }

    //积分限幅
    if(pid->integral>pid->integral_limit)
    {
        pid->integral=pid->integral_limit;
    }
    if(pid->integral<-pid->integral_limit)
    {
        pid->integral=-pid->integral_limit;
    }

    pid->p_out = pid->Kp * pid->err;
    pid->i_out = pid->Ki * pid->integral;
    pid->d_out = pid->Kd * (pid->err-pid->prev_error);

    // PID 输出
    pid->pid_out = pid->p_out + pid->i_out + pid->d_out;
    // 输出限幅
    if(pid->pid_out>pid->output_limit)
    {
        pid->pid_out=pid->output_limit;
    }
    if(pid->pid_out<-pid->output_limit)
    {
        pid->pid_out=-pid->output_limit;
    }

    return pid->pid_out;
}

