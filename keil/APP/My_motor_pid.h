#ifndef _MY_MOTOR_PID_H_
#define _MY_MOTOR_PID_H_
#include "std_types.h"

#define MOTOR_NUM   2

#define PID_MOTOR_KP (0.8f) //0.8
#define PID_MOTOR_KI (0.06f) //0.06
#define PID_MOTOR_KD (0.5f) //0.5

#define G_SPEED 250     //全局速度

//增量式pid结构体
typedef struct Motor_PID
{
    //pid本体：
    float Kp,Ki,Kd;//三个系数
    float p_out,i_out,d_out;//各自的数值
    float pid_out;//总pid输出值,实际上直接对应pwm值（限幅在850以下）
    float err,err_last_1,err_last_2;//三个时刻的误差
    
    //车轮相关成员：
    float target;//目标速度 mm/s
    float actual;//实际速度 mm/s
    uint8_t enable; //车轮使能标志 0：未使能  1：使能

    //输出限幅
    float max_out;
    float min_out;

}Motor_PID;

extern Motor_PID motor_pid[MOTOR_NUM];

void Motor_PID_Init(Motor_PID *pid, float kp, float ki, float kd,
                    float max_out, float min_out);
float Motor_PID_Cal(Motor_PID *pid);
void Motor_SetActualSpeed(Motor_PID *pid, float speed);
void Motor_Set_TargetSpeed(Motor_PID *pid, float speed);
void Motor_PID_Reset(Motor_PID *pid);


#endif