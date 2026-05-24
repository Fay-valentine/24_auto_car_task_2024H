#include "oledfont.h"  	 
#include "oled.h"
#include "AllHeader.h"

u8 OLED_GRAM[144][4];

//���Ժ���
void OLED_ColorTurn(u8 i)
{
	if(i==0)
		{
			OLED_WR_Byte(0xA6,OLED_CMD);//������ʾ
		}
	if(i==1)
		{
			OLED_WR_Byte(0xA7,OLED_CMD);//��ɫ��ʾ
		}
}

//��Ļ��ת180��
void OLED_DisplayTurn(u8 i)
{
	if(i==0)
		{
			OLED_WR_Byte(0xC8,OLED_CMD);//������ʾ
			OLED_WR_Byte(0xA1,OLED_CMD);
		}
	if(i==1)
		{
			OLED_WR_Byte(0xC0,OLED_CMD);//��ת��ʾ
			OLED_WR_Byte(0xA0,OLED_CMD);
		}
}

//��ʱ
void IIC_delay(void)
{
	delay_us(4);
}

//��ʼ�ź�
void I2C_Start(void)
{
	OLED_SDA_Set();
	OLED_SCL_Set();
	IIC_delay();
	OLED_SDA_Clr();
	IIC_delay();
	OLED_SCL_Clr();
	IIC_delay();
}

//�����ź�
void I2C_Stop(void)
{
	OLED_SDA_Clr();
	OLED_SCL_Set();
	IIC_delay();
	OLED_SDA_Set();
}

//�ȴ��ź���Ӧ
void I2C_WaitAck(void) //�������źŵĵ�ƽ
{
	OLED_SDA_Set();
	IIC_delay();
	OLED_SCL_Set();
	IIC_delay();
	OLED_SCL_Clr();
	IIC_delay();
}

//д��һ���ֽ�
void Send_Byte(u8 dat)
{
	u8 i;
	for(i=0;i<8;i++)
	{
		if(dat&0x80)//��dat��8λ�����λ����д��
		{
			OLED_SDA_Set();
    }
		else
		{
			OLED_SDA_Clr();
    }
		IIC_delay();
		OLED_SCL_Set();
		IIC_delay();
		OLED_SCL_Clr();//��ʱ���ź�����Ϊ�͵�ƽ
		dat<<=1;
  }
}

//����һ���ֽ�
//mode:����/�����־ 0,��ʾ����;1,��ʾ����;
void OLED_WR_Byte(u8 dat,u8 mode)
{
	I2C_Start();
	Send_Byte(0x78);
	I2C_WaitAck();
	if(mode){Send_Byte(0x40);}
  else{Send_Byte(0x00);}
	I2C_WaitAck();
	Send_Byte(dat);
	I2C_WaitAck();
	I2C_Stop();
}

//����OLED��ʾ 
void OLED_DisPlay_On(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//��ɱ�ʹ��
	OLED_WR_Byte(0x14,OLED_CMD);//������ɱ�
	OLED_WR_Byte(0xAF,OLED_CMD);//������Ļ
}

//�ر�OLED��ʾ 
void OLED_DisPlay_Off(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//��ɱ�ʹ��
	OLED_WR_Byte(0x10,OLED_CMD);//�رյ�ɱ�
	OLED_WR_Byte(0xAE,OLED_CMD);//�ر���Ļ
}

//�����Դ浽OLED	
void OLED_Refresh(void)
{
	u8 i,n;
	for(i=0;i<4;i++)
	{
		OLED_WR_Byte(0xb0+i,OLED_CMD); //��������ʼ��ַ
		OLED_WR_Byte(0x00,OLED_CMD);   //���õ�����ʼ��ַ
		OLED_WR_Byte(0x10,OLED_CMD);   //���ø�����ʼ��ַ
		I2C_Start();
		Send_Byte(0x78);
		I2C_WaitAck();
		Send_Byte(0x40);
		I2C_WaitAck();
		for(n=0;n<128;n++)
		{
			Send_Byte(OLED_GRAM[n][i]);
			I2C_WaitAck();
		}
		I2C_Stop();
  }
}
//��������
void OLED_Clear(void)
{
	u8 i,n;
	for(i=0;i<4;i++)
	{
	   for(n=0;n<128;n++)
			{
			 OLED_GRAM[n][i]=0;//�����������
			}
  }
	OLED_Refresh();//������ʾ
}

//���� 
//x:0~127
//y:0~63
//t:1 ��� 0,���	
void OLED_DrawPoint(u8 x,u8 y,u8 t)
{
	u8 i,m,n;
	i=y/8;
	m=y%8;
	n=1<<m;
	if(t){OLED_GRAM[x][i]|=n;}
	else
	{
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
		OLED_GRAM[x][i]|=n;
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
	}
}

//����
//x1,y1:�������
//x2,y2:��������
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1;
	uRow=x1;//�����������
	uCol=y1;
	if(delta_x>0)incx=1; //���õ������� 
	else if (delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//ˮƽ�� 
	else {incy=-1;delta_y=-delta_x;}
	if(delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		OLED_DrawPoint(uRow,uCol,mode);//����
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}
//x,y:Բ������
//r:Բ�İ뾶
void OLED_DrawCircle(u8 x,u8 y,u8 r)
{
	int a, b,num;
    a = 0;
    b = r;
    while(2 * b * b >= r * r)      
    {
        OLED_DrawPoint(x + a, y - b,1);
        OLED_DrawPoint(x - a, y - b,1);
        OLED_DrawPoint(x - a, y + b,1);
        OLED_DrawPoint(x + a, y + b,1);
 
        OLED_DrawPoint(x + b, y + a,1);
        OLED_DrawPoint(x + b, y - a,1);
        OLED_DrawPoint(x - b, y - a,1);
        OLED_DrawPoint(x - b, y + a,1);
        
        a++;
        num = (a * a + b * b) - r*r;//���㻭�ĵ���Բ�ĵľ���
        if(num > 0)
        {
            b--;
            a--;
        }
    }
}



//��ָ��λ����ʾһ���ַ�,���������ַ�
//x:0~127
//y:0~63
//size1:ѡ������ 6x8/6x12/8x16/12x24
//mode:0,��ɫ��ʾ;1,������ʾ
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1,u8 mode)
{
	u8 i,m,temp,size2,chr1;
	u8 x0=x,y0=y;
	if(size1==8)size2=6;
	else size2=(size1/8+((size1%8)?1:0))*(size1/2);  //�õ�����һ���ַ���Ӧ������ռ���ֽ���
	chr1=chr-' ';  //����ƫ�ƺ��ֵ
	for(i=0;i<size2;i++)
	{
		if(size1==8)
			  {temp=asc2_0806[chr1][i];} //����0806����
		else if(size1==12)
        {temp=asc2_1206[chr1][i];} //����1206����
		else if(size1==16)
        {temp=asc2_1608[chr1][i];} //����1608����
		else if(size1==24)
        {temp=asc2_2412[chr1][i];} //����2412����
		else return;
		for(m=0;m<8;m++)
		{
			if(temp&0x01)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp>>=1;
			y++;
		}
		x++;
		if((size1!=8)&&((x-x0)==size1/2))
		{x=x0;y0=y0+8;}
		y=y0;
  }
}


//��ʾ�ַ���
//x,y:�������  
//size1:�����С 
//*chr:�ַ�����ʼ��ַ 
//mode:0,��ɫ��ʾ;1,������ʾ
void OLED_ShowString(u8 x,u8 y,const char *chr,u8 size1,u8 mode)
{
	while((*chr>=' ')&&(*chr<='~'))//�ж��ǲ��ǷǷ��ַ�!
	{
		OLED_ShowChar(x,y,*chr,size1,mode);
		if(size1==8)x+=6;
		else x+=size1/2;
		chr++;
  }
}

//m^n
u32 OLED_Pow(u8 m,u8 n)
{
	u32 result=1;
	while(n--)
	{
	  result*=m;
	}
	return result;
}

//��ʾ����
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//mode:0,��ɫ��ʾ;1,������ʾ
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode)
{
	u8 t,temp,m=0;
	if(size1==8)m=2;
	for(t=0;t<len;t++)
	{
		temp=(num/OLED_Pow(10,len-t-1))%10;
			if(temp==0)
			{
				OLED_ShowChar(x+(size1/2+m)*t,y,'0',size1,mode);
      }
			else 
			{
			  OLED_ShowChar(x+(size1/2+m)*t,y,temp+'0',size1,mode);
			}
  }
}

//��ʾ���и���������
void OLED_ShowSNum(uint8_t x,uint8_t y,int num,uint8_t len,uint8_t size1,uint8_t mode)
{
	uint8_t t=0,temp,m=0;
	if(size1==8)m=2;
	if(num<0)
	{
		OLED_ShowChar(x+(size1/2+m)*t,y,'-',size1,mode);
		num = -num;
	}
	else
		OLED_ShowChar(x+(size1/2+m)*t,y,' ',size1,mode);
	for(t=1;t<len+1;t++)
	{
		temp=(num/OLED_Pow(10,len-t))%10;
			if(temp==0)
			{
				OLED_ShowChar(x+(size1/2+m)*t,y,'0',size1,mode);
			}
			else 
			{
			  OLED_ShowChar(x+(size1/2+m)*t,y,temp+'0',size1,mode);
			}
	}
}
//��ʾ����
//x,y:�������
//num:���ֶ�Ӧ�����
//mode:0,��ɫ��ʾ;1,������ʾ
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1,u8 mode)
{
	u8 m,temp;
	u8 x0=x,y0=y;
	u16 i,size3=(size1/8+((size1%8)?1:0))*size1;  //�õ�����һ���ַ���Ӧ������ռ���ֽ���
	for(i=0;i<size3;i++)
	{
		if(size1==16)
				{temp=Hzk1[num][i];}//����16*16����
		else if(size1==24)
				{temp=Hzk2[num][i];}//����24*24����
		else if(size1==32)       
				{temp=Hzk3[num][i];}//����32*32����
		else if(size1==64)
				{temp=Hzk4[num][i];}//����64*64����
		else return;
		for(m=0;m<8;m++)
		{
			if(temp&0x01)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp>>=1;
			y++;
		}
		x++;
		if((x-x0)==size1)
		{x=x0;y0=y0+8;}
		y=y0;
	}
}

//num ��ʾ���ֵĸ���
//space ÿһ����ʾ�ļ��
//mode:0,��ɫ��ʾ;1,������ʾ
void OLED_ScrollDisplay(u8 num,u8 space,u8 mode)
{
	u8 i,n,t=0,m=0,r;
	while(1)
	{
		if(m==0)
		{
	    OLED_ShowChinese(128,8,t,16,mode); //д��һ�����ֱ�����OLED_GRAM[][]������
			t++;
		}
		if(t==num)
			{
				for(r=0;r<16*space;r++)      //��ʾ���
				 {
					for(i=1;i<144;i++)
						{
							for(n=0;n<4;n++)
							{
								OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
							}
						}
           OLED_Refresh();
				 }
        t=0;
      }
		m++;
		if(m==16){m=0;}
		for(i=1;i<144;i++)   //ʵ������
		{
			for(n=0;n<4;n++)
			{
				OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
			}
		}
		OLED_Refresh();
	}
}

//x,y���������
//sizex,sizey,ͼƬ����
//BMP[]��Ҫд���ͼƬ����
//mode:0,��ɫ��ʾ;1,������ʾ
void OLED_ShowPicture(u8 x,u8 y,u8 sizex,u8 sizey,u8 BMP[],u8 mode)
{
	u16 j=0;
	u8 i,n,temp,m;
	u8 x0=x,y0=y;
	sizey=sizey/8+((sizey%8)?1:0);
	for(n=0;n<sizey;n++)
	{
		 for(i=0;i<sizex;i++)
		 {
				temp=BMP[j];
				j++;
				for(m=0;m<8;m++)
				{
					if(temp&0x01)OLED_DrawPoint(x,y,mode);
					else OLED_DrawPoint(x,y,!mode);
					temp>>=1;
					y++;
				}
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y0=y0+8;
				}
				y=y0;
     }
	 }
}
//OLED�ĳ�ʼ��
void OLED_Init(void)
{
     
//        OLED_RES_Clr();
        delay_ms(200);
//        OLED_RES_Set();
        
        OLED_WR_Byte(0xAE,OLED_CMD); /*display off*/
        OLED_WR_Byte(0x00,OLED_CMD); /*set lower column address*/ 
        OLED_WR_Byte(0x10,OLED_CMD); /*set higher column address*/
        OLED_WR_Byte(0x00,OLED_CMD); /*set display start line*/ 
        OLED_WR_Byte(0xB0,OLED_CMD); /*set page address*/ 
        OLED_WR_Byte(0x81,OLED_CMD); /*contract control*/ 
        OLED_WR_Byte(0xff,OLED_CMD); /*128*/ 
        OLED_WR_Byte(0xA1,OLED_CMD); /*set segment remap*/ 
        OLED_WR_Byte(0xA6,OLED_CMD); /*normal / reverse*/ 
        OLED_WR_Byte(0xA8,OLED_CMD); /*multiplex ratio*/ 
        OLED_WR_Byte(0x1F,OLED_CMD); /*duty = 1/32*/ 
        OLED_WR_Byte(0xC8,OLED_CMD); /*Com scan direction*/ 
        OLED_WR_Byte(0xD3,OLED_CMD); /*set display offset*/ 
        OLED_WR_Byte(0x00,OLED_CMD); 
        OLED_WR_Byte(0xD5,OLED_CMD); /*set osc division*/ 
        OLED_WR_Byte(0x80,OLED_CMD); 
        OLED_WR_Byte(0xD9,OLED_CMD); /*set pre-charge period*/ 
        OLED_WR_Byte(0x1f,OLED_CMD); 
        OLED_WR_Byte(0xDA,OLED_CMD); /*set COM pins*/ 
        OLED_WR_Byte(0x00,OLED_CMD); 
        OLED_WR_Byte(0xdb,OLED_CMD); /*set vcomh*/ 
        OLED_WR_Byte(0x40,OLED_CMD); 
        OLED_WR_Byte(0x8d,OLED_CMD); /*set charge pump enable*/ 
        OLED_WR_Byte(0x14,OLED_CMD);
        OLED_Clear();
        OLED_WR_Byte(0xAF,OLED_CMD); /*display ON*/
       
       OLED_Clear();
       OLED_Refresh();

       OLED_Draw_Line("OLED init success!",1,true,true);
	   delay_ms(200);
}


//����һ��������ʾ
/* д��һ���ַ� */
void OLED_Draw_Line(char *data, uint8_t line, bool clear, bool refresh)
{
	
		if (line > 0 && line <= 4)
		{
            if(clear == true)
                OLED_Clear();
            
            OLED_ShowString(0,(line-1)*8,data,8,1);
            
            if(refresh==true)
                OLED_Refresh();
		}
        
        
		
}

//����ѡ�����ʾ�����������꣩
// ============== ����������ʾ��װ��6x8���壩 ==============
#define CHAR_WIDTH  6   // �ַ�����(����)
#define LINE_HEIGHT 8   // �и�(����)
#define MAX_COL     (128 / CHAR_WIDTH)  // ������� = 21

/**
 * @brief ��ָ��������ʾһ���ַ�
 * @param line  �к� (1~4)
 * @param col   �к� (0~20)��������Χ����ʾ
 * @param chr   �ַ� (ASCII��)
 * @param mode  0=��ɫ��ʾ, 1=������ʾ
 * @param refresh  �Ƿ�����ˢ��
 */
void OLED_ShowChar_Grid(uint8_t line, uint8_t col, uint8_t chr, uint8_t mode, bool refresh)
{
    if (line < 1 || line > 4 || col >= MAX_COL) return;
    uint8_t x = col * CHAR_WIDTH;
    uint8_t y = (line - 1) * LINE_HEIGHT;
    OLED_ShowChar(x, y, chr, 8, mode);
    if (refresh) OLED_Refresh();
}

/**
 * @brief ��ָ�����п�ʼ��ʾ�ַ������Զ����У��������ֽضϣ�
 * @param line  �к� (1-4)
 * @param col   ��ʼ�к� (0-20)
 * @param str   �ַ��� (֧�ֿո�Ϳɼ�ASCII)
 * @param mode  ��ʾģʽ
 * @param clear_line  �Ƿ���������У��ӵ�0�е�ĩβ��
 * @param refresh  �Ƿ�����ˢ��
 */
void OLED_ShowString_Grid(uint8_t line, uint8_t col, const char *str, uint8_t mode, bool clear_line, bool refresh)
{
    if (line < 1 || line > 4 || col >= MAX_COL) return;
    
    uint8_t y = (line - 1) * LINE_HEIGHT;
    
    // ������У������Ҫ��
    if (clear_line) {
        for (uint8_t x = 0; x < 128; x++) {
            for (uint8_t dy = 0; dy < LINE_HEIGHT; dy++) {
                OLED_DrawPoint(x, y + dy, !mode);
            }
        }
    }
    
    // ���ַ���ʾ
    uint8_t current_col = col;
    while (*str && current_col < MAX_COL) {
        uint8_t x = current_col * CHAR_WIDTH;
        OLED_ShowChar(x, y, *str++, 8, mode);
        current_col++;
    }
    
    if (refresh) OLED_Refresh();
}

/**
 * @brief ��ָ��������ʾ�޷�������������λ����0��
 * @param line  �к� (1~4)
 * @param col   ��ʼ�к� (0~20)
 * @param num   Ҫ��ʾ������
 * @param len   ��ʾλ����1~10�������㲹ǰ��0
 * @param mode  ��ʾģʽ
 * @param clear_line  �Ƿ����������
 * @param refresh  �Ƿ�����ˢ��
 */
void OLED_ShowNum_Grid(uint8_t line, uint8_t col, uint32_t num, uint8_t len, uint8_t mode, bool clear_line, bool refresh)
{
    if (line < 1 || line > 4 || col >= MAX_COL) return;
    
    uint8_t y = (line - 1) * LINE_HEIGHT;
    
    if (clear_line) {
        for (uint8_t x = 0; x < 128; x++) {
            for (uint8_t dy = 0; dy < LINE_HEIGHT; dy++) {
                OLED_DrawPoint(x, y + dy, !mode);
            }
        }
    }
    
    // ������ʼx���꣨�����ڲ����Զ�������ƫ�ƣ�
    OLED_ShowNum(col * CHAR_WIDTH, y, num, len, 8, mode);
    if (refresh) OLED_Refresh();
}

/**
 * @brief ��ָ��������ʾ�з�������������ռһλ��λ�����㲹�ո�
 * @param line  �к� (1~4)
 * @param col   ��ʼ�к� (0~20)
 * @param num   �з�������
 * @param len   ����ʾλ���������ţ���ʵ�����ֲ��ְ�����ֵλ������
 * @param mode  ��ʾģʽ
 * @param clear_line  �Ƿ����������
 * @param refresh  �Ƿ�����ˢ��
 */
void OLED_ShowSNum_Grid(uint8_t line, uint8_t col, int32_t num, uint8_t len, uint8_t mode, bool clear_line, bool refresh)
{
    if (line < 1 || line > 4 || col >= MAX_COL) return;
    
    uint8_t y = (line - 1) * LINE_HEIGHT;
    
    if (clear_line) {
        for (uint8_t x = 0; x < 128; x++) {
            for (uint8_t dy = 0; dy < LINE_HEIGHT; dy++) {
                OLED_DrawPoint(x, y + dy, !mode);
            }
        }
    }
    
    OLED_ShowSNum(col * CHAR_WIDTH, y, num, len, 8, mode);
    if (refresh) OLED_Refresh();
}

/**
 * @brief ���ָ���У���䱳��ɫ��
 * @param line  �к� (1~4)
 * @param mode  0=ȫ��(��ɫ), 1=ȫ��(����)
 * @param refresh  �Ƿ�����ˢ��
 */
void OLED_ClearLine_Grid(uint8_t line, uint8_t mode, bool refresh)
{
    if (line < 1 || line > 4) return;
    uint8_t y = (line - 1) * LINE_HEIGHT;
    for (uint8_t x = 0; x < 128; x++) {
        for (uint8_t dy = 0; dy < LINE_HEIGHT; dy++) {
            OLED_DrawPoint(x, y + dy, !mode);
        }
    }
    if (refresh) OLED_Refresh();
}

/**
 * @brief ��ָ���о�����ʾ�ַ���
 * @param line  �к� (1~4)
 * @param str   �ַ���
 * @param mode  ��ʾģʽ
 * @param clear_line  �Ƿ����������
 * @param refresh  �Ƿ�ˢ��
 */
void OLED_ShowString_Center(uint8_t line, const char *str, uint8_t mode, bool clear_line, bool refresh)
{
    if (line < 1 || line > 4) return;
    uint8_t len = strlen(str);
    if (len > MAX_COL) len = MAX_COL;
    uint8_t start_col = (MAX_COL - len) / 2;
    OLED_ShowString_Grid(line, start_col, str, mode, clear_line, refresh);
}

/**
 * @brief ���ָ���У�ҳ������������
 * @param line �к� (1~4)����Ӧ OLED ��ҳ 0~3
 * @note ������Զ�ˢ���Դ�
 */
void OLED_ClearLine(u8 line)
{
    u8 page, col;
    
    // ������飺�кŷ�Χ 1~4
    if (line < 1 || line > 4) {
        return;
    }
    
    page = line - 1;   // ת��Ϊҳ���� (0~3)
    
    // ����ҳ�������У�0~127������
    for (col = 0; col < 128; col++) {
        OLED_GRAM[col][page] = 0;
    }
    
    // ˢ����ʾ
    OLED_Refresh();
}
