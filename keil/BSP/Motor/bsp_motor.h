#ifndef __BSP_MOTOR_H__
#define __BSP_MOTOR_H__
#include "AllHeader.h"
#define MOTOR_DEAD_ZONE (75)//死区补偿值
#define MAX_SPEED   (850)//占空比为1000，设置占空比上限

typedef enum
{
	MOTOR_ID_M1 = 0, //值为0
	MOTOR_ID_M2,     //值为1
	MAX_MOTOR		 //值为2，C语言会隐式递增	
} Motor_ID;

void Init_Motor_PWM();
int16_t speed_limit(int16_t speed,int16_t max,int16_t min);
int myAbs(int value);
void L1_Control(int16_t motor_speed,uint8_t direction);
void R1_Control(int16_t motor_speed,uint8_t direction);
void PWM_Control_Car(int16_t L_motor_speed,int16_t R_motor_speed);

void Motor_Stop(uint8_t brake);
void Motor_Run(float sLeft,float sRight);
void Motor_Back(float sLeft,float sRight);
void Motor_Left(float sLeft,float sRight);
void Motor_Right(float sLeft,float sRight);

#endif