#include "app_straight_yaw.h"

static YawPID_t yaw_pid;
volatile float target_yaw = 0.0f;         // 锁定的目标航向
static uint8_t yaw_locked = 0;          // 是否已锁定朝向

/**
 * @brief 通过改变target_yaw来改变目标航向角
 * 
 * @param yaw 目标航向角，单位：度
 */
void set_target_yaw(float yaw)
{
    target_yaw=yaw;
}

float get_target_yaw(void)
{
    return target_yaw;
}

void YawPID_Init(YawPID_t *pid, float kp, float ki, float kd,
                 float integral_limit, float output_limit)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->integral_limit = integral_limit;
    pid->output_limit = output_limit;
}

/**
 * @brief 获取一个经过滤波，漂移补偿后的yaw，消除MPU6050本身的缺陷
 * 
 * @return yaw_compensated:可以把它当作一个 无漂移的航向反馈值，用来与目标航向角作比较 
 */
float get_filtered_yaw(void)
{
    //一阶低通滤波
    static float lpf = 0.0f;
    static uint8_t init = 1;
    if (init) { lpf = yaw; init = 0; }
    lpf = 0.8f * lpf + 0.2f * yaw;   // 低通滤波（系数可调）
    float filtered = lpf;
    //环绕处理
    if (filtered > 180.0f)  filtered -= 360.0f;
    if (filtered < -180.0f) filtered += 360.0f;
    return filtered;
}

/**
 * @brief  角度环 PID 计算
 * @param  pid    PID 对象
 * @param  error  目标 - 当前（已处理过角度环绕）
 * @return        转向角速度 Vz
 */
float YawPID_Compute(YawPID_t*pid,float error)
{
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

    //微分
    float derivative=error-pid->prev_error;

    // PID 输出
    float outpid=pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;

    // 输出限幅
    if (outpid > pid->output_limit) 
    { 
        outpid = pid->output_limit;
    }
    if (outpid < -pid->output_limit) 
    {
        outpid = -pid->output_limit;
    }

    pid->prev_error = error;
    return outpid;
}

/**
 * @brief 用于重置状态
 * 
 */
void StraightLineWalk_IMU_Reset(void) 
{
    //yaw_locked = 0;   // 清空锁定标志
    // PID 积分也可以清零（可选）
    //Yaw_lock();不解锁采样
    yaw_pid.integral = 0;
    PID_Clear_Motor(MAX_MOTOR);   // 清零两个电机的积分和输出
}

void Yaw_Unlock(void)
{
    yaw_locked = 0;   // 清空锁定标志
}

void StraightLineWalk_IMU_Init(void)
{
    // 初始化 PID 参数
    YawPID_Init(&yaw_pid,
                6.0f,     // Kp
                0.03f,     // Ki
                2.0f,      // Kd
                40.0f,    // 积分限幅
                300.0f);   // Vz 输出限幅 (对应你的转向能力)
    
    // 重置锁定标志
    yaw_locked = 0;
}

/**
 * @brief 陀螺仪走直线，包括启动前的100ms采样
 * 
 */
float Vz_Bias = 10.0f;  //转向偏差值，来克服左右轮差距
void StraightLineWalk_IMU(void)
{
    mpu_dmp_get_data(&pitch, &roll, &yaw);
    float current_yaw=get_filtered_yaw();//获取滤波，补偿后的航向

    //采样
    if (yaw_locked == 0)
    {
        // 使用原始 yaw 多次采样平均，完全避开滤波器滞后
        float sum_raw = 0.0f;
        for (int i = 0; i < 40; i++)   // 取 40 次，共400ms
        {
            mpu_dmp_get_data(&pitch, &roll, &yaw);   // 直接更新原始 yaw
            sum_raw += yaw;
            delay_ms(10);
        }
        target_yaw = sum_raw / 40.0f;
        yaw_locked = 1;
        //OLED_ShowSNum_Grid(3,11,target_yaw,4,1,0,1);//更新显示一次target_yaw
        return;//进行下一次周期，防止current_yaw未更新
    }

    //计算误差
    float error=target_yaw-current_yaw;
    //环绕修正
    if (error > 180.0f)  error -= 360.0f;
    if (error < -180.0f) error += 360.0f;

    //使用角度环PID
    float Vz=YawPID_Compute(&yaw_pid,error);

    // 前进速度（复用原来的巡线速度宏）
    int16_t Vx = g_IR_track_speed;
    
    float Vz_total = Vz + Vz_Bias;
    Motion_Car_Control(Vx, 0, -(int16_t)Vz_total);

}

