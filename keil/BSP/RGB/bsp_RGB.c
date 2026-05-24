#include "bsp_RGB.h"
#include "AllHeader.h"

#define LED_NUM 1          // LED����
#define DELAY_0H 3         // 0��ߵ�ƽʱ��(us)
#define DELAY_0L 9         // 0��͵�ƽʱ��(us)
#define DELAY_1H 6         // 1��ߵ�ƽʱ��(us)
#define DELAY_1L 6         // 1��͵�ƽʱ��(us)
#define RESET_DELAY 50     // ��λʱ��(us)

static uint8_t g_breathe_cnt = 0;   // 0..255
unsigned char LedsArray[WS2812_MAX * 3];      //������ɫ���ݴ洢����
unsigned char LedsArray1[255];      //������ɫ���ݴ洢����
unsigned int  ledsCount   = WS2812_NUMBERS;   //����ʵ�ʲʵ�Ĭ�ϸ���
unsigned int  nbLedsBytes = WS2812_NUMBERS*3; //����ʵ�ʲʵ���ɫ���ݸ���

unsigned int  nbLedsBytes1 = 255; //����ʵ�ʲʵ���ɫ���ݸ���
unsigned int colorIndex = 0;

// ��ʱ0.25us
void delay_0_25us(void)
{
    //volatile
    for( int i = 0; i < 13; i++); //5:32MHZ  13��80Mhz
}

/******************************************************************
 * �� �� �� �ƣ�rgb_SetColor
 * �� �� ˵ �������òʵ���ɫ
 * �� �� �� �Σ�LedId���Ƶĵڼ�����  color��ɫ����
 * �� �� �� �أ���
******************************************************************/

void rgb_SetColor(unsigned char LedId, unsigned long color)
{
    if( LedId > ledsCount )
    {
        return;    //to avoid overflow
    }
    LedsArray[LedId * 3]     = (color>>8)&0xff;
    LedsArray[LedId * 3 + 1] = (color>>16)&0xff;
    LedsArray[LedId * 3 + 2] = (color>>0)&0xff;
}

/******************************************************************
 * �� �� �� �ƣ�rgb_SetRGB
 * �� �� ˵ �������òʵ���ɫ(��ԭɫ����)
 * �� �� �� �Σ�LedId���Ƶĵڼ����� red��ɫ����  green��ɫ����  blue��ɫ����
 * �� �� �� �أ���
******************************************************************/
void rgb_SetRGB(unsigned char LedId, unsigned long red, unsigned long green, unsigned long blue)
{
    unsigned long Color=red<<16|green<<8|blue;
    rgb_SetColor(LedId,Color);
}

void GET_RGB(void)
{
    for (unsigned int i = 0; i < 1; i++)
    {
        rgb_SetColor(i, rgbColors[colorIndex]);
    }
	rgb_SendArray();

	colorIndex++;
    colorIndex = colorIndex % RGB_COLOR_COUNT;
				
	for (unsigned int i = 0; i < 2; i++)
    {
        rgb_SetColor(i, rgbColors[colorIndex]);
    }
	rgb_SendArray();

	colorIndex++;
    colorIndex = colorIndex % RGB_COLOR_COUNT;
}

/******************************************************************
 * �� �� �� �ƣ�rgb_SendArray
 * �� �� ˵ �������Ͳʵ�����
 * �� �� �� �Σ���
 * �� �� �� �أ���
******************************************************************/
void rgb_SendArray(void)
{
    unsigned int i;
    //��������
    for(i=0; i<nbLedsBytes; i++)
        Ws2812b_WriteByte(LedsArray[i]);
}
/******************************************************************
 * �� �� �� �ƣ�rgb_SendArray
 * �� �� ˵ �������Ͳʵ�����
 * �� �� �� �Σ���
 * �� �� �� �أ���
******************************************************************/
void rgb_SendArray1(void)
{
    unsigned int i;
    //��������
    for(i=0; i<nbLedsBytes1; i++)
        Ws2812b_WriteByte(LedsArray1[i]);
}
/******************************************************************
 * �� �� �� �ƣ�RGB_LED_Reset
 * �� �� ˵ ������λws2812
 * �� �� �� �Σ���
 * �� �� �� �أ���
******************************************************************/
void RGB_LED_Reset(void)
{
        RGB_PIN_L();              
        delay_us(285);
}

/******************************************************************
 * �� �� �� �ƣ�Ws2812b_WriteByte
 * �� �� ˵ ������WS2812д�뵥�ֽ�����
 * �� �� �� �Σ�byteд����ֽ�����
 * �� �� �� �أ���
 * ��       ע��1���ʱ�� = �ߵ�ƽ580ns~1us    �ٵ͵�ƽ220ns~420ns
 *              0���ʱ�� = �ߵ�ƽ220ns~380ns  �ٵ͵�ƽ580ns~1us
******************************************************************/
void Ws2812b_WriteByte(unsigned char byte)
{
    int i = 0, j = 0, k = 0;
        for(i = 0; i < 8; i++ )
        {
            if( byte & (0x80 >> i) )//��ǰλΪ1
            { 
                RGB_PIN_H();

                //0.75us
                delay_us(1);


                RGB_PIN_L();

                delay_0_25us(); //0.25us
            }
            else//��ǰλΪ0
            {
                RGB_PIN_H();
                
                delay_0_25us(); //0.25us
                RGB_PIN_L();

                //0.833us
                delay_us(1);
            }
        }
}
  
void ws2812_send(void)
{
    __disable_irq(); // �ر��ж�ȷ��ʱ��׼ȷ
    
    for(int i = 0; i < WS2812_MAX*3 ; i++) {
        uint8_t byte_val = LedsArray[i];
        
        for(int bit = 7; bit >= 0; bit--) {
            if(byte_val & (1 << bit)) {
                // ����'1'��
                RGB_PIN_H();
                delay_us(1);
								RGB_PIN_L()  ;
                delay_0_25us();
            } else {
                // ����'0'��
                 RGB_PIN_H();
               delay_0_25us();
                RGB_PIN_L();
               delay_us(1);
            }
        }
    }
    
    // ���͸�λ�ź�
     RGB_PIN_L();
    delay_us(RESET_DELAY);
    
    __enable_irq(); // ���������ж�
}

void set_led_color(uint8_t id,uint8_t red, uint8_t green, uint8_t blue)
{
    // WS2812ʹ��GRB˳��
    LedsArray[id*3] = green;
    LedsArray[id*3+1] = red;
    LedsArray[id*3+2] = blue;
}

// void breathing_effect(uint8_t r, uint8_t g, uint8_t b)
// {
//     uint16_t brightness;
    
//     // ����Ч�������ȴ�0��255�ٵ�0
//     for(brightness = 0; brightness < 256; brightness++) {
//         set_led_color(0,r * brightness / 256, 
//                      g * brightness / 256, 
//                      b * brightness / 256);
// 			  set_led_color(1,r * brightness / 256, 
//                      g * brightness / 256, 
//                      b * brightness / 256);
//         ws2812_send();
//         delay_ms(10);
//     }
    
//     for(brightness = 255; brightness > 0; brightness--) {
// 			        set_led_color(0,r * brightness / 256, 
//                      g * brightness / 256, 
//                      b * brightness / 256);
//         set_led_color(1,r * brightness / 256, 
//                      g * brightness / 256, 
//                      b * brightness / 256);
//         ws2812_send();
//         delay_ms(10);
//     }
// }


// void Ws2812b_WriteByte1(unsigned char byte)
// {
//     int i = 0, j = 0, k = 0;
//         for(i = 0; i < 8; i++ )
//         {
//             if( byte & (0x80 >> i) )//��ǰλΪ1
//             { 
//                 RGB_PIN_H();

//                 //0.75us
//                 delay_us(1);


//                 RGB_PIN_L();

//                 delay_0_25us(); //0.25us
//             }
//             else//��ǰλΪ0
//             {
//                 RGB_PIN_H();
                
//                 delay_0_25us(); //0.25us
//                 RGB_PIN_L();

//                 //0.833us
//                 delay_us(1);
//             }
//         }
// }
    
// void BSP_Loop(void)
// {

// 	static uint8_t send_key1 = 0;
// 	static uint8_t send_key2 = 0;
// 	static uint8_t send_key3 = 0;
// 	// ʹ�ð�������һ�¸ı�k230��RGB��ɫ
// 	// Using the button, press once to change the RGB color of k230
// 	if (Key1_State(1))
// 	{
// 		switch (send_key1)
// 		{
// 		case 1:
// 	 Control_RGB_ALL(Red_RGB);
//     delay_ms(10);
// 			break;
// 		case 0:
// 		 Control_RGB_ALL(OFF);
//     delay_ms(10);
// 		default:
// 			break;
		
// 		}
// 		send_key1 = (send_key1 + 1) % 2;

// 	}

// 	// ʹ�ð�������һ�¸ı�k230��RGB��ɫ
// 	// Using the button, press once to change the RGB color of k230
// 	if (Key2_State(1))
// 	{
// 				switch (send_key2)
// 		{
// 	case 1:
// 	 Control_RGB_ALL(Red_RGB);
//     delay_ms(10);
// 		Control_RGB_ALL(OFF);
// 		delay_ms(10);
// 		Control_RGB_ALL(Orange_RGB);
// 		delay_ms(10);
// 	Control_RGB_ALL(OFF);
// 		delay_ms(10);
// 	Control_RGB_ALL(Yellow_RGB);
// 	delay_ms(10);
// 	Control_RGB_ALL(OFF);
// 		delay_ms(10);
// 		Control_RGB_ALL(Green_RGB);
// 	delay_ms(10);
// 	Control_RGB_ALL(OFF);
// 		delay_ms(10);
	
// 			Control_RGB_ALL(Cyan_RGB);
// 		delay_ms(10);
// 			Control_RGB_ALL(OFF);
// 		delay_ms(10);
// 				Control_RGB_ALL(Blue_RGB);
// 		delay_ms(10);
// 			Control_RGB_ALL(OFF);
// 		delay_ms(10);
		
// 		Control_RGB_ALL(Purple_RGB);
// 	delay_ms(10);
// 	Control_RGB_ALL(OFF);

// 	delay_ms(10);

// 				break;
// 		case 0:
// 	 Control_RGB_ALL(Red_RGB);
//     delay_ms(10);
// 			break;
// 		default:
// 			break;
// 		}

		

// 	send_key2 = (send_key2 + 1) % 2;
// 	}
// 	if (Key3_State(1))
// 	{
// 		switch(send_key3)
// 			{
			
			
// 			case 1:
				
//  breathing_effect(255, 0, 0);
//         delay_ms(100);
// 			break;
// 			case 0:



// 			break;
// 	default:
// 			break;
			
// 		}
			
// 	send_key3 = (send_key3 + 1) % 2;
// 	}
// }
 