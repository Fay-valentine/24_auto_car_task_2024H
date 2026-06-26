#ifndef _MODE3_H_
#define _MODE3_H_

#include "app_motor.h"

extern float target_od;//目标里程（余弦补偿后的实际目标）
extern float base_target_od;//用户可调节的基础目标里程
extern odometry_t od;
extern float turn_angle;//转弯角度

//在室内场地，base_target_od=105cm到110cm为宜
//yaw_adjust=-8°为宜

void Mode3_Init(void);
void Mode3_Loop(void);
void Mode3_Tick(void);
void Mode3_Exit(void);

#endif
