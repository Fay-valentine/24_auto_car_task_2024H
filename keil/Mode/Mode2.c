#include "AllHeader.h"

/*模式2*/
static uint8_t Black_Flag=0;//是否在黑线上  1：在  0：不在

void Mode2_Init(void)
{
	g_speed=250;		//设置速度
	Mode_Loop_flag=1;			//循环开启flag
	Stop_Num=5;					//在第5个点停车，即第二次的A点
	sampleYaw(&yaw_pid);		//采样锁定目标航向角
	//显示相应信息
	OLED_ShowNum_Grid(1,15,Mode_Loop_flag,1,1,0,1);     //显示循环开启flag
	OLED_ShowNum_Grid(1,10,Stop_Num,1,1,0,1);           //显示停车点
	//OLED_ShowString_Grid(2,0,"Mode2_Init",1,0,1);       //显示初始化信息
}

void Mode2_Loop(void)//在直行函数和循迹函数之间切换，通过是否在黑线上来判断
{
	if(Mode_Loop_flag)//只有在初始化设置了 循环开启flag 为1时 才执行循环Loop
	{
        Black_Flag=get_is_black();//检测是否在黑线上
		if(g_speed!=0)
		{
            if(Black_Flag==0)//不在黑线上
            {
                walkStraight_Yaw(&yaw_pid);//直行
            }
            else//在黑线上
            {
                LineWalking();//循迹
            }
		}
	}
	
}

void Mode2_Exit(void)
{
	Mode_Loop_flag=0;               //关闭循环flag
	Stop_Num=0;                     //清零停车点
    g_speed = 0;   		//清零目标速度
	Motion_Stop(STOP_BRAKE);		//优先刹车
    walkStraight_Yaw_Reset(&yaw_pid);	//重置直行函数
	YawPID_Unlock(&yaw_pid);//解锁朝向
	Black_Check_Reset();			//重置黑线判断
	//OLED_ShowString_Grid(3,0,"Mode2_Exit",1,0,1);//显示退出信息
}
