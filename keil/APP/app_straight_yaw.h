#ifndef _APP_STRAIGHT_YAW_H
#define _APP_STRAIGHT_YAW_H

#include "AllHeader.h"

extern float Vz_Bias;
extern volatile float target_yaw;
// 角度环 PID 参数
typedef struct 
{
    float Kp, Ki, Kd;
    float integral;
    float prev_error;
    float integral_limit;   // 积分限幅
    float output_limit;     // 输出限幅 (±)
} YawPID_t;

// 与 yaw 传感器相关的函数
void set_target_yaw(float yaw);
float get_target_yaw(void);

float get_filtered_yaw(void);

float YawPID_Compute(YawPID_t*pid,float error);
void YawPID_Init(YawPID_t *pid, float kp, float ki, float kd,
                 float integral_limit, float output_limit);
              
void Yaw_Unlock(void);                 
void Yaw_Lock(void);                 
void StraightLineWalk_IMU_Reset(void);                 
void StraightLineWalk_IMU_Init(void);
void StraightLineWalk_IMU(void);

#endif