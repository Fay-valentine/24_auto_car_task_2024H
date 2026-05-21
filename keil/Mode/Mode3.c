#include "Mode3.h"
#include "AllHeader.h"

/*模式3*/
#define POINT_A      1
#define POINT_C      2   //先到达C点，再到B点
#define POINT_B      3
#define POINT_D      4

static uint8_t turn_flag_A=0;//是否右转39°  1：已转  0：未转
static uint8_t turn_flag_C=0;//是否左转39°  1：已转  0：未转
static uint8_t turn_flag_B=0;//是否左转39°  1：已转  0：未转
static uint8_t turn_flag_D=0;//是否右转39°  1：已转  0：未转
uint8_t Black_Flag=0;//是否在黑线上  1：在  0：不在

void Mode3_Init(void)
{
	g_IR_track_speed=250;		//设置速度
	Mode_Loop_flag=1;			//循环开启flag
	Stop_Num=5;					//在第5个点停车，即第二次的A点

	Yaw_sample();//采样一次目标航向角

	//显示相应信息
	OLED_ShowNum_Grid(1,15,Mode_Loop_flag,1,1,0,1);     //显示循环开启flag
	OLED_ShowNum_Grid(1,10,Stop_Num,1,1,0,1);           //显示停车点
	//OLED_ShowString_Grid(2,0,"Mode3_Init",1,0,1);       //显示初始化信息
}

//轨迹：A->C->B->D->A2
void Mode3_Loop(void)
{
	if(Mode_Loop_flag)//只有在初始化设置了 循环开启flag 为1时 才执行循环Loop
	{
        Black_Flag=get_is_black();//检测是否在黑线上

		if(g_IR_track_speed)
		{
			switch (point_count)
    		{
    		case POINT_A://右转39°
				if(turn_flag_A==0)
				{
					turnByAngle(1,39);
					turn_flag_A=1;
				}
    		    break;
			
			case POINT_C://左转39°
				if(turn_flag_C==0)
				{
					turnByAngle(-1,39);
					//更新target_yaw为循迹半圆后的角度
					target_yaw=target_yaw-180;
					turn_flag_C=1;
				}
				break;

			case POINT_B://左转39°
				if(turn_flag_B==0)
				{
					turnByAngle(-1,39);
					turn_flag_B=1;
				}
				break;
				
			case POINT_D://右转39°
				if(turn_flag_D==0)
				{
					turnByAngle(1,39);
					//更新target_yaw为循迹半圆后的角度
					target_yaw=target_yaw+180;
					turn_flag_D=1;
				}
				break;
    		default:
    		    break;
    		}

           if(Black_Flag==0)//不在黑线上
    		{
    		    StraightLineWalk_IMU();//直行
    		}
    		else//在黑线上
    		{
    		    IR_PID_Reset();//重置PID参数
    		    LineWalking();//循迹
    		}
		}

	}

	
}

void Mode3_Tick(void)
{
	
}
void Mode3_Exit(void)
{
    Mode_Loop_flag=0;               //关闭循环flag
	Stop_Num=0;                     //清零停车点
    g_IR_track_speed = 0;   		//清零目标速度
	turn_flag_A=0;                  //重置转向标志
	turn_flag_C=0;
	turn_flag_B=0;
	turn_flag_D=0;
	Motion_Stop(STOP_BRAKE);		//优先刹车
    StraightLineWalk_IMU_Reset();	//重置直行函数
	Yaw_Unlock();//解锁朝向
	Black_Check_Reset();			//重置黑线判断
	//OLED_ShowString_Grid(3,0,"Mode3_Exit",1,0,1);//显示退出信息
}
