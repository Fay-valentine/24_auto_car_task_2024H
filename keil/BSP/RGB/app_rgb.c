#include "app_rgb.h"
#include "AllHeader.h"

//RGB�򵥵�Ч

#define 	Red_ON     1
#define 	Red_OFF     0

uint8_t RGB_flag ;
uint8_t RGB_On=0;
uint16_t RGB_Count=0;//��λ��1ms

//#define 	Red_RGB     '2'//����ǰ Before pressing the button
//#define 	Green_RGB    '3'//������ After pressing the button
//#define 	Blue_RGB    '4'//������ Left button
////#define 	Yellow_RGB   '4'//������ Right button
////#define 	Cyan_RGB    '5'//����ͣ Button stop
//#define 	OFF    '8'//����ͣ Button stop

extern uint8_t ProtocolString[80];//���뱸�������� Introducing backup data area
static void set_ALL_RGB_COLOR(unsigned long color)
{
    rgb_SetColor(Left_RGB,color);
    rgb_SetColor(Right_RGB,color);
}

/**
 * @brief ��RGB������RGB_On��־λ��������,�ɶ�ʱ���Զ��ص�
 * @param turnOn true:����
 * @param color ��ɫ
 */
void Set_RGB(bool turnOn,RGB_Color_t color)
{
	
	if(turnOn)
	{
		RGB_On=1;
		RGB_Count = 0;          // ���õ���ʱ�����¿�ʼ��ʱ
		Control_RGB_ALL(color);//����
	}
	else
	{
		RGB_On=0;
		RGB_Count = 0;          // ���õ���ʱ�����¿�ʼ��ʱ
		Control_RGB_ALL(OFF);//�ص�
	}
}

/**
 * @brief RGB��ʱ����������RGB��һ��
 * 
 */
void RGB_Tick(void)
{
	
	if(RGB_On)
	{
		RGB_Count++;
	}
	if(RGB_Count>=1000)//��1s
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
    //���Ͳʵ�����
    ws2812_send();
    
}

