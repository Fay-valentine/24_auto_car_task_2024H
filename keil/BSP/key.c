#include "key.h"
#include "AllHeader.h"

static uint8_t KeyNum=0;

uint8_t Key1_is_Press(void)
{
    if((DL_GPIO_readPins(KEY_button1_PORT,KEY_button1_PIN) & KEY_button1_PIN)!=0)//��������
    {
        return KEY_PRESS;//����1
    }
    else
    {
        return KEY_RELEASE;//����0
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

//��ȡ����ֵ
uint8_t Key_GetNum(void)
{
	if(KeyNum)
	{
		uint8_t temp=KeyNum;
		KeyNum=0;//���㣬��ֹһֱ���ؼ�ֵ
		return temp;
	}
	return 0;
}

/**
 * @brief ����״̬���
 * @return ���� ��ֵ
 */
uint8_t Key_State(void)
{
	if(Key1_is_Press()==KEY_PRESS)//����1������
	{
		return KEY1;
	}
	if(Key2_is_Press()==KEY_PRESS)//����2������
	{
		return KEY2;
	}
	if(Key3_is_Press()==KEY_PRESS)//����2������
	{
		return KEY3;
	}
	return 0;
}

/**
 * @brief 20ms�����������������ʱ�̼�ֵ״̬ 
 * 
 */
void Key_Tick(void)
{
	static uint8_t count=0;//��̬���������������20����
	static uint8_t pre_state,cur_state;//��������ʱ��״̬��staticĬ��ֵΪ0
	count++;
	if(count>=20)
	{
		count=0;

		pre_state=cur_state;
		cur_state=Key_State();//��ȡ��ǰ��ֵ
		if(pre_state!=0 && cur_state==0)//˵���а�������
		{
			KeyNum=pre_state;
		}
	}
}

uint8_t Key_Scan(void)//�����������ؼ�ֵ�汾
{
    //���º��������ؼ�ֵ������Ա��ְ��»᷵��0����ֻ֤����һ��keyx ��ֵx
    static uint8_t release=1;//��ʼΪ�ɿ�״̬
    volatile uint8_t ret=0;//ÿ��ѭ����ʼret������
    //release=1ʱ����⵽key1 or key2���£�release=0������״̬
    if(release==1&&(Key1_is_Press()==KEY_PRESS||Key2_is_Press()==KEY_PRESS||Key3_is_Press()==KEY_PRESS))
    {
        delay_ms(10);//����
        release=0;
        if(Key1_is_Press()==KEY_PRESS)
        {
            //��ֵ������ֱ�ӷ��أ����ɿ����ٷ���
            ret = KEY1;//ֵΪ1
        }
        else if(Key2_is_Press()==KEY_PRESS)
        {
            ret = KEY2;//ֵΪ2
        }
        else if(Key3_is_Press()==KEY_PRESS)
        {
            ret = KEY3;
        }
    }
    else if(Key1_is_Press()==KEY_RELEASE && Key2_is_Press()==KEY_RELEASE && Key3_is_Press()==KEY_RELEASE)//key1 or key2���ɿ�
    {
        release=1;//��Ϊ�ɿ�״̬
    }
    return ret;
}


uint8_t switch_mode(void)
{
	uint8_t	select_mode = 1, key_num = 0;
	OLED_Clear();
	OLED_ShowString_Grid(1,0,"Select Mode:",1,1,1);
	OLED_ShowNum_Grid(1,13,select_mode,1,1,0,1);
    OLED_ShowString_Grid(2,0,"bias:",1,1,0);
    OLED_ShowSNum_Grid(2,5,turn_adjust,3,1,0,1);
	OLED_Refresh();
	
	while(1)
    {
		key_num = Key_Scan();
		if(key_num)
        {
			if(key_num == KEY1)
            {
				select_mode +=1;
				if(select_mode > 6 )
				{
					select_mode = 0;
				}
				OLED_ShowNum_Grid(1,13,select_mode,1,1,0,1);
				OLED_Refresh();
			}
			else if(key_num == KEY2)
            {
				OLED_ShowString(92,0,"OK!!!",8,1);
				OLED_Refresh();
				delay_ms(700);
				OLED_Clear();
				break;
			}
			else if(key_num == KEY3)//��ͷ����ֵ����
            {
                if(turn_adjust>50)
                {
                    turn_adjust=-50.0f;
                }
                turn_adjust+=5.0f;
                OLED_ShowSNum_Grid(2,5,turn_adjust,3,1,0,1);
            }
		}
        delay_ms(10);
	}
	return select_mode;
}

