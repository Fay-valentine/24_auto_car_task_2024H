#include "app_irtracking_eight.h"
#include "AllHeader.h"


#define IRTrack_Trun_KP (200)
#define IRTrack_Trun_KI (0.02) 
#define IRTrack_Trun_KD (0) 

int pid_output_IRR = 0;
u8 trun_flag = 0;

static int8_t IR_error_last = 0;
static float IRTrack_Integral = 0.0f;

/**
 * @brief 重置位置式PID算法的积分项
 */
void IR_PID_Reset(void)
{
	IR_error_last = 0;
	IRTrack_Integral = 0.0f;
}

#define IRR_SPEED 			  250  //巡线速度	Line patrol speed

int g_IR_track_speed=IRR_SPEED;//全局巡线速度



/**
 * @brief 根据当前传感器偏差值，通过位置式PID算法计算用于转向的PWM差速值
 * 
 * @param actual_value 当前的偏差值（来自对八路红外循迹信号的判断逻辑）
 * @return float 返回限幅后的转向差速值
 * @note 在LineWalking被调用，获取转向差速值
 */
float APP_IR_PID_Calc(int8_t actual_value)
{

	float IRTrackTurn = 0;
	int8_t error;
	
	error=actual_value;
	
	IRTrack_Integral +=error;
	
	//位置式pid	Position pid
	IRTrackTurn=error*IRTrack_Trun_KP
							+IRTrack_Trun_KI*IRTrack_Integral
							+(error - IR_error_last)*IRTrack_Trun_KD;
	


	if (IRTrackTurn > (MAX_SPEED - MOTOR_DEAD_ZONE))
			IRTrackTurn = (MAX_SPEED - MOTOR_DEAD_ZONE);
	if (IRTrackTurn < (MOTOR_DEAD_ZONE - MAX_SPEED))
			IRTrackTurn = (MOTOR_DEAD_ZONE - MAX_SPEED);
	
	IR_error_last = error;
	return IRTrackTurn;
}

/**
 * @brief 获取八路红外循迹模块的数据
 */
void deal_IRdata(u8 *x1,u8 *x2,u8 *x3,u8 *x4,u8 *x5,u8 *x6,u8 *x7,u8 *x8)
{

	
*x1 = IR_Data_Number[0];
*x2 = IR_Data_Number[1];
*x3 = IR_Data_Number[2];
*x4 = IR_Data_Number[3];
*x5 = IR_Data_Number[4];
*x6 = IR_Data_Number[5];
*x7 = IR_Data_Number[6];
*x8 = IR_Data_Number[7];
}

/**
 * @brief 核心循迹逻辑。读取8路传感器，根据预设的几种传感器组合模式判断小车偏移量，计算偏差 err，调用PID获得差速，并更新目标值
 * 
 */
void LineWalking(void)
{
	static int8_t err = 0;
	static u8 x1,x2,x3,x4,x5,x6,x7,x8;
	
	deal_IRdata(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
		
	if(x3 == 0 &&  x4 == 0  && x5 == 0 && x6 == 0 ) //俩边都亮，直跑	Both sides are bright, run straight
	{
		err = 0;
		if(trun_flag == 1)
		{
			trun_flag = 0;//走到圈了	Got into the circle
		}
	}
	
	//优先判断	Priority judgment
 else if(x1 == 0 &&  x3 == 0 && x4 == 0 && x5 == 0 && x8 == 0 )
	{
		err = 0;
	}
	else if(x1 == 1 && x2 == 1 &&x3 == 1 &&  x4 == 1  && x5 == 1 && x6  == 1 && x7 == 1 && x8 == 1 ) //过锐角	Over sharp angle
	{
		if(trun_flag == 0) //出线了	Qualified
		{
			err = 0; 
			trun_flag = 1;
		}
		//其它情况保持上个状态	Stay in the previous state in other situations
	}
	

	//添加直角	Add a right angle
	else if((x1 == 0 || x2 == 0 ) && x8 == 1) 
	{
		err = -30; 
		//delay_ms(100);
	}
	//添加直角	Add a right angle
	else if((x7 == 0 ||  x8 == 0) && x1 == 1) 
	{
		err = 30 ;
		//delay_ms(100);
	}
	


	else if(x1 == 1 && x2 == 1  && x3 == 1&& x4 == 0 && x5 == 1 && x6 == 1  && x7 == 1 && x8 == 1) // 1110 1111
	{
		err = -1;
	}
	else if(x1 == 1 && x2 == 1  && x3 == 0&& x4 == 0 && x5 == 1 && x6 == 1  && x7 == 1 && x8 == 1) // 1100 1111
	{
		err = -2;
	}
	else if(x1 == 1 && x2 == 1  && x3 == 0&& x4 == 1 && x5 == 1 && x6 == 1  && x7 == 1 && x8 == 1) // 1101 1111
	{
		err = -2;
	}
	
	else if(x1 == 1 && x2 == 0  && x3 == 0&& x4 == 1 && x5 == 1 && x6 == 1  && x7 == 1 && x8 == 1) // 1001 1111
	{
		err = -3;
	}

	else if(x1 == 1 && x2 == 1  && x3 == 1&& x4 == 1 && x5 == 0 && x6 == 1  && x7 == 1 && x8 == 1) // 1111 0111
	{
		err = 1;
	} 
	else if(x1 == 1 && x2 == 1  && x3 == 1&& x4 == 1 && x5 == 0 && x6 == 0  && x7 == 1 && x8 == 1) // 1111 0011
	{
		err = 2;
	}
	else if(x1 == 1 && x2 == 1  && x3 == 1&& x4 == 1 && x5 == 1 && x6 == 0  && x7 == 1 && x8 == 1) // 1111 1011
	{
		err = 2;
	}
	else if(x1 == 1 && x2 == 1  && x3 == 1&& x4 == 1 && x5 == 1 && x6 == 0  && x7 == 0 && x8 == 1) // 1111 1001
	{
		err = 3;
	}
	
 
	else if(x1 == 1 &&x2 == 1 &&x3 == 1 && x4 == 0 && x5 == 0 && x6 == 1 && x7 == 1&& x8 == 1) //直走	Go straight
	{
		err = 0;
	}
	
	
	
	//剩下的就保持上一个状态	The rest will remain in the previous state
	pid_output_IRR = (int)(APP_IR_PID_Calc(err));

	Motion_Car_Control(g_IR_track_speed, 0, pid_output_IRR);//改变转向差速值

}


/**
 * @brief 检测现在位于黑线还是在白线上
 * 
 * @return int 0：在黑线上  1：在白线上
 */
int LineCheck(void)
{
	int if_have = 0;
    static u8 x1,x2,x3,x4,x5,x6,x7,x8;
	deal_IRdata(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
	//Traversing the grayscale sensor
	if(!x1)
	{
		if_have = 1;
	}
    if(!x2)
	{
		if_have = 1;
	}
    if(!x3)
	{
		if_have = 1;
	}
	if(!x4)
	{
		if_have = 1;
	}
	if(!x5)
	{
		if_have = 1;
	}
	if(!x6)
	{
		if_have = 1;
	}
    if(!x7)
	{
		if_have = 1;
	}
    if(!x8)
	{
		if_have = 1;
	}
		
	//If a black line is detected
	if(if_have >= 1) 
	{
		  return IR_BLACK;
	}
	else
	{
		  return IR_WHITE;
	}
}