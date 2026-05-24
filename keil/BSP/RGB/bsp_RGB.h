#ifndef _BSP_WS2812_H_
#define _BSP_WS2812_H_
 
#include "std_types.h"

#define WS2812_MAX        2   //�ʵ�������
#define WS2812_NUMBERS    2   //�ʵƸ���
#define RGB_COLOR_COUNT (sizeof(rgbColors)/sizeof(rgbColors[0]))

//�û��޸Ĳ�����
//#define WS2812_FREQUENCY
#define RGB_PIN_L()       DL_GPIO_clearPins(RGB_PORT, RGB_WQ2812_PIN)  //���Ʋʵ����ţ���Ҫ����Ϊǿ���������
#define RGB_PIN_H()       DL_GPIO_setPins(RGB_PORT, RGB_WQ2812_PIN)    //���Ʋʵ����ţ���Ҫ����Ϊǿ���������


typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB_Color;

extern unsigned int colorIndex;

#define RED               0xff0000                  //��ɫ
#define ORANGE              0xffa500                  //��ɫ
#define GREEN             0x00ff00                  //��ɫ
#define BLUE              0x0000ff                  //��ɫ
#define YELLOW            0xffff00                  //��ɫ
#define PURPLE            0xff00ff                  //��ɫ
#define CYAN              0x00ffff                  //��ɫ
#define BLACK             0x000000                  //Ϩ��
#define WHITE             0xffffff                  //��ɫ


static const unsigned long rgbColors[] = {
    RED,
	  ORANGE,
    GREEN,
    BLUE,
    YELLOW,
    PURPLE,
    CYAN,
    WHITE,
    BLACK
};




static const char *colorNames[] = {
    "Red",
		"Orange",
    "Green",
    "Blue",
    "Yellow",
    "Purple",
    "Cyan",
    "White",
    "Black"
};





void BSP_Loop(void);

//8.3 -8  0.000000083 
//4.16 -9 0.00000000416
void Ws2812b_WriteByte(unsigned char byte);//����һ���ֽ����ݣ�@12.000MHz,����ÿ����������83ns,����ԼΪ76ns��                                                      
void setLedCount(unsigned char count);//���òʵ���Ŀ����Χ0-25.                                                           
unsigned char getLedCount();//�ʵ���Ŀ��ѯ����                                                                     
void rgb_SetColor(unsigned char LedId, unsigned long color);//���òʵ���ɫ                                     
void rgb_SetRGB(unsigned char LedId, unsigned long red, unsigned long green, unsigned long blue);//���òʵ���ɫ
void rgb_SendArray();//���Ͳʵ�����   

void breathe_update(void);

void RGB_LED_Write1(void);
void RGB_LED_Reset(void );

void rgb_SendArray1(void);
void rgb_SetColor1(uint16_t n, uint32_t RGBcolor);
void rgb_GetRGB(unsigned char LedId, unsigned long i);

void delay_0_25us(void);

void breathing_effect(uint8_t r, uint8_t g, uint8_t b);
void set_led_color(uint8_t id,uint8_t red, uint8_t green, uint8_t blue);
void ws2812_send(void);


void GET_RGB(void);
#endif
