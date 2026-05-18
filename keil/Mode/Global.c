#include "AllHeader.h"
#include "Global.h"

uint8_t KeyNum;
extern uint8_t Next_Mode;//在main中定义的
volatile uint8_t Mode_Loop_flag=0;
/*全局模式*/
void Global_Init(void)
{
	bsp_Init();//初始化
}

/**
 * @brief 1-进行模式切换  2-刷新yaw值  3-刷新point_count
 */
void Global_Loop(void)
{
	KeyNum=Key_GetNum();
	if(KeyNum == 1)
	{
		Next_Mode++;
		OLED_ShowNum_Grid(1,5,Next_Mode,1,1,0,1);//刷新模式显示
		if(Next_Mode>4)
		{
			Next_Mode=0;//切回模式1
		}
	}
	static uint32_t last_oled = 0;
	//低频刷新yaw值,200ms
    if (Get_Time() - last_oled > 200)
    {
        //OLED_ShowSNum_Grid(2,4,yaw,4,1,0,0);//刷新yaw
        OLED_ShowSNum_Grid(4,12,point_count,4,1,0,1);//刷新point_count
        last_oled = Get_Time();
    }
}
void Global_Exit(void)
{

}

/**
 * @brief 按键和RGB计时
 */
void Global_Tick(void)
{
	RGB_Tick();
    Key_Tick();
}

