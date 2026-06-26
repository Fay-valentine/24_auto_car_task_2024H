#ifndef _PID_TRACK_H_
#define _PID_TRACK_H_

#include "std_types.h"
#include "bsp_motor.h"//要使用MAX_SPEED和MOTOR_DEAD_ZONE宏

#define PID_TRACK_KP                (70.0f)//200速：360
#define PID_TRACK_KI                (0.0f)//原1.5，停用积分，只用PD
#define PID_TRACK_KD                (35.0f)//200速：270.0f
#define PID_TRACK_INTEGRAL_LIMIT    (20.0f)  // 积分限幅 (绝对值)，原100
#define PID_TRACK_OUTPUT_LIMIT      (MAX_SPEED)  // 输出限幅, 沿用第一套宏
#define PID_TRACK_INTEGRAL_THRESHOLD (2.0f)  //积分分离阈值，只在误差小于阈值时才进行积分
/**
 * @brief 循迹PID结构体 (位置式PD+I，支持积分限幅)
 */
typedef struct
{
    float Kp, Ki, Kd;        /**< PID三个系数 */
    
    float p_out;             /**< P分量输出 */
    float i_out;             /**< I分量输出 */
    float d_out;             /**< D分量输出 */
    float pid_out;           /**< PID总输出 */
    
    float err;               /**< 当前误差 */
    float prev_error;        /**< 上一次误差 (微分用) */
    float integral;          /**< 积分累加值 */
    
    float integral_limit;    /**< 积分限幅 (绝对值) */
    
    float output_limit;      /**< 输出限幅 (绝对值) */
    float integral_threshold; /**< 积分分离阈值 (阈值) */
    
    float target;            /**< 目标偏差 (通常为0) */
} TrackPID_t;

/**
 * @brief 初始化循迹PID
 * @param pid           PID结构体指针
 * @param kp            比例系数
 * @param ki            积分系数
 * @param kd            微分系数
 * @param integral_limit 积分限幅 (正值)
 * @param output_limit   输出限幅 (正值)
 * @param integral_threshold 积分分离阈值 (阈值)
 * */
void TrackPID_Init(TrackPID_t *pid, float kp, float ki, float kd,
                   float integral_limit, float output_limit, float integral_threshold);

/**
 * @brief 重置循迹PID (清零历史误差和积分)
 * @param pid PID结构体指针
 */
void TrackPID_Reset(TrackPID_t *pid);

/**
 * @brief 设置PID的目标偏差 (通常为0)
 * @param pid    PID结构体指针
 * @param target 目标值
 */
void TrackPID_SetTarget(TrackPID_t *pid, float target);

/**
 * @brief 获取PID当前输出
 * @param pid PID结构体指针
 * @return float 输出值
 */
float TrackPID_GetOutput(TrackPID_t *pid);

/**
 * @brief 计算PID输出 (位置式PID)
 * @param pid      PID结构体指针
 * @param error    当前误差 (黑线重心位置)
 * @return float   PID输出值 (已限幅)
 */
float TrackPID_Compute(TrackPID_t *pid, float error);

#endif /* _PID_TRACK_H_ */