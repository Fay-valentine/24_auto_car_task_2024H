#include "AllHeader.h"

/*模式1*/
void Mode1_Init(void)
{ 
	g_IR_track_speed=250;		//设置速度
	Mode_Loop_flag=1;			//循环开启flag
	Stop_Num=2;					//在第二个点停车，即B点
	//显示相应信息
	OLED_ShowNum_Grid(1,15,Mode_Loop_flag,1,1,0,1);
	OLED_ShowNum_Grid(1,10,Stop_Num,1,1,0,1);
	//OLED_ShowString_Grid(2,0,"Mode1_Init",1,0,1);
}

void Mode1_Loop(void)
{
	if(Mode_Loop_flag)//只有在初始化设置了 循环开启flag 为1时 才执行循环Loop
	{
		if(g_IR_track_speed!=0)
		{
			StraightLineWalk_IMU();//直行
		}
	}
	
}

void Mode1_Exit(void)
{
	Mode_Loop_flag=0;
	Stop_Num=0;//清零停车点
	Motion_Stop(STOP_BRAKE);		//优先刹车
	g_IR_track_speed = 0;   		//清零目标速度
    StraightLineWalk_IMU_Reset();	//重置直行函数
	Yaw_Unlock();//解锁朝向
	Black_Check_Reset();			//重置黑线判断
	//OLED_ShowString_Grid(3,0,"Mode1_Exit",1,0,1);
}
