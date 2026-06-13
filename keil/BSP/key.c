#include "key.h"
#include "AllHeader.h"

static uint8_t KeyNum=0;

uint8_t Key1_is_Press(void)
{
    if((DL_GPIO_readPins(KEY_button1_PORT,KEY_button1_PIN) & KEY_button1_PIN)!=0)
    {
        return KEY_PRESS;
    }
    else
    {
        return KEY_RELEASE;
    }
}

uint8_t Key2_is_Press(void)
{
    if((DL_GPIO_readPins(KEY_button2_PORT, KEY_button2_PIN) & KEY_button2_PIN) ==0)
    {
        return KEY_PRESS;
    }
    else
    {
        return KEY_RELEASE;
    }
}

uint8_t Key3_is_Press(void)
{
    if((DL_GPIO_readPins(KEY_button3_PORT, KEY_button3_PIN) & KEY_button3_PIN) ==0)
    {
        return KEY_PRESS;
    }
    else
    {
        return KEY_RELEASE;
    }
}


uint8_t Key_GetNum(void)
{
	if(KeyNum)
	{
		uint8_t temp=KeyNum;
		KeyNum=0;
		return temp;
	}
	return 0;
}

/**
 * @brief 按键状态检测
 * @return 按键 键值
 */
uint8_t Key_State(void)
{
	if(Key1_is_Press()==KEY_PRESS)//按键1被按下
	{
		return KEY1;
	}
	if(Key2_is_Press()==KEY_PRESS)//按键2被按下
	{
		return KEY2;
	}
	if(Key3_is_Press()==KEY_PRESS)//按键2被按下
	{
		return KEY3;
	}
	return 0;
}

/**
 * @brief 20ms计数器，并检测两个时刻键值状态 
 * 
 */
void Key_Tick(void)
{
	static uint8_t count=0;//静态，毫秒计数器，满20归零
	static uint8_t pre_state,cur_state;//按键两个时刻状态，static默认值为0
	count++;
	if(count>=20)
	{
		count=0;

		pre_state=cur_state;
		cur_state=Key_State();//读取当前键值
		if(pre_state!=0 && cur_state==0)//说明有按键按下
		{
			KeyNum=pre_state;
		}
	}
}

uint8_t Key_Scan(void)//按下立即返回键值版本
{
    //按下后立即返回键值，随后仍保持按下会返回0，保证只返回一次keyx 键值x
    static uint8_t release=1;//初始为松开状态
    volatile uint8_t ret=0;//每次循环开始ret都归零
    //release=1时，检测到key1 or key2按下，release=0，按下状态
    if(release==1&&(Key1_is_Press()==KEY_PRESS||Key2_is_Press()==KEY_PRESS||Key3_is_Press()==KEY_PRESS))
    {
        delay_ms(10);//消抖
        release=0;
        if(Key1_is_Press()==KEY_PRESS)
        {
            //赋值而不是直接返回，等松开后再返回
            ret = KEY1;//值为1
        }
        else if(Key2_is_Press()==KEY_PRESS)
        {
            ret = KEY2;//值为2
        }
        else if(Key3_is_Press()==KEY_PRESS)
        {
            ret = KEY3;
        }
    }
    else if(Key1_is_Press()==KEY_RELEASE && Key2_is_Press()==KEY_RELEASE && Key3_is_Press()==KEY_RELEASE)//key1 or key2都松开
    {
        release=1;//设为松开状态
    }
    return ret;
}


uint8_t switch_mode(void)
{
	uint8_t	select_mode = 1, key_num = 0;
	OLED_Clear();
	OLED_ShowString_Grid(1,0,"Select Mode:",1,1,1);
	OLED_ShowNum_Grid(1,13,select_mode,1,1,0,1);
	OLED_Refresh();
	
	while(1)
    {
		key_num = Key_Scan();
		if(key_num)
        {
			if(key_num == KEY1)
            {
				
			}
			else if(key_num == KEY2)
            {
				
			}
			else if(key_num == KEY3)
            {
                
            }
		}
        delay_ms(10);
	}
	return select_mode;
}

