#include "Global.h"
#include "AllHeader.h"
uint8_t KeyNum;
extern uint8_t Next_Mode;//在main中定义的
volatile uint8_t Mode_Loop_flag=0;

void Global_LF_refresh(void);
void Global_yaw_refresh(void);
void Global_blackLine_check(void);

void setModeLoopFlag(uint8_t flag)
{
    Mode_Loop_flag=flag;
}

uint8_t getModeLoopFlag(void)
{
    return Mode_Loop_flag;
}

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
		if(Next_Mode>Mode6)
		{
			Next_Mode=-1;//切回模式1
		}
	}

    //刷新yaw值
    Global_yaw_refresh();

	//黑线检测
    Global_blackLine_check();
	
	//低频刷新IR,yaw,point_count,black_flag,200ms
	Global_LF_refresh();
	
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

/**
 * @brief 低频刷新2行IR,3行yaw,4行point_count,black_flag,200ms
 */
void Global_LF_refresh(void)
{ 
	static uint32_t last_oled = 0;
    if (Get_Time() - last_oled > 200)
    {
        //显示 8 位数据
        sprintf((char*)oledbuf, "%d%d%d%d%d%d%d%d", 
            IR_Data_Number[0], IR_Data_Number[1],
            IR_Data_Number[2], IR_Data_Number[3],
            IR_Data_Number[4], IR_Data_Number[5],
            IR_Data_Number[6], IR_Data_Number[7]);
        OLED_ShowString_Grid(2, 3, (char*)oledbuf, 1, 0, 1);//刷新IR数据

        
        OLED_ShowSNum_Grid(3,6,YawPID_GetTarget(&yaw_pid),4,1,0,1);//更新显示一次target_yaw
        OLED_ShowSNum_Grid(3,16,Get_Yaw(),4,1,0,0);//刷新yaw

        // OLED_ShowNum_Grid(4,6,point_count,1,1,0,1);//刷新point_count
        
		
        last_oled = Get_Time();
    }
}

void Global_yaw_refresh(void)
{
    static uint32_t last_yaw_time = 0;
	SCHEDULE(last_yaw_time, 20, Yaw_Update());
}

void Global_blackLine_check(void)
{
	if(getModeLoopFlag())
    {
        //10ms以上轮询,判断是否在黑线上
	    static uint32_t last_black_time = 0;
		SCHEDULE(last_black_time, 20, Black_Check(Stop_Num));
    }
}