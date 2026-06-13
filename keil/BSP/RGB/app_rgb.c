#include "app_rgb.h"
#include "AllHeader.h"

#define 	Red_ON     1
#define 	Red_OFF     0

uint8_t RGB_flag ;
uint8_t RGB_On=0;
uint16_t RGB_Count=0;//1ms

//#define 	Red_RGB     '2'//按键前 Before pressing the button
//#define 	Green_RGB    '3'//按键后 After pressing the button
//#define 	Blue_RGB    '4'//按键左 Left button
////#define 	Yellow_RGB   '4'//按键右 Right button
////#define 	Cyan_RGB    '5'//按键停 Button stop
//#define 	OFF    '8'//按键停 Button stop

extern uint8_t ProtocolString[80];//引入备份数据区 Introducing backup data area

static void set_ALL_RGB_COLOR(unsigned long color)
{
    rgb_SetColor(Left_RGB,color);
    rgb_SetColor(Right_RGB,color);
}

/**
 * @brief 设置RGB灯为点亮状态和颜色
 * @param turnOn true:开启
 * @param color 颜色
 */
void Set_RGB(bool turnOn,RGB_Color_t color)
{
	
	if(turnOn)
	{
		RGB_On=1;
		RGB_Count = 0;          
		Control_RGB_ALL(color);
	}
	else
	{
		RGB_On=0;
		RGB_Count = 0;          
		Control_RGB_ALL(OFF);
	}
}

/**
 * @brief RGB灯定时器，执行点亮一秒后关闭
 * 
 */
void RGB_Tick(void)
{
	
	if(RGB_On)
	{
		RGB_Count++;
	}
	if(RGB_Count>=1000)//
	{
		RGB_Count=0;
		Set_RGB(false,OFF);
	}
}

void Control_RGB_ALL(RGB_Color_t color)
{
    switch(color)
    {
        case    Red_RGB:     set_ALL_RGB_COLOR(RED);break;
				case    Orange_RGB:    set_ALL_RGB_COLOR(ORANGE);break;
        case    Green_RGB:   set_ALL_RGB_COLOR(GREEN);break;
        case    Blue_RGB:    set_ALL_RGB_COLOR(BLUE);break;
        case    Yellow_RGB:  set_ALL_RGB_COLOR(YELLOW);break;
        case    Purple_RGB:  set_ALL_RGB_COLOR(PURPLE); break;  
        case    Cyan_RGB:    set_ALL_RGB_COLOR(CYAN);break;
				case    White_RGB:   set_ALL_RGB_COLOR(WHITE);break;
        case    OFF  :       set_ALL_RGB_COLOR(BLACK);break;
			
        
        default : return;
        
    }

    ws2812_send();
}

