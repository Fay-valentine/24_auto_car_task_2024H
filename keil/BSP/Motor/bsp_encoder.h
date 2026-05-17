#ifndef __BSP_ENCODER_H__
#define __BSP_ENCODER_H__

#include "AllHeader.h"

extern int Encoder_L;
extern int Encoder_R;
//方向枚举
typedef enum
{
    Forward,
    Reversal
}Encoder_Dir;

//车轮状态结构体
typedef struct
{
    volatile int temp_count;//用于中断实时改变
    int count;//定时更新为temp_count的值
    int AllCount;//总计数值
    Encoder_Dir dir;//车轮方向
}Encoder_Res;

void Encoder_Init(void);
Encoder_Dir get_encoderL_dir(void);
Encoder_Dir get_encoderR_dir(void);
void Encoder_Get_ALL(int *Encoder_all);
void Encoder_Get_Temp(int *Encoder_temp);
void Encoder_Update(void);

#endif