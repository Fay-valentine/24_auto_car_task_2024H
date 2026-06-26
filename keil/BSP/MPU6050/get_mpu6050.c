#include "get_mpu6050.h"
#include "AllHeader.h"

#define Pi 3.14159265

volatile static float pitch,roll,yaw;   //欧拉角 Euler Angles

/**
 * @brief pitch,roll,yaw单位为度
 * 
 */
void Get_EulerAngles(void)
{
	
	static int a;
    //获取欧拉角 Get Euler angles
	float p, r, y;
	mpu_dmp_get_data(&p, &r, &y);//读取周期为10ms
	pitch = p; roll = r; yaw = y;
	//MPU6050ReadGyro(&gyrox,&gyroy,&gyroz);	//陀螺仪
	//MPU6050ReadAcc(&aacx,&aacy,&aacz);
	
	OLED_ShowSNum_Grid(1,0,yaw,4,1,0,1);
				  
	//printf("gyrox :%d gyroy :%d gyroz:%d\r\n",gyrox,gyroy,gyroz);
	//printf("aacx :%d aacy :%d aacz:%d\r\n",aacx,aacy,aacz);
//	printf("angle[0] :%3.2f angle[1] :%3.2f angle[2]:%3.2f\r\n",angle[0],angle[1],angle[2]);

}

void Yaw_Update(void)
{
    static float last_valid_yaw = 0.0f;

    float p, r, y;
    if (mpu_dmp_get_data(&p, &r, &y) == 0)
    {
        if (y >= -180.0f && y <= 180.0f)
        {
            pitch = p; roll = r; yaw = y;
            last_valid_yaw = y;
            return;
        }
    }
    yaw = last_valid_yaw;
}

/**
 * @brief 刷新并返回yaw
 * 
 * @return float 
 */
float Get_Yaw(void)
{
    return yaw;
}

void Yaw_tick(void)
{
    static uint8_t count=0;
    count++;
    if(count>=10)
    {
        count=0;
        Yaw_Update();
    }
}