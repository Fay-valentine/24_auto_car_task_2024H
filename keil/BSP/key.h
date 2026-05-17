#ifndef __KEY_H__
#define __KEY_H__

//按键是非阻塞式的，在运行中按下，也可切换模式
//按键1用来切换模式，按一下，模式+1
#include "AllHeader.h"


#define KEY_PRESS      1
#define KEY_RELEASE    0


#define KEY1           1
#define KEY2           2
#define KEY3           3

uint8_t Key1_is_Press(void);
uint8_t Key2_is_Press(void);
uint8_t Key3_is_Press(void);

void Key_Tick(void);
uint8_t Key_GetNum(void);

uint8_t Key_Scan(void);
uint8_t switch_mode(void);

#endif