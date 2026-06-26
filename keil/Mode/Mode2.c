#include "AllHeader.h"

#define POINT_A      1
#define POINT_B      2   //先到达C点，再到B点
#define POINT_C      3
#define POINT_D      4

/*模式2*/
static uint8_t Black_Flag=0;//是否在黑线上  1：在  0：不在
static uint8_t InitFlag_A=0;//是否初始化A点  1：已初始化  0：未初始化
static uint8_t InitFlag_B=0;//是否初始化B点  1：已初始化  0：未初始化
static uint8_t InitFlag_C=0;//是否初始化C点  1：已初始化  0：未初始化
static uint8_t InitFlag_D=0;//是否初始化D点  1：已初始化  0：未初始化

int yaw_adjust=-10;//yaw调整，用于在地板上的地图刚好

void Mode2_Init(void)
{
	g_speed=G_SPEED;				//设置速度
	Stop_Num=5;					//在第5个点停车，即第二次的A点
	setModeLoopFlag(1);			//循环开启flag
	sampleYaw(&yaw_pid);		//采样锁定目标航向角
	
	//显示相应信息
	OLED_ShowNum_Grid(1,15,getModeLoopFlag(),1,1,0,1);     //显示循环开启flag
	OLED_ShowNum_Grid(1,10,Stop_Num,1,1,0,1);           //显示停车点
	//OLED_ShowString_Grid(2,0,"Mode2_Init",1,0,1);       //显示初始化信息
}

void Mode2_Loop(void)//在直行函数和循迹函数之间切换，通过是否在黑线上来判断
{
	if(getModeLoopFlag())//只有在初始化设置了 循环开启flag 为1时 才执行循环Loop
	{
        Black_Flag=get_is_black();//检测是否在黑线上
		static uint32_t last_track_time = 0;  // 上一次调用循迹函数的时间，用于间隔调用循迹函数
		static uint32_t last_yaw_time = 0;    // 上一次调用直行函数的时间，用于间隔调用直行函数
	
		if (point_count == POINT_A)//A点直行至B点
		{
		    SCHEDULE(last_yaw_time, 20, walkStraight_Yaw(&yaw_pid));
		}
		else if (point_count == POINT_B)//B点循迹至C点
		{
			SCHEDULE(last_track_time, 20, IRTracking(&track_pid));
		}
		else if (point_count == POINT_C)//C点直行至D点
		{
		    if (InitFlag_C == 0)
		    {
		        // 更新target_yaw为循迹半圆后的角度
		        float target_yaw = YawPID_GetTarget(&yaw_pid);
		        YawPID_SetTarget(&yaw_pid, target_yaw -(180+yaw_adjust));//yaw_adjust=-10在地板上的地图刚好
		        InitFlag_C = 1;
		    }
		    SCHEDULE(last_yaw_time, 20, walkStraight_Yaw(&yaw_pid));
		}
		else if (point_count == POINT_D)//D点循迹至A点
		{
		    if (InitFlag_D == 0)
		    {
		        IRTracking_ResetPID(&track_pid);  // 重置循迹PID参数
		        InitFlag_D = 1;
		    }
		    SCHEDULE(last_track_time, 20, IRTracking(&track_pid));  // D点循迹至A点
		}
	}
	
}

void Mode2_Exit(void)
{
	setModeLoopFlag(0);             //关闭循环flag
	Stop_Num=0;                     //清零停车点
    g_speed = 0;   				    //清零目标速度
	InitFlag_A=0;
	InitFlag_B=0;
	InitFlag_C=0;
	InitFlag_D=0;
	Motion_Stop(STOP_BRAKE);		//优先刹车
    walkStraight_Yaw_Reset(&yaw_pid);	//重置直行函数
	YawPID_Unlock(&yaw_pid);//解锁朝向
	Black_Check_Reset();			//重置黑线判断
	IRTracking_ResetPID(&track_pid);//重置循迹PID
	//OLED_ShowString_Grid(3,0,"Mode2_Exit",1,0,1);//显示退出信息
}
