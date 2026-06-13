#ifndef __APP_MOTOR_H_
#define __APP_MOTOR_H_

#include "std_types.h"

// 450RPM电机，轮子转一整圈，编码器获得的脉冲数=减速比*码盘线数*编码器脉冲（45*13*4）
#define ENCODER_CIRCLE_450 (1170.0f)

// Half of the sum of the distances between the car chassis motors 小车底盘电机间距之和的一半
#define MSPM0Car_APB (157.09f) //(143.8+170.38)/2

// The displacement of a wheel in one complete revolution, measured in meters 轮子转一整圈的位移，单位为米
#define MECANUM_CIRCLE_MM (210.486f)

// 停止模式，STOP_FREE表示自由停止，STOP_BRAKE表示刹车。
//Stop mode, STOP_ FREE stands for free stop, STOP_ BRAKE stands for braking.
typedef enum _stop_mode
{
    STOP_FREE = 0,
    STOP_BRAKE
} stop_mode_t;
typedef struct _car_data
{
    int16_t Vx;
    int16_t Vy;
    int16_t Vz;
} car_data_t;
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

