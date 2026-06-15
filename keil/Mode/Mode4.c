#include "AllHeader.h"

/*模式4*/

#define POINT_A      1
#define POINT_C      2   //先到达C点，再到B点
#define POINT_B      3
#define POINT_D      4
#define POINT_FINAL  5   

static uint8_t turn_flag_A=0;//是否右转39°  1：已转  0：未转
static uint8_t turn_flag_C=0;//是否左转39°  1：已转  0：未转
static uint8_t turn_flag_B=0;//是否左转39°  1：已转  0：未转
static uint8_t turn_flag_D=0;//是否右转39°  1：已转  0：未转

static uint8_t Black_Flag=0;//是否在黑线上  1：在  0：不在
static uint8_t circle_count=0;//当前圈数

/**
 * @brief 每完成一圈后的状态重置
 */
void state_reset(void)
{
    point_count=POINT_A;
    turn_flag_A=0;
    turn_flag_C=0;
    turn_flag_B=0;
    turn_flag_D=0;
}

void Mode4_Init(void)
{
	setModeLoopFlag(1);			//循环开启flag
	g_speed=250;				//设置速度
	Stop_Num=13;				//在第13个点停车，即第二次的A点
	sampleYaw(&yaw_pid);		//采样一次目标航向角
	//显示相应信息
	OLED_ShowNum_Grid(1,15,getModeLoopFlag(),1,1,0,1);     //显示循环开启flag
	OLED_ShowNum_Grid(1,10,Stop_Num,1,1,0,1);           //显示停车点
	//OLED_ShowString_Grid(2,0,"Mode4_Init",1,0,1);       //显示初始化信息
	
}

//轨迹：A->C->B->D->A2
void Mode4_Loop(void)
{
	if(getModeLoopFlag())//只有在初始化设置了 循环开启flag 为1时 才执行循环Loop
	{
        Black_Flag=get_is_black();//检测是否在黑线上

		if(g_speed!=0)
		{

			switch (point_count)
    		{
    		case POINT_A://右转39°
				if(turn_flag_A==0)
				{
					if(turnByAngle(&yaw_pid,1,39))
					{
						turn_flag_A=1;
					}
				}
    		    break;
			
			case POINT_C://左转39°
				if(turn_flag_C==0)
				{
					if(turnByAngle(&yaw_pid,-1,39))
					{
						//更新target_yaw为循迹半圆后的角度
						float target_yaw=YawPID_GetTarget(&yaw_pid);
						YawPID_SetTarget(&yaw_pid,target_yaw-180);
						turn_flag_C=1;
					}
				}
				break;

			case POINT_B://左转39°
				if(turn_flag_B==0)
				{
					if(turnByAngle(&yaw_pid,-1,39))
					{
						turn_flag_B=1;
					}
				}
				break;
				
			case POINT_D://右转39°
				if(turn_flag_D==0)
				{
					if(turnByAngle(&yaw_pid,1,39))
					{
						//更新target_yaw为循迹半圆后的角度
						float target_yaw=YawPID_GetTarget(&yaw_pid);
						YawPID_SetTarget(&yaw_pid,target_yaw+180);
						turn_flag_D=1;
					}
				}
				break;

            case POINT_FINAL://到达目标点
                circle_count++;//增加圈数
                if(circle_count==3)//完成3圈后停车
                {
                    Mode4_Exit();
                }
                //刷新状态
                state_reset();
                break;
    		default:
    		    break;
    		}

            if(!turnByAngle_IsBusy())//转向期间不直行也不循迹
    		{
		        if(Black_Flag==0)//不在黑线上
    		    {
    		        static uint32_t last_yaw_time = 0;
					SCHEDULE(last_yaw_time,20,walkStraight_Yaw(&yaw_pid));//20ms调用一次直行函数
    		    }
    		    else//在黑线上
    		    {
    		        IRTracking_ResetPID(&track_pid);//重置PID参数
    		        static uint32_t last_track_time = 0;
					SCHEDULE(last_track_time,20,LineWalking(&track_pid));//20ms调用一次循迹函数
    		    }
    		}
		}
	}
}

void Mode4_Tick(void)
{
	
}
void Mode4_Exit(void)
{
    setModeLoopFlag(0);               //关闭循环flag
	Stop_Num=0;                     //清零停车点
    g_speed = 0;   		//清零目标速度
	Motion_Stop(STOP_BRAKE);		//优先刹车
    walkStraight_Yaw_Reset(&yaw_pid);	//重置直行函数参数
	YawPID_Unlock(&yaw_pid);//解锁朝向
	Black_Check_Reset();			//重置黑线判断
	turnByAngle_Reset();            //重置转向状态机
	//OLED_ShowString_Grid(3,0,"Mode4_Exit",1,0,1);//显示退出信息
}
