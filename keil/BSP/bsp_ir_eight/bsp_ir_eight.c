#include "bsp_ir_eight.h"
#include "AllHeader.h"


#define RE_0_BUFF_LEN_MAX   200     //最大缓冲区长度

volatile uint8_t IR_Data_Number[IR_NUM];//存储红外传感器的数据 0 or 1
volatile uint8_t  IR_recv0_buff[RE_0_BUFF_LEN_MAX] = {0};
volatile uint16_t IR_recv0_length = 0;
volatile uint8_t  IR_recv0_complete_flag = 0;
volatile u8 oledbuf[20] = {0};  // OLED显示缓冲区 (OLED display buffer)
/** 
*@brief 红外传感器_串口初始化函数
*/
void IR_UART_Init(void)
{
	//清除中断请求标志
	NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
	//使能中断
	NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
}

/**
 * @brief 八路循迹模块数据处理函数
 * 
 */
void IR_DataAnalysis(void)
{
	char temp[60]={0};//用于存放一个完整帧的数据
	int head=0;//帧头索引指针
	int end=0;//帧尾索引指针,(end+1)的数值代表了完整帧的长度
	char* buff;//找到帧头后，直接从buff[head]开始找，高效率，且能防止误读数据

	//1.判断完成flag(判断是否完成中断接收)
	if(IR_recv0_complete_flag==0)//未完成接收
	{
		return;
	}

	//2.！！！！处理数据时一定要关中断，防止触发中断，中途修改数据
	__disable_irq();

	//3.开始找帧头和帧尾，得到完整的数据帧地址
	while(IR_recv0_buff[head]!='$'&& head<IR_recv0_length)
	{
		head++;
	}
	//超出长度处理：
	if(head>=IR_recv0_length)//说明这组数据无效，丢弃
	{
		IR_recv0_length=0;//长度置零
		IR_recv0_complete_flag=0;//flag置为0，未完成
		__enable_irq();//！！！重新开启中断
		return;
	}

	//找到帧头后，buff[head]开始找帧尾 #
	buff=(char*)&IR_recv0_buff[head];
	while(buff[end]!='#' && end<(IR_recv0_length-head))
	{
		end++;//注意end从0开始++，最终是buff[end]指向‘#’而不是IR_recv0_buff[end]
		//但是buff是没有移动的，一直指向IR_recv0_buff[head]
	}
	//超出长度处理：
	if(end>=(IR_recv0_length-head))//说明这组数据无效，丢弃
	{
		IR_recv0_length=0;//长度置零
		IR_recv0_complete_flag=0;//flag置为0，未完成
		__enable_irq();//！！！重新开启中断
		return;
	}

	//4.找到帧头帧尾后,将完整帧拷贝到temp临时字符数组进行数据处理，开启中断，不影响数据接收
	if(end+1<sizeof(temp))//!!!先确保temp可以装下数据和最后一位结束符\0
	{
		//再进行拷贝
		strncpy(temp,buff,end+1);//!!由于buff的数量=end+1，所以strncpy不会自动添加\0,需要手动添加
		temp[end+1]='\0';//手动添加\0
		if(temp[0]=='$'&&temp[1]=='D')//判断帧头是否有效(之前只使用head确认了'$',但并没有确认第二个'D')
		{
			//有效后开始进行数据处理
			//(1):strtok(temp,',');按，分割字符串为若干子串，并把，变为\0,实现分割效果
			//依次调用，依次分割一次，返回子串，下一次从上次结束的地方开始，无更多子串后返回NULL
			char* token;//token接收返回的子串
			token=strtok(temp,",");
			//(2)循环八次处理分割的子串
			int index=0;//每个红外探头的数据索引
			while(token!=NULL && index<IR_NUM)
			{
				//(3)如果子串正确地包含“X”，则是有效子串，获取数据
				//strstr():查找字符串函数，如果找到：返回指向 haystack 中第一次出现 needle 的起始位置的指针。
				//如果未找到：返回 NULL。
				if(strstr(token,"x")!=NULL)
				{
					//(4)寻找：，通过：找到探头的数据
					//strchr():如果找到：返回指向 str 中第一次出现 ch 的地址的指针。
					//如果未找到：返回 NULL。
					char* colon;
					colon=strchr(token,':');
					if(colon!=NULL)
					{
						IR_Data_Number[index++]=(colon[1]-'0');
					}
				}
				token=strtok(NULL,",");//继续下一次分割，并获取子串
			}
			//            // 显示解析结果
            // Display parsing result
            sprintf((char*)oledbuf, "%d%d%d%d%d%d%d%d",
                IR_Data_Number[0], IR_Data_Number[1],
                IR_Data_Number[2], IR_Data_Number[3],
                IR_Data_Number[4], IR_Data_Number[5],
                IR_Data_Number[6], IR_Data_Number[7]);
// //            OLED_Clear();
//             OLED_ShowString(0, 40, (uint8_t *)oledbuf, 8, 1);
//             OLED_Refresh();
		}
		
	}
	//获取完数据或数据无效后，重置状态
		IR_recv0_complete_flag=0;
		IR_recv0_length=0;
		memset((void*)IR_recv0_buff,0,RE_0_BUFF_LEN_MAX);//重置接收缓冲区数据，防御编程
		__enable_irq();//！！！重新开启中断
	
}

/** 
*@brief 接收八路红外循迹模块的数据，并做了帧尾判断与'\0'格式处理
*/
void UART_0_INST_IRQHandler(void)
{
	//DL_GPIO_togglePins(LED_PORT, LED_MCU_PIN);
	uint8_t ReceiveData=0;
	switch(DL_UART_getPendingInterrupt(UART_0_INST))
	{
		case DL_UART_IIDX_RX:
			ReceiveData=DL_UART_Main_receiveData(UART_0_INST);
			if(IR_recv0_length<RE_0_BUFF_LEN_MAX-1)//只存RE_0_BUFF_LEN_MAX-1个字节，最后一个要存 \0
			{
				IR_recv0_buff[IR_recv0_length++]=ReceiveData;
				IR_recv0_buff[IR_recv0_length]='\0';//添加结束符,下一次添加也在这里，确保没有接收到完整的数据也符合格式
				
			}
			else//超出长度
			{
				IR_recv0_length=0;//满了之后长度置0
                IR_recv0_complete_flag=0;//完成标志重置
			}
            
            if(ReceiveData=='#')//接收到 # 代表收到 帧尾
            {
                IR_recv0_complete_flag=1;//接收完成，可以开始解析数据
            }
			break;
		//是其他串口的中断则退出
		default:
			break;
	}
	
}

// volatile uint8_t  recv0_buff[RE_0_BUFF_LEN_MAX] = {0};
// volatile uint16_t recv0_length = 0;
// volatile uint8_t  recv0_flag = 0;

// void UART_0_INST_IRQHandler(void)
// {
// 	uint8_t ReceiveData=0;
// 	switch(DL_UART_getPendingInterrupt(UART_0_INST))
// 	{
// 		case DL_UART_IIDX_RX:
// 			ReceiveData=DL_UART_receiveData(UART_0_INST);
// 			if(recv0_length<RE_0_BUFF_LEN_MAX-1)//只存RE_0_BUFF_LEN_MAX-1个字节
// 			{
// 				recv0_buff[recv0_length++]=ReceiveData;
// 				//回传
// 				uart0_send_char(ReceiveData);
// 			}
// 			else
// 			{
// 				recv0_length=0;//满了之后长度置0
// 			}
// 		//是其他串口的中断则退出
// 		default:
// 			break;
// 	}
	
// }