#include "AllHeader.h"

/*模式1*/
void Mode1_Init(void)
{ 
	g_speed=250;				//设置速度
	Stop_Num=2;					//在第二个点停车，即B点
	setModeLoopFlag(1);			//循环开启flag
	sampleYaw(&yaw_pid);		//采样锁定目标航向角
	//显示相应信息
	OLED_ShowNum_Grid(1,15,getModeLoopFlag(),1,1,0,1);//循环标志
	OLED_ShowNum_Grid(1,10,Stop_Num,1,1,0,1);//停车点
	//OLED_ShowString_Grid(2,0,"Mode1_Init",1,0,1);
}

void Mode1_Loop(void)
{
	if(getModeLoopFlag())//只有在初始化设置了 循环开启flag 为1时 才执行循环Loop
	{
		if(g_speed!=0)
		{
			static uint32_t last_yaw_time = 0;
			SCHEDULE(last_yaw_time,20,walkStraight_Yaw(&yaw_pid));//20ms调用一次直行函数
		}
	}
}

void Mode1_Exit(void)
{
	setModeLoopFlag(0);
	Stop_Num=0;//清零停车点
	Motion_Stop(STOP_BRAKE);		//优先刹车
	g_speed = 0;   		//清零目标速度
    walkStraight_Yaw_Reset(&yaw_pid);	//重置直行函数
	YawPID_Unlock(&yaw_pid);//解锁朝向
	Black_Check_Reset();			//重置黑线判断
	//OLED_ShowString_Grid(3,0,"Mode1_Exit",1,0,1);
}
