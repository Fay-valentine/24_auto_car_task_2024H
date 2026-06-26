#include "AllHeader.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif


#define POINT_A      1
#define POINT_C      2   //先到达C点，再到B点
#define POINT_B      3
#define POINT_D      4

static uint8_t InitFlag_B=0;//POINT_B是否已初始化
static uint8_t InitFlag_C=0;//POINT_C是否已初始化
static uint8_t InitFlag_D=0;//POINT_D是否已初始化
static uint8_t Black_Flag=0;//是否在黑线上  1：在  0：不在

//A到C点的标志位
static uint8_t A_turn_first=0;//是否完成第一次旋转
static uint8_t A_turn_second=0;//是否完成第二次旋转
static uint8_t A_walk_flag=0;//是否完成第一次行进
static uint8_t A_odomReset_flag=0;//是否重置里程计
//B到D点的标志位
static uint8_t B_turn_first=0;//是否完成第一次旋转
static uint8_t B_turn_second=0;//是否完成第二次旋转
static uint8_t B_walk_flag=0;//是否完成第一次行进
static uint8_t B_odomReset_flag=0;//是否重置里程计

odometry_t od;//里程计数据
float target_od = 100.0f;//目标里程（余弦补偿后的实际目标）
float base_target_od = 100.0f;//用户可调节的基础目标里程
float turn_angle=50.0f;//转弯角度



void Mode3_flagReset(void)
{
	A_turn_first=0;
	A_turn_second=0;
	A_walk_flag=0;
	A_odomReset_flag=0;

	B_turn_first=0;
	B_turn_second=0;
	B_walk_flag=0;
	B_odomReset_flag=0;

	InitFlag_B=0;
	InitFlag_C=0;
	InitFlag_D=0;
	Black_Flag=0;
}

void Mode3_Init(void)
{
	g_speed=G_SPEED;				//设置速度
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

		od=get_odometry_cm();//刷新里程
		
		if (point_count == POINT_A)//开始A点正对B点
		{
			//先重置里程计，以A点为参考起点
			if(A_odomReset_flag==0)
			{
				reset_odometry();
				A_odomReset_flag=1;
			}
			//1.顺时针旋转40°左右
			//2.直行100cm
			//3.逆时针旋转40°左右
			//4.直行至C点
			
			//1.顺时针旋转40°左右
		if(A_turn_first==0 && turnByAngle(&yaw_pid, 1, turn_angle)==1)
		{
			A_turn_first=1;
			Motion_Stop(STOP_BRAKE);//旋转完后刹车
			/* 余弦补偿：根据转弯后的航向误差放大里程目标 */
			float err = Get_Yaw() - YawPID_GetTarget(&yaw_pid);
			Wrap_Process(&err);
			target_od = base_target_od / cosf(fabs(err) * M_PI / 180.0f);
		}
			//2.完成第一次旋转后，直行100cm
			if(A_turn_first)
			{
				if(od.left_cm<target_od && od.right_cm<target_od)//若未到达目标里程
				{
					SCHEDULE(last_yaw_time, 20, walkStraight_Yaw(&yaw_pid));//则继续直行
				}
				else
				{
					A_walk_flag=1;//若已到达目标里程，标记为已行进
					walkStraight_Yaw_RunningReset(&yaw_pid);//重置直行函数运行时状态
				}
			}
			//3.完成第一次旋转和行进后，逆时针转回40°左右
			if(A_turn_first && A_walk_flag)
			{
				if(A_turn_second==0 && turnByAngle(&yaw_pid, -1, turn_angle)==1)
				{
					Motion_Stop(STOP_BRAKE);//旋转完后刹车
					A_turn_second=1;//完成第二次旋转
				}
			}
			//4.完成第二次旋转后，直行至C点
			if(A_turn_first && A_walk_flag && A_turn_second)
			{
				SCHEDULE(last_yaw_time, 20, walkStraight_Yaw(&yaw_pid));
			}
		}
		else if (point_count == POINT_C)
		{
		    if (InitFlag_C == 0)
		    {
		        // 更新target_yaw为循迹半圆后的角度
		        float target_yaw = YawPID_GetTarget(&yaw_pid);
		        YawPID_SetTarget(&yaw_pid, target_yaw +180.0f);  // target_yaw逆时针旋转180°,指向D点，保证是逆时针转弯
		        IRTracking_ResetPID(&track_pid);  // 重置循迹PID参数
		        InitFlag_C = 1;
		    }
		    SCHEDULE(last_track_time, 20, IRTracking(&track_pid));  // 20ms调用一次循迹函数
		}
		else if (point_count == POINT_B)
		{
			//先重置里程计，以B点为参考起点
			if(B_odomReset_flag==0)
			{
				reset_odometry();
				B_odomReset_flag=1;
			}
		    //1.逆时针旋转40°左右
			//2.直行100cm
			//3.顺时针旋转40°左右
			//4.直行至D点
			
			//1.逆时针旋转40°左右
		if(B_turn_first==0 && turnByAngle(&yaw_pid, -1, turn_angle)==1)
		{
			B_turn_first=1;
			Motion_Stop(STOP_BRAKE);//旋转完后刹车
			/* 余弦补偿：根据转弯后的航向误差放大里程目标 */
			float err = Get_Yaw() - YawPID_GetTarget(&yaw_pid);
			Wrap_Process(&err);
			target_od = base_target_od / cosf(fabs(err) * M_PI / 180.0f);
		}
			//2.完成第一次旋转后，直行100cm
			if(B_turn_first)
			{
				if(od.left_cm<target_od && od.right_cm<target_od)//若未到达目标里程
				{
					SCHEDULE(last_yaw_time, 20, walkStraight_Yaw(&yaw_pid));//则继续直行
				}
				else
				{
					B_walk_flag=1;//若已到达目标里程，标记为已行进
					walkStraight_Yaw_RunningReset(&yaw_pid);//重置直行函数运行时状态
				}
			}
			//3.完成第一次旋转和行进后，顺时针转回40°左右
			if(B_turn_first && B_walk_flag)
			{
				if(B_turn_second==0 && turnByAngle(&yaw_pid, 1, turn_angle+yaw_adjust)==1)
				{
					Motion_Stop(STOP_BRAKE);//旋转完后刹车
					B_turn_second=1;//完成第二次旋转
				}
			}
			//4.完成第二次旋转后，直行至D点
			if(B_turn_first && B_walk_flag && B_turn_second)
			{
				SCHEDULE(last_yaw_time, 20, walkStraight_Yaw(&yaw_pid));
			}
		}
		else if (point_count == POINT_D)
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

void Mode3_Tick(void)
{
	
}
void Mode3_Exit(void)
{
    setModeLoopFlag(0);               //关闭循环flag
	Stop_Num=0;                     //清零停车点
    g_speed = 0;   					//清零目标速度
	Mode3_flagReset();//重置标志位
	Motion_Stop(STOP_BRAKE);		//优先刹车
    walkStraight_Yaw_Reset(&yaw_pid);	//重置直行函数
	YawPID_Unlock(&yaw_pid);		//解锁朝向
	Black_Check_Reset();			//重置黑线判断
	IRTracking_ResetPID(&track_pid);//重置循迹PID
	turnByAngle_Reset();            //重置转向状态机
	//OLED_ShowString_Grid(3,0,"Mode3_Exit",1,0,1);//显示退出信息
}
