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

uint8_t Cur_Mode=0,Next_Mode=0;


int main(void)
{
    Global_Init();
    delay_ms(500);
    uart0_send_string("$0,0,1#");//八路红外模块启动指令

	OLED_ShowString_Grid(1,0,"Mode:",1,1,1);
	OLED_ShowNum_Grid(1,5,Next_Mode,1,1,0,1);

    //OLED_ShowString_Grid(2,0,"yaw:",1,1,1);//原始yaw
    //OLED_ShowString_Grid(3,0,"target_yaw:",1,1,1);//目标yaw
    OLED_ShowString_Grid(4,0,"point_count:",1,1,1);//目标yaw
	while(1)
	{
		Global_Loop();
		if(Cur_Mode==Next_Mode && Key2_is_Press()==KEY_RELEASE)//运行当前模式的Loop
		{
			switch (Cur_Mode)
			{
			case 1:Mode1_Loop();break;
			case 2:Mode2_Loop();break;
			case 3:Mode3_Loop();break;
			}
		}
		if(Cur_Mode!=Next_Mode && Key2_is_Press()==KEY_PRESS)//进行模式切换
		{
			switch (Cur_Mode)
			{
			case 1:Mode1_Exit();break;
			case 2:Mode2_Exit();break;
			case 3:Mode3_Exit();break;
			}
			switch(Next_Mode)
			{
			case 1:Mode1_Init();break;
			case 2:Mode2_Init();break;
			case 3:Mode3_Init();break;
			}
			Cur_Mode=Next_Mode;
		}
		
	}
}

