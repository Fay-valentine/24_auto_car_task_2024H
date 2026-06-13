#ifndef _APP_IRTRACKING_H_
#define _APP_IRTRACKING_H_

#include "std_types.h"
#include "PID_track.h"

// 黑白定义 (与第一套硬件一致: 黑线=0, 白线=1)
#define IR_BLACK       0        
#define IR_WHITE       1    

#define TRACK_SPEED       200      // 默认巡线速度

// 全局变量 (供外部访问)
extern int g_IR_track_speed;   // 巡线速度, 可动态修改

// 循迹PID实例
extern TrackPID_t track_pid;

/**
 * @brief 初始化循迹模块 (包括PID参数, 权重等)
 * @param speed  初始巡线速度
 */
void IRTracking_Init(TrackPID_t *pid, int speed);

/**
 * @brief 重置循迹PID (清零历史偏差和积分)
 */
void IRTracking_ResetPID(TrackPID_t *pid);

/**
 * @brief 获取八路红外传感器数据 (兼容原接口)
 */
void deal_IRdata(u8 *x1, u8 *x2, u8 *x3, u8 *x4, u8 *x5, u8 *x6, u8 *x7, u8 *x8);

/**
 * @brief 检测当前是否在黑线上 (兼容原接口)
 * @return IR_BLACK(0) 在黑线上, IR_WHITE(1) 在白线上
 */
uint8_t LineCheck(void);

/**
 * @brief 主循迹函数 (在main循环中周期调用)
 */
void LineWalking(TrackPID_t *pid);  

#endif /* _APP_IRTRACKING_H_ */
