#ifndef	_APP_MPU6050_H_
#define _APP_MPU6050_H_

#include "AllHeader.h"
#include "bsp_mpu6050.h"



extern volatile float pitch,roll,yaw;//全局可访问的欧拉角变量

extern volatile short angle[3];
extern volatile short accel[3];


void Get_EulerAngles(void);

#endif

