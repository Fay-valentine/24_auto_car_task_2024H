#ifndef __APP_MOTOR_H_
#define __APP_MOTOR_H_

#include "std_types.h"

// 450RPM电机，轮子转一整圈，编码器获得的脉冲数=减速比*码盘线数*编码器脉冲（45*13*2）bsp_encoder实现的其实是2倍频而不是4倍频
#define ENCODER_CIRCLE_450 (1170.0f)

//小车底盘电机间距之和的一半
#define MSPM0Car_APB (157.09f) //(143.8+170.38)/2

//轮子转一整圈的位移，单位为毫米
#define MECANUM_CIRCLE_MM (210.486f)

// 停止模式，STOP_FREE表示自由停止，STOP_BRAKE表示刹车
typedef enum _stop_mode
{
    STOP_FREE = 0,
    STOP_BRAKE
} stop_mode_t;

// 里程计数据结构：存储左右轮相对于起点的累计行驶距离（单位：毫米）
// 正数代表正向移动，负数代表反向移动
typedef struct 
{
    float left_mm;   // 左轮累计行驶距离(单位：毫米)
    float right_mm;  // 右轮累计行驶距离(单位：毫米)
    float left_cm;  // 左轮累计行驶距离（单位：厘米）
    float right_cm;  // 右轮累计行驶距离（单位：厘米）
} odometry_t;

// 获取当前左右轮累计行驶距离（从上次重置或开机算起）
odometry_t get_odometry_mm(void);
odometry_t get_odometry_cm(void);

// 重置里程计起点（将当前位置设为零点，重新开始累积距离）
void reset_odometry(void);


void Motors_Init(void);
void Motion_Stop(uint8_t brake);//刹车
void Motion_Handle(void);//task定时调用
void Motion_Car_Control(int16_t V_x, int16_t V_y, int16_t V_z);

void Motion_Set_Speed(int16_t speed_m1, int16_t speed_m2);//mm/s
void Motion_Get_Speed(void);
float Motion_Get_APB(void);
float Motion_Get_Circle_MM(void);
void Motion_Get_Encoder(void);

#endif

