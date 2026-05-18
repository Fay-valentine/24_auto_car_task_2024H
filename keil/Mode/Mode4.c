#include "Mode4.h"
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
	g_IR_track_speed=250;		//设置速度
	Mode_Loop_flag=1;			//循环开启flag
	Stop_Num=13;					//在第5个点停车，即第二次的A点
	//显示相应信息
	OLED_ShowNum_Grid(1,15,Mode_Loop_flag,1,1,0,1);     //显示循环开启flag
	OLED_ShowNum_Grid(1,10,Stop_Num,1,1,0,1);           //显示停车点
	OLED_ShowString_Grid(2,0,"Mode4_Init",1,0,1);       //显示初始化信息
	
}

//轨迹：A->C->B->D->A2
void Mode4_Loop(void)
{
	if(Mode_Loop_flag)//只有在初始化设置了 循环开启flag 为1时 才执行循环Loop
	{
        Black_Flag=get_is_black();//检测是否在黑线上
		if(g_IR_track_speed!=0)
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

            if(Black_Flag==0)//不在黑线上
            {
                StraightLineWalk_IMU();//直行
            }
            else//在黑线上
            {
                LineWalking();//循迹
            }
		}

		//10ms以上轮询,判断是否在黑线上
		static uint32_t last_black_time = 0;
		if (Get_Time() - last_black_time >= 10) 
		{
		    Black_Check(Stop_Num);
		    last_black_time = Get_Time();
		}
	}

	
}

void Mode4_Tick(void)
{
	
}
void Mode4_Exit(void)
{
    Mode_Loop_flag=0;               //关闭循环flag
	Stop_Num=0;                     //清零停车点
    g_IR_track_speed = 0;   		//清零目标速度
	Motion_Stop(STOP_BRAKE);		//优先刹车
    StraightLineWalk_IMU_Reset();	//重置直行函数
	Black_Check_Reset();			//重置黑线判断
	OLED_ShowString_Grid(3,0,"Mode4_Exit",1,0,1);//显示退出信息
}
