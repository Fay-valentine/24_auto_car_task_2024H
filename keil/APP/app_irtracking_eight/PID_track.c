#include "PID_track.h"

void TrackPID_Init(TrackPID_t *pid, float kp, float ki, float kd,
                   float integral_limit, float output_limit, float integral_threshold)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->integral_limit = integral_limit;
    pid->output_limit = output_limit;
    pid->integral_threshold = integral_threshold;
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

    if(fabs(pid->err) >=3.5f)//极大偏差：4,3压线（最边缘的两个点）
    {
        pid->Kp=90.0f;
    }
    else if(fabs(pid->err)<=2.0f)//最小偏差：中间四个点的两个压线
    {
        pid->Kp=PID_TRACK_KP;
    }
    else // 中等偏差 (误差在 2.0 到 3.5 之间)
    {
    // 1. 计算当前误差离 2.0 有多远
    float diff = fabs(pid->err) - 2.0f;
    
    // 2. 计算误差每变化 1，Kp 需要变化多少 (斜率)
    float slope = (90.0f - PID_TRACK_KP) / (3.5f - 2.0f); // 结果约等于 23.3
    
    // 3. 计算最终的 Kp
    pid->Kp = PID_TRACK_KP + diff * slope;
    }
    
    // 比例项
    pid->p_out = pid->Kp * pid->err;
    
    //积分分离, 当误差小于阈值时，才进行积分
    if(fabs(pid->err) < pid->integral_threshold)
    {
        pid->integral += pid->err;
    }
    
    // 积分项 (带积分限幅)
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