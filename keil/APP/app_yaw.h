#ifndef _APP_YAW_H_
#define _APP_YAW_H_
#include "PID_yaw.h"

// 前置声明，避免包含整个头文件

extern YawPID_t yaw_pid;

void sampleYaw(YawPID_t* pid);
void walkStraight_Yaw_Init(YawPID_t* pid);
void walkStraight_Yaw_Reset(YawPID_t* pid);
void walkStraight_Yaw(YawPID_t* pid);



#endif