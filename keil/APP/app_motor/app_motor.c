#include "app_motor.h"
#include "AllHeader.h"

static float speed_lr = 0;
static float speed_fb = 0;
static float speed_spin = 0;//旋转速度

static int speed_L1_setup = 0;
static int speed_R1_setup = 0;


static int g_offset_yaw = 0;
static uint16_t g_speed_setup = 0;

// 编码器10毫秒前后数据
//Encoder data before and after 10 milliseconds
int g_Encoder_All_Now[MAX_MOTOR] = {0};
int g_Encoder_All_Last[MAX_MOTOR] = {0};
int g_Encoder_All_Offset[MAX_MOTOR] = {0};

uint8_t g_start_ctrl = 0;

car_data_t car_data;

static float Motion_Get_Circle_Pulse(void)
{
    return ENCODER_CIRCLE_450;
}

// 返回当前小车轮子轴间距和的一半
//Returns half of the sum of the current wheel spacing of the small car
float Motion_Get_APB(void)
{
    return MSPM0Car_APB;
}

// 返回当前小车轮子转一圈的多少毫米
//Returns the number of millimeters the current car wheel has rotated once
float Motion_Get_Circle_MM(void)
{
    return MECANUM_CIRCLE_MM;
}

// Car Stop 小车停止
/**
 * @brief 刹车函数
 * 
 * @param brake 1: 刹车（将两路都置为高电平），0: 自由停止（输出0占空比）
 */
void Motion_Stop(uint8_t brake)
{
    Motion_Set_Speed(0, 0);
    g_start_ctrl = 0;
    Motor_PID_Reset(&motor_pid[0]);
    Motor_PID_Reset(&motor_pid[1]);
    Motor_Stop(brake);
}

// speed_mX=[-1000, 1000], 单位为：mm/s
void Motion_Set_Speed(int16_t speed_m1, int16_t speed_m2)
{
    g_start_ctrl = 1;
    Motor_Set_TargetSpeed(&motor_pid[0],speed_m1); 
    Motor_Set_TargetSpeed(&motor_pid[1],speed_m2);
}

// 从编码器读取当前各轮子速度，单位mm/s
//Read the current speed of each wheel from the encoder, in mm/s
void Motion_Get_Speed(void)
{
    int i = 0;
    float speed_mm[MAX_MOTOR] = {0};
    float circle_mm = Motion_Get_Circle_MM();
    float circle_pulse = Motion_Get_Circle_Pulse();
    float robot_APB = Motion_Get_APB();

    Motion_Get_Encoder();

    // 计算轮子速度，单位mm/s。
    //Calculate the wheel speed in mm/s.
    for (i = 0; i < MAX_MOTOR; i++)
    {
        speed_mm[i] = (g_Encoder_All_Offset[i]) * 50 * circle_mm / circle_pulse;
    }

    // 更新电机控制器的实际速度
    Motor_SetActualSpeed(&motor_pid[0], speed_mm[0]);
    Motor_SetActualSpeed(&motor_pid[1], speed_mm[1]);
}

// 获取编码器数据，并计算偏差脉冲数
//Obtain encoder data and calculate the number of deviation pulses
void Motion_Get_Encoder(void)
{
    Encoder_Get_ALL(g_Encoder_All_Now);

    for (uint8_t i = 0; i < MAX_MOTOR; i++)
    {
        // 记录两次测试时间差的脉冲数
    	//Record the number of pulses between two test times
        g_Encoder_All_Offset[i] = g_Encoder_All_Now[i] - g_Encoder_All_Last[i];
        // 记录上次编码器数据
        //Record Last Encoder Data
        g_Encoder_All_Last[i] = g_Encoder_All_Now[i];
    }
}

void Motors_Init(void)
{
    Motor_PID_Init(&motor_pid[0], PID_MOTOR_KP, PID_MOTOR_KI, PID_MOTOR_KD, (float)(MAX_SPEED - MOTOR_DEAD_ZONE), -(float)(MAX_SPEED - MOTOR_DEAD_ZONE));
    Motor_PID_Init(&motor_pid[1], PID_MOTOR_KP, PID_MOTOR_KI, PID_MOTOR_KD, (float)(MAX_SPEED - MOTOR_DEAD_ZONE), -(float)(MAX_SPEED - MOTOR_DEAD_ZONE));
}

// 运动控制句柄，每20ms调用一次，主要处理速度相关的数据
//Motion control handle, called every 10ms, mainly processing speed related data
void Motion_Handle(void)
{
    Motion_Get_Speed();//更新pid->actual

    if (g_start_ctrl)
    {
        // 计算两侧电机 PWM
        int16_t pwm_L = (int16_t)Motor_PID_Cal(&motor_pid[0]);
        int16_t pwm_R = (int16_t)Motor_PID_Cal(&motor_pid[1]);
        PWM_Control_Car(pwm_L, pwm_R);//通过PWM控制电机速度
    }
    else
    {
        PWM_Control_Car(0, 0);
    }
}

/**
 * @brief 上层设置速度的函数，单位是mm/s
 * 
 * @param V_x 前进线速度（正值前进，负值后退）
 * @param V_y 横向移动速度
 * @param V_z 转向角速度（等效线速度差，不是度/秒;传入负数，左转，正数，右转）
 * @note 根据期望的前进速度和转向速度，计算出左右轮各自的目标线速度，并将它们下发给电机速度环去执行
 */
void Motion_Car_Control(int16_t V_x, int16_t V_y, int16_t V_z)
{
	float robot_APB = Motion_Get_APB();
	speed_lr = 0;
    speed_fb = V_x;
    speed_spin = (V_z / 1000.0f) * robot_APB;
    if (V_x == 0 && V_y == 0 && V_z == 0)
    {
        Motion_Set_Speed(0,0);
        return;
    }
    //通过加减旋转速度实现转向速度差，最终得到两轮各自的目标速度
    speed_L1_setup = speed_fb + speed_spin;

    speed_R1_setup = speed_fb  - speed_spin;

	//速度限幅	
    if (speed_L1_setup > 1000) speed_L1_setup = 1000;
    if (speed_L1_setup < -1000) speed_L1_setup = -1000;

    if (speed_R1_setup > 1000) speed_R1_setup = 1000;
    if (speed_R1_setup < -1000) speed_R1_setup = -1000;

    Motion_Set_Speed(speed_L1_setup,speed_R1_setup);
}