#ifndef	_GET_MPU6050_H_
#define _GET_MPU6050_H_

#include "bsp_mpu6050.h"

//extern volatile float pitch,roll,yaw;//全局可访问的欧拉角变量

void Get_EulerAngles(void);
void Yaw_Update(void);
float Get_Yaw(void);
void Yaw_tick(void);

#endif

