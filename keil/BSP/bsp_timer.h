#ifndef __BSP_TIMER_H_
#define __BSP_TIMER_H_

#include "std_types.h"

void Timer_20ms_Init(void);
uint32_t Get_Time(void);
void Timer_1ms_Init(void);

/**
 * @brief 定时调度宏：每隔 interval_ms 毫秒执行一次 action
 * @param last_time  静态/全局的 uint32_t 时间戳变量，宏会更新它
 * @param interval_ms 定时间隔（毫秒）
 * @param action     要执行的语句或语句块
 * 
 * @code
 *     static uint32_t last = 0;
 *     SCHEDULE(last, 15, LineWalking(&track_pid));
 * 
 *     // 也支持多语句块
 *     SCHEDULE(last, 200, 
 *     {
 *         OLED_Update();
 *         Yaw_Update();
 *     });
 * @endcode
 */
#define SCHEDULE(last_time, interval_ms, action) \
    do { \
        if (Get_Time() - (last_time) >= (interval_ms)) \
        { \
            (last_time) = Get_Time(); \
            { action; } \
        } \
    } while(0)

#endif