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

//调整角度，调整里程

int main(void)//需要调试循迹PID参数
{
    Global_Init();
    delay_ms(500);
    uart0_send_string("$0,0,1#");//八路红外模块启动指令

	OLED_ShowString_Grid(1,0,"Mode:",1,1,1);
	OLED_ShowNum_Grid(1,5,Next_Mode,1,1,0,1);
	OLED_ShowString_Grid(1,7,"V:",1,0,1);

	OLED_ShowString_Grid(2,0,"IR:",1,1,1);//八路红外的8路状态
	
	 OLED_ShowString_Grid(2,11,"Od",1,0,1);//目标里程

	OLED_ShowString_Grid(3,0,"t_yaw:",1,1,1);//目标yaw
    OLED_ShowString_Grid(3,12,"yaw:",1,0,1);//原始yaw
    
	OLED_ShowString_Grid(4,0,"P:",1,1,1);
	OLED_ShowString_Grid(4,5,"D:",1,0,1);
	
    // OLED_ShowString_Grid(4,0,"point:",1,1,1);//点数
    // OLED_ShowString_Grid(4,8,"y_adjust",1,0,1);//yaw调整

	
	// while(1)
	// {
	// 	static uint32_t last_time=0;
	// 	SCHEDULE(last_time, 20, IRTracking_test());
	// }
		
	

	
	while(1)
	{
		Global_Loop();
		uint8_t key = Key_Scan();
		if(Next_Mode==Mode2)//选中mode2，调整yaw，用于mode2的180度翻转，和mode3,mode4点B转回角度的调整
		{
			if(key == KEY3)
			{
				yaw_adjust++;
	        	if(yaw_adjust>16)
	        	{
	        	    yaw_adjust=-15;
	        	}
	        	OLED_ShowSNum_Grid(4,17,yaw_adjust,3,1,0,1);//每次按键都刷新yaw调整
			}
		}
		else if(Next_Mode==Mode3)//选中mode3，调整mode3和mode4的目标里程
		{
			if(key == KEY3)
			{
				base_target_od+=5.0f;
	        	if(base_target_od>141.0f)
	        	{
	        	    base_target_od=70.0f;
	        	}
	        	OLED_ShowSNum_Grid(2,13,(int16_t)base_target_od,3,1,0,1);//每次按键都刷新目标里程
			}
		}
		if(Next_Mode==5)
		{
			if(key == KEY3)//调整KP
    		{
    		    track_pid.Kp+=5.0f;
    		    OLED_ShowNum_Grid(4,2,track_pid.Kp,3,1,0,1);
    		    if(track_pid.Kp>121.0f)
    		    {
    		        track_pid.Kp=0.0f;
    		    }
    		}
    		
		}
		if(Next_Mode==6)
		{
			if(key == KEY3)//调整KD
    		{
    		    track_pid.Kd+=5.0f;
    		    OLED_ShowNum_Grid(4,7,track_pid.Kd,3,1,0,1);
    		    if(track_pid.Kd>50.0f)
    		    {
    		        track_pid.Kd=0.0f;
    		    }
    		}
		}
		if(Next_Mode==7)
		{
			if(key == KEY3)//调整循迹速度
    		{
    		    g_IR_track_speed+=10.0f;
    		    OLED_ShowNum_Grid(1,9,g_IR_track_speed,3,1,0,1);
    		    if(g_IR_track_speed>301)
    		    {
    		        g_IR_track_speed=50.0f;
    		    }
    		}
		}
		
		if(Cur_Mode==Next_Mode && Key2_is_Press()==KEY_RELEASE)//运行当前模式的Loop
		{
			switch (Cur_Mode)
			{
			case 1:Mode1_Loop();break;
			case 2:Mode2_Loop();break;
			case 3:Mode3_Loop();break;
			case 4:Mode4_Loop();break;
			}
		}
		if(Cur_Mode!=Next_Mode && Key2_is_Press()==KEY_PRESS)//进行模式切换
		{
			switch (Cur_Mode)
			{
			case 1:Mode1_Exit();break;
			case 2:Mode2_Exit();break;
			case 3:Mode3_Exit();break;
			case 4:Mode4_Exit();break;
			}
			switch(Next_Mode)
			{
			case 1:Mode1_Init();break;
			case 2:Mode2_Init();break;
			case 3:Mode3_Init();break;
			case 4:Mode4_Init();break;
			}
			Cur_Mode=Next_Mode;
		}
		
	}


}
