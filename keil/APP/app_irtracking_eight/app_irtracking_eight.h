#ifndef _APP_IRTRACKING_H_
#define _APP_IRTRACKING_H_


#include "std_types.h"

#define IR_BLACK       0        
#define IR_WHITE       1       


#ifndef u8
#define u8 uint8_t
#endif
#ifndef u16
#define u16 uint16_t
#endif

extern int g_IR_track_speed;

void IR_PID_Reset(void);
void LineWalking(void);
int LineCheck(void);
void deal_IRdata(u8 *x1,u8 *x2,u8 *x3,u8 *x4,u8 *x5,u8 *x6,u8 *x7,u8 *x8);
#endif