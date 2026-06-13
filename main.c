/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "AllHeader.h"

int8_t Cur_Mode=0,Next_Mode=0;

//OLED显示：
//1行：模式	 停车点  循环flag
//2行：八路红外的8路状态
//3行：target_yaw和cur_yaw
//4行：point_count

int main(void)//需要调试循迹PID参数
{
    Global_Init();
    delay_ms(500);
    uart0_send_string("$0,0,1#");//八路红外模块启动指令

	OLED_ShowString_Grid(1,0,"Mode:",1,1,1);
	OLED_ShowNum_Grid(1,5,Next_Mode,1,1,0,1);

	OLED_ShowString_Grid(2,0,"IR:",1,1,1);//八路红外的8路状态

	OLED_ShowString_Grid(3,0,"t_yaw:",1,1,1);//目标yaw
    OLED_ShowString_Grid(3,12,"yaw:",1,0,1);//原始yaw
    
	OLED_ShowString_Grid(4,0,"P:",1,1,1);
	OLED_ShowString_Grid(4,5,"I:",1,0,1);
	OLED_ShowString_Grid(4,10,"D:",1,0,1);
    //OLED_ShowString_Grid(4,0,"point:",1,1,1);//点数
    //OLED_ShowString_Grid(4,9,"black_f:",1,0,1);//黑线flag

	
	while(1)
	{
		Global_Loop();
		
		uint8_t key = Key_Scan();

		if(key == KEY1)//直走
		{
		    sampleYaw(&yaw_pid);        // 采样锁定目标航向角
		    while(1)
		    {
		        Global_Loop();
		        static uint32_t last_time = 0;
		        if (Get_Time() - last_time >= 20) // 20ms调用一次yaw纠偏
		        {
		            walkStraight_Yaw(&yaw_pid);
		            last_time = Get_Time();
		        }
		        if(Key_Scan() == KEY3) // 再次按KEY3退出
		        {
					Motion_Stop(STOP_BRAKE);		//优先刹车
					walkStraight_Yaw_Reset(&yaw_pid);	//重置直行函数
					YawPID_Unlock(&yaw_pid);//解锁朝向
		            break;
		        }
		    }
		}
		if(key == KEY2)//运行循迹
		{
			while(1)
			{
				Global_Loop();
				//15ms调用一次LineWalking
				static uint32_t last_time = 0;
				if (Get_Time() - last_time >= 15) 
				{
				    LineWalking(&track_pid);
				    last_time = Get_Time();
				}
			}
		}
		if(key == KEY3)
		{
			
		}


	}
	
	// while(1)
	// {
		
	// 	if(Cur_Mode==Next_Mode && Key2_is_Press()==KEY_RELEASE)//运行当前模式的Loop
	// 	{
	// 		switch (Cur_Mode)
	// 		{
	// 		case 1:Mode1_Loop();break;
	// 		case 2:Mode2_Loop();break;
	// 		case 3:Mode3_Loop();break;
	// 		case 4:Mode4_Loop();break;
	// 		}
	// 	}
	// 	if(Cur_Mode!=Next_Mode && Key2_is_Press()==KEY_PRESS)//进行模式切换
	// 	{
	// 		switch (Cur_Mode)
	// 		{
	// 		case 1:Mode1_Exit();break;
	// 		case 2:Mode2_Exit();break;
	// 		case 3:Mode3_Exit();break;
	// 		case 4:Mode4_Exit();break;
	// 		}
	// 		switch(Next_Mode)
	// 		{
	// 		case 1:Mode1_Init();break;
	// 		case 2:Mode2_Init();break;
	// 		case 3:Mode3_Init();break;
	// 		case 4:Mode4_Init();break;
	// 		}
	// 		Cur_Mode=Next_Mode;
	// 	}
		
	// }
}
