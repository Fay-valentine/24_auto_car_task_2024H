#ifndef __TASK_H__
#define __TASK_H__

#include "std_types.h"

#define BLACK_LINE      1
#define WHITE_LINE      0

extern uint8_t point_count;
extern volatile int8_t turn_adjust;
extern volatile int Stop_Num;
extern uint8_t is_black_flag;
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
uint8_t get_is_black(void);

void Schedule_Run(void);
void turnByAngle(int8_t direction, float angle);

#endif