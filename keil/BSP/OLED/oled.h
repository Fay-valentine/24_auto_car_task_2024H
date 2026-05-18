#ifndef __OLED_H
#define __OLED_H 

#include "AllHeader.h"

#ifndef u8
#define u8 uint8_t
#endif

#ifndef u16
#define u16 uint16_t
#endif

#ifndef u32
#define u32 uint32_t
#endif

//-----------------OLED端口定义---------------- 

#define OLED_SCL_Clr() DL_GPIO_clearPins(OLED_PORT,OLED_SCL1_PIN)//SCL
#define OLED_SCL_Set() DL_GPIO_setPins(OLED_PORT,OLED_SCL1_PIN)

#define OLED_SDA_Clr() DL_GPIO_clearPins(OLED_PORT,OLED_SDA1_PIN)//DIN
#define OLED_SDA_Set() DL_GPIO_setPins(OLED_PORT,OLED_SDA1_PIN)


#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

void OLED_ClearPoint(u8 x,u8 y);
void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_WaitAck(void);
void Send_Byte(u8 dat);
void OLED_WR_Byte(u8 dat,u8 mode);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode);
void OLED_DrawCircle(u8 x,u8 y,u8 r);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1,u8 mode);
void OLED_ShowChar6x8(u8 x,u8 y,u8 chr,u8 mode);
void OLED_ShowString(u8 x,u8 y,const char *chr,u8 size1,u8 mode);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode);
void OLED_ShowSNum(uint8_t x,uint8_t y,int num,uint8_t len,uint8_t size1,uint8_t mode);
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1,u8 mode);
void OLED_ScrollDisplay(u8 num,u8 space,u8 mode);
void OLED_ShowPicture(u8 x,u8 y,u8 sizex,u8 sizey,u8 BMP[],u8 mode);
void OLED_Init(void);

void OLED_Draw_Line(char *data, uint8_t line, bool clear, bool refresh);

//行列显示函数
/* 行列网格显示封装（基于6x8字体） */
#define CHAR_WIDTH  6
#define LINE_HEIGHT 8
#define MAX_COL     (128 / CHAR_WIDTH)  // 21

void OLED_ShowChar_Grid(uint8_t line, uint8_t col, uint8_t chr, uint8_t mode, bool refresh);
void OLED_ShowString_Grid(uint8_t line, uint8_t col, const char *str, uint8_t mode, bool clear_line, bool refresh);
void OLED_ShowNum_Grid(uint8_t line, uint8_t col, uint32_t num, uint8_t len, uint8_t mode, bool clear_line, bool refresh);
void OLED_ShowSNum_Grid(uint8_t line, uint8_t col, int32_t num, uint8_t len, uint8_t mode, bool clear_line, bool refresh);
void OLED_ClearLine_Grid(uint8_t line, uint8_t mode, bool refresh);
void OLED_ShowString_Center(uint8_t line, const char *str, uint8_t mode, bool clear_line, bool refresh);
void OLED_ClearLine(u8 line);

#endif

