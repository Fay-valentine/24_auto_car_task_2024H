#include "app_yaw.h"
#include "AllHeader.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

float Vz_Bias = 0.0f;  //转向偏差值，来克服左右轮差距

YawPID_t yaw_pid;
YawFilter_t yaw_filter;

/**
 * @brief  计算一组角度的环形平均值（矢量平均）
 * @param  angles  角度数组，单位度，范围 [-180, 180]
 * @param  count   数组长度
 * @return 平均角度，范围 [-180, 180)
 */
float average_yaw_deg(const float angles[], int count)
{
    // 处理空数组
    if (count <= 0) return 0.0f;

    float sum_sin = 0.0f;  // 所有角度的正弦和
    float sum_cos = 0.0f;  // 所有角度的余弦和

    for (int i = 0; i < count; i++) 
    {
        // 将角度转换为弧度（C标准库三角函数使用弧度）
        float rad = angles[i] * M_PI / 180.0f;
        // 累加单位向量的分量
        sum_sin += sinf(rad);
        sum_cos += cosf(rad);
    }

    // 检查合向量是否几乎为零（例如角度均匀分布在圆周上）
    // 此时无法定义唯一平均方向，返回默认值 0°
    if (fabsf(sum_sin) < 1e-6f && fabsf(sum_cos) < 1e-6f) 
    {
        return 0.0f;
    }

    // 计算合向量的辐角（弧度），即环形平均角度
    float avg_rad = atan2f(sum_sin, sum_cos);
    // 弧度转换为度
    float avg_deg = avg_rad * 180.0f / M_PI;

    // 将结果归一化到 [-180, 180) 区间，与输入角度范围一致
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
        
        Yaw_Update();//确保yaw被正确更新
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
    yaw_filter_Init(&yaw_filter,LPF_ALPHA);
}
/**
 * @brief 重置pid，清空电机的积分和输出
 * @param pid 
 */
void walkStraight_Yaw_Reset(YawPID_t* pid) 
{
    YawPID_Reset(pid);
    Motor_PID_Reset(&motor_pid[0]); 
    Motor_PID_Reset(&motor_pid[1]); // 清零两个电机的积分和输出
}

void walkStraight_Yaw(YawPID_t* pid)
{
    if (pid->locked == 0) 
    {
        return;  // ← 未锁定时不执行
    }

    float current_yaw=yaw_filter_Process(&yaw_filter,Get_Yaw());//一阶低通滤波

    pid->actual = current_yaw;//更新实际航向

    //计算误差
    float yaw_error=pid->target-current_yaw;

    //环绕修正
    Wrap_Process(&yaw_error);

    //使用角度环PID
    float Vz=YawPID_Compute(pid,yaw_error);
    
    float Vz_total = Vz + Vz_Bias;
    //往左偏时，error<0,需要Vz>0,往右调整;
    //往右偏时，error>0,需要Vz<0，往左调整
    //所以这里加了负号
    Motion_Car_Control(g_speed, 0, -(int16_t)Vz_total);
}