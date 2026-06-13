#include "PID_track.h"

void TrackPID_Init(TrackPID_t *pid, float kp, float ki, float kd,
                   float integral_limit, float output_limit)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->integral_limit = integral_limit;
    pid->output_limit = output_limit;
    
    TrackPID_Reset(pid);
    pid->target = 0.0f;
}

void TrackPID_Reset(TrackPID_t *pid)
{
    pid->p_out = 0.0f;
    pid->i_out = 0.0f;
    pid->d_out = 0.0f;
    pid->pid_out = 0.0f;
    pid->err = 0.0f;
    pid->prev_error = 0.0f;
    pid->integral = 0.0f;
}

void TrackPID_SetTarget(TrackPID_t *pid, float target)
{
    pid->target = target;
}

float TrackPID_GetOutput(TrackPID_t *pid)
{
    return pid->pid_out;
}

float TrackPID_Compute(TrackPID_t *pid, float error)
{
    // 计算误差 (target - actual), 通常 target = 0                                                                                                                                                                                                                                                                                                                                                                      
    pid->err = error;
    
    // 比例项
    pid->p_out = pid->Kp * pid->err;
    
    // 积分项 (带积分限幅)
    pid->integral += pid->err;
    if (pid->integral > pid->integral_limit)
    {
        pid->integral = pid->integral_limit;
    }
    if (pid->integral < -pid->integral_limit)
    {
        pid->integral = -pid->integral_limit;
    }
    pid->i_out = pid->Ki * pid->integral;
    
    // 微分项
    pid->d_out = pid->Kd * (pid->err - pid->prev_error);
    
    // PID 总输出
    pid->pid_out = pid->p_out + pid->i_out + pid->d_out;
    
    // 输出限幅
    if (pid->pid_out > pid->output_limit)
    {
        pid->pid_out = pid->output_limit;
    }
    if (pid->pid_out < -pid->output_limit)
    {
        pid->pid_out = -pid->output_limit;
    }
    
    // 保存本次误差供下次微分使用
    pid->prev_error = pid->err;
    
    return pid->pid_out;
}