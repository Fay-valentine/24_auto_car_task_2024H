#ifndef _GLOBAL_H
#define _GLOBAL_H
#include "std_types.h"

//注意模式对应的枚举值
typedef enum
{
    Mode0=0,
    Mode1=1,
    Mode2=2,
    Mode3=3,
    Mode4=4,
}Mode;

void setModeLoopFlag(uint8_t flag);
uint8_t getModeLoopFlag(void);
void Global_LF_refresh(void);
void Global_yaw_refresh(void);
void Global_blackLine_check(void);

void Global_Init(void);
void Global_Loop(void);
void Global_Exit(void);
void Global_Tick(void);

#endif
