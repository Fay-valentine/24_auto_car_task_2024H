#include "bsp_RGB.h"

#define LED_NUM 1          // LED数量
#define DELAY_0H 3         // 0码高电平时间(us)
#define DELAY_0L 9         // 0码低电平时间(us)
#define DELAY_1H 6         // 1码高电平时间(us)
#define DELAY_1L 6         // 1码低电平时间(us)
#define RESET_DELAY 50     // 复位时间(us)

static uint8_t g_breathe_cnt = 0;   // 0..255
unsigned char LedsArray[WS2812_MAX * 3];      //定义颜色数据存储数组
unsigned char LedsArray1[255];      //定义颜色数据存储数组
unsigned int  ledsCount   = WS2812_NUMBERS;   //定义实际彩灯默认个数
unsigned int  nbLedsBytes = WS2812_NUMBERS*3; //定义实际彩灯颜色数据个数

unsigned int  nbLedsBytes1 = 255; //定义实际彩灯颜色数据个数
unsigned int colorIndex = 0;

// 延时0.25us
void delay_0_25us(void)
{
    //volatile
    for( int i = 0; i < 13; i++); //5:32MHZ  13：80Mhz
}

/******************************************************************
 * 函 数 名 称：rgb_SetColor
 * 函 数 说 明：设置彩灯颜色
 * 函 数 形 参：LedId控制的第几个灯  color颜色数据
 * 函 数 返 回：无
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
 * 函 数 名 称：rgb_SetRGB
 * 函 数 说 明：设置彩灯颜色(三原色设置)
 * 函 数 形 参：LedId控制的第几个灯 red红色数据  green绿色数据  blue蓝色数据
 * 函 数 返 回：无
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
 * 函 数 名 称：rgb_SendArray
 * 函 数 说 明：发送彩灯数据
 * 函 数 形 参：无
 * 函 数 返 回：无
******************************************************************/
void rgb_SendArray(void)
{
    unsigned int i;
    //发送数据
    for(i=0; i<nbLedsBytes; i++)
        Ws2812b_WriteByte(LedsArray[i]);
}
/******************************************************************
 * 函 数 名 称：rgb_SendArray
 * 函 数 说 明：发送彩灯数据
 * 函 数 形 参：无
 * 函 数 返 回：无
******************************************************************/
void rgb_SendArray1(void)
{
    unsigned int i;
    //发送数据
    for(i=0; i<nbLedsBytes1; i++)
        Ws2812b_WriteByte(LedsArray1[i]);
}
/******************************************************************
 * 函 数 名 称：RGB_LED_Reset
 * 函 数 说 明：复位ws2812
 * 函 数 形 参：无
 * 函 数 返 回：无
******************************************************************/
void RGB_LED_Reset(void)
{
        RGB_PIN_L();              
        delay_us(285);
}

/******************************************************************
 * 函 数 名 称：Ws2812b_WriteByte
 * 函 数 说 明：向WS2812写入单字节数据
 * 函 数 形 参：byte写入的字节数据
 * 函 数 返 回：无
 * 备       注：1码的时序 = 高电平580ns~1us    再低电平220ns~420ns
 *              0码的时序 = 高电平220ns~380ns  再低电平580ns~1us
******************************************************************/
void Ws2812b_WriteByte(unsigned char byte)
{
    int i = 0, j = 0, k = 0;
        for(i = 0; i < 8; i++ )
        {
            if( byte & (0x80 >> i) )//当前位为1
            { 
                RGB_PIN_H();

                //0.75us
                delay_us(1);


                RGB_PIN_L();

                delay_0_25us(); //0.25us
            }
            else//当前位为0
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
    __disable_irq(); // 关闭中断确保时序准确
    
    for(int i = 0; i < WS2812_MAX*3 ; i++) {
        uint8_t byte_val = LedsArray[i];
        
        for(int bit = 7; bit >= 0; bit--) {
            if(byte_val & (1 << bit)) {
                // 发送'1'码
                RGB_PIN_H();
                delay_us(1);
								RGB_PIN_L()  ;
                delay_0_25us();
            } else {
                // 发送'0'码
                 RGB_PIN_H();
               delay_0_25us();
                RGB_PIN_L();
               delay_us(1);
            }
        }
    }
    
    // 发送复位信号
     RGB_PIN_L();
    delay_us(RESET_DELAY);
    
    __enable_irq(); // 重新启用中断
}

void set_led_color(uint8_t id,uint8_t red, uint8_t green, uint8_t blue)
{
    // WS2812使用GRB顺序
    LedsArray[id*3] = green;
    LedsArray[id*3+1] = red;
    LedsArray[id*3+2] = blue;
}

// void breathing_effect(uint8_t r, uint8_t g, uint8_t b)
// {
//     uint16_t brightness;
    
//     // 呼吸效果：亮度从0到255再到0
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
//             if( byte & (0x80 >> i) )//当前位为1
//             { 
//                 RGB_PIN_H();

//                 //0.75us
//                 delay_us(1);


//                 RGB_PIN_L();

//                 delay_0_25us(); //0.25us
//             }
//             else//当前位为0
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
// 	// 使用按键，按一下改变k230的RGB颜色
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

// 	// 使用按键，按一下改变k230的RGB颜色
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
 