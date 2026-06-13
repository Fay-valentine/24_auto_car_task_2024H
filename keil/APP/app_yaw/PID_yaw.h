#ifndef _PID_YAW_H_
#define _PID_YAW_H_

#include "std_types.h"


#define PID_YAW_KP                      (12.0f) 
#define PID_YAW_KI                      (0.02f) 
#define PID_YAW_KD                      (3.0f) 
#define PID_YAW_INTEGRAL_LIMIT          (200.0f)  
#define PID_YAW_OUTPUT_LIMIT            (1200.0f)


typedef struct YawPID_t 
{
    float Kp, Ki, Kd;          /**< PID三个系数 */
    float p_out, i_out, d_out; /**< P/I/D各分量输出(调试用) */
    float pid_out;             /**< PID总输出 Vz */
    float err;                 /**< 当前误差 */
    float integral;            /**< 积分累积 */
    float prev_error;          /**< 上次误差(微分用) */
    float integral_limit;      /**< 积分限幅 */
    float output_limit;        /**< 输出限幅 */
    float target;              /**< 目标yaw角度 */
    float actual;              /**< 当前yaw角度 */
    uint8_t locked;            /**< 是否已锁定目标角度 */
} YawPID_t;

void YawPID_Init(YawPID_t *pid, float kp, float ki, float kd,
                 float integral_limit, float output_limit);
                 
void YawPID_Reset(YawPID_t* pid);
void YawPID_Lock(YawPID_t* pid);
void YawPID_Unlock(YawPID_t* pid);

float YawPID_Compute(YawPID_t*pid,float error);

float YawPID_GetTarget(YawPID_t* pid);
void YawPID_SetTarget(YawPID_t* pid, float target);

#endif