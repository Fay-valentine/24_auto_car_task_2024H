#include "AllHeader.h"

//思路：
//1.A点开局斜对C点出发，到C点侧面入黑线，循迹到B点，把target_yaw顺时针旋转103°
//2.直行至D点，循迹到A点停止

//到中间大概4.2秒左右

#define POINT_A      1
#define POINT_C      2   //先到达C点，再到B点
#define POINT_B      3
#define POINT_D      4

static uint8_t InitFlag_B=0;//POINT_B是否已初始化
static uint8_t InitFlag_C=0;//POINT_C是否已初始化
static uint8_t InitFlag_D=0;//POINT_D是否已初始化
uint8_t Black_Flag=0;//是否在黑线上  1：在  0：不在

void Mode3_Init(void)
{
	g_speed=250;				//设置速度
	Stop_Num=5;					//在第5个点停车，即第二次的A点
	setModeLoopFlag(1);			//循环开启flag
	sampleYaw(&yaw_pid);		//采样一次目标航向角

	//显示相应信息
	OLED_ShowNum_Grid(1,15,getModeLoopFlag(),1,1,0,1);     //显示循环开启flag
	OLED_ShowNum_Grid(1,10,Stop_Num,1,1,0,1);           //显示停车点
	//OLED_ShowString_Grid(2,0,"Mode3_Init",1,0,1);       //显示初始化信息
}

//轨迹：A->C->B->D->A2
void Mode3_Loop(void)
{
	if(getModeLoopFlag())//只有在初始化设置了 循环开启flag 为1时 才执行循环Loop
	{
        Black_Flag=get_is_black();//检测是否在黑线上
		static uint32_t last_track_time = 0;  // 上一次调用循迹函数的时间，用于间隔调用循迹函数
		static uint32_t last_yaw_time = 0;    // 上一次调用直行函数的时间，用于间隔调用直行函数
	
		if (point_count == POINT_A)
		{
		    SCHEDULE(last_yaw_time, 20, walkStraight_Yaw(&yaw_pid));  // A点开局斜对C点出发
		}
		else if (point_count == POINT_C)
		{
		    if (InitFlag_C == 0)
		    {
		        // 更新target_yaw为循迹半圆后的角度
		        float target_yaw = YawPID_GetTarget(&yaw_pid);
		        YawPID_SetTarget(&yaw_pid, target_yaw +257);  // target_yaw逆时针旋转257°,指向D点，保证是逆时针转弯
		        IRTracking_ResetPID(&track_pid);  // 重置循迹PID参数
		        InitFlag_C = 1;
		    }
		    SCHEDULE(last_track_time, 20, LineWalking(&track_pid));  // 20ms调用一次循迹函数
		}
		else if (point_count == POINT_B)
		{
		    SCHEDULE(last_yaw_time, 20, walkStraight_Yaw(&yaw_pid));  // B点直行至D点
		}
		else if (point_count == POINT_D)
		{
		    if (InitFlag_D == 0)
		    {
		        IRTracking_ResetPID(&track_pid);  // 重置循迹PID参数
		        InitFlag_D = 1;
		    }
		    SCHEDULE(last_track_time, 20, LineWalking(&track_pid));  // D点循迹至A点
		}
	}
}

void Mode3_Tick(void)
{
	
}
void Mode3_Exit(void)
{
    setModeLoopFlag(0);               //关闭循环flag
	Stop_Num=0;                     //清零停车点
    g_speed = 0;   					//清零目标速度
	InitFlag_C=0;                   //重置C点初始化标志
	InitFlag_D=0;                   //重置D点初始化标志
	Motion_Stop(STOP_BRAKE);		//优先刹车
    walkStraight_Yaw_Reset(&yaw_pid);	//重置直行函数
	YawPID_Unlock(&yaw_pid);		//解锁朝向
	Black_Check_Reset();			//重置黑线判断
	IRTracking_ResetPID(&track_pid);//重置循迹PID
	//OLED_ShowString_Grid(3,0,"Mode3_Exit",1,0,1);//显示退出信息
}
