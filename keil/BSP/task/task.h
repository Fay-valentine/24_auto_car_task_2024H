#ifndef __TASK_H__
#define __TASK_H__

#include "std_types.h"

struct YawPID_t;

#define BLACK_LINE      0
#define WHITE_LINE      1

extern uint8_t point_count;
extern volatile int8_t turn_adjust;
extern volatile int Stop_Num;
extern uint8_t cur_blackLine_flag;
typedef void(*TaskFunc)(uint8_t arg);// 定义统一的函数指针类型

//任务结构体
typedef struct
{
    uint32_t interval;//期望的执行时间间隔,单位为ms
    uint32_t last_call;//上次调用的时间
    //void (*task)(void);//名为 task 函数指针，可以调用相同返回值，参数的函数
    TaskFunc task_func;
    int arg;
}Task;

void Black_Check(uint8_t stop_num);
void Black_Check_Reset(void);
uint8_t try_point_change(uint8_t seconds);
uint8_t get_is_black(void);

void Schedule_Run(void);
uint8_t turnByAngle(struct YawPID_t *pid,int8_t direction, float angle);
uint8_t turnByAngle_IsBusy(void);
void turnByAngle_Reset(void);
void TurnByAngle_Test(void);
void line_yaw_walk_test(void);
void odometry_test(void);
void IRTracking_test(void);

#endif