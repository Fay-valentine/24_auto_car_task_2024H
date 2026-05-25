#include "app_yaw.h"
#include "AllHeader.h"

float Vz_Bias = 0.0f;  //转向偏差值，来克服左右轮差距

YawPID_t yaw_pid;


#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/**
 * @brief  计算一组角度的环形平均值（矢量平均）
 * @param  angles  角度数组，单位度，范围 [-180, 180]
 * @param  count   数组长度
 * @return 平均角度，范围 [-180, 180)
 */
float average_yaw_deg(const float angles[], int count)
{
    if (count <= 0) return 0.0f;

    float sum_sin = 0.0f, sum_cos = 0.0f;
    for (int i = 0; i < count; i++) 
    {
        float rad = angles[i] * M_PI / 180.0f;
        sum_sin += sinf(rad);
        sum_cos += cosf(rad);
    }

    // 防止合向量为零（如角度均匀环绕一周）
    if (fabsf(sum_sin) < 1e-6f && fabsf(sum_cos) < 1e-6f) 
    {
        return 0.0f;
    }

    float avg_rad = atan2f(sum_sin, sum_cos);
    float avg_deg = avg_rad * 180.0f / M_PI;

    // 归一化到 [-180, 180)
    if (avg_deg >= 180.0f)      avg_deg -= 360.0f;
    else if (avg_deg < -180.0f) avg_deg += 360.0f;

    return avg_deg;
}

void sampleYaw(YawPID_t* pid)
{
    //采样
    if (pid->locked == 0)
    {
        float yaw_samples[40];//采样数组
        int valid_count = 0;//有效采样次数
        
        for (int i = 0; i < 40; i++)   // 取 40 次，共400ms
        {
            yaw_samples[valid_count++] = Get_Yaw();
            delay_ms(10);
        }
        pid->target = average_yaw_deg(yaw_samples, valid_count);//计算平均值

        pid->locked = 1;//锁定采样

        OLED_ShowSNum_Grid(3,6,pid->target,4,1,0,1);//更新显示一次target_yaw
        
        return;//进行下一次周期，防止current_yaw未更新
    }
}


void walkStraight_Yaw_Init(YawPID_t* pid)
{
    YawPID_Init(pid,PID_YAW_KP,PID_YAW_KI,PID_YAW_KD,
                PID_YAW_INTEGRAL_LIMIT,PID_YAW_OUTPUT_LIMIT);
}
/**
 * @brief 重置pid，清空电机的积分和输出
 * @param pid 
 */
void walkStraight_Yaw_Reset(YawPID_t* pid) 
{
    YawPID_Reset(pid);
    PID_Clear_Motor(MAX_MOTOR);   // 清零两个电机的积分和输出
}

void walkStraight_Yaw(YawPID_t* pid)
{
    if (pid->locked == 0) 
    {
        return;  // ← 未锁定时不执行
    }

    float current_yaw=Get_lpf1st_yaw();//一阶低通滤波

    pid->actual = current_yaw;//更新实际航向

    //计算误差
    float yaw_error=pid->target-current_yaw;

    //环绕修正
    Wrap_Process(&yaw_error);

    //使用角度环PID
    float Vz=YawPID_Compute(pid,yaw_error);
    
    float Vz_total = Vz + Vz_Bias;
    Motion_Car_Control(g_speed, 0, -(int16_t)Vz_total);
 
}