#include "get_mpu6050.h"

#define Pi 3.14159265
volatile short angle[3];
volatile short accel[3];
volatile float pitch,roll,yaw;   //欧拉角 Euler Angles

volatile short gyrox,gyroy,gyroz;				//陀螺仪--角速度

volatile short aacx,aacy,aacz;				//陀螺仪--角速度

/**
 * @brief pitch,roll,yaw单位为度
 * 
 */
void Get_EulerAngles(void)
{
	
	static int a;
    //获取欧拉角 Get Euler angles
	mpu_dmp_get_data(&pitch,&roll,&yaw);
	//MPU6050ReadGyro(&gyrox,&gyroy,&gyroz);	//陀螺仪
	//MPU6050ReadAcc(&aacx,&aacy,&aacz);
	
	OLED_ShowSNum_Grid(1,0,yaw,4,1,0,1);
				  
	//printf("gyrox :%d gyroy :%d gyroz:%d\r\n",gyrox,gyroy,gyroz);
	//printf("aacx :%d aacy :%d aacz:%d\r\n",aacx,aacy,aacz);
//	printf("angle[0] :%3.2f angle[1] :%3.2f angle[2]:%3.2f\r\n",angle[0],angle[1],angle[2]);

}


