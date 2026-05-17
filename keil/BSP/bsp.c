#include "bsp.h"

void board_Init(void)//使用串口打印初始化成功
{
    SYSCFG_DL_init();
	IR_UART_Init();//八路红外模块串口
}

void bsp_Init(void)
{
    board_Init();

    OLED_Init();//OLED
    delay_ms(500);

    Encoder_Init();//编码器
    Motors_Init();//车轮pid初始化
    Init_Motor_PWM();
    //定时器
    Timer_20ms_Init();
    Timer_1ms_Init();
    //MPU6050
	MPU6050_Init();
    DMP_Init();
    
    // ===== 预热 18 秒，等待温漂稳定 =====
    OLED_Clear();
    OLED_ShowString_Center(2, "wait stable", 1, true, true);
    delay_ms(18000);   // 也可留出余量，写成 18000
    OLED_Clear();
    //直行
    StraightLineWalk_IMU_Init();
}
