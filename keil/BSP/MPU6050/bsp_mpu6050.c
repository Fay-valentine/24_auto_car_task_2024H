#include "bsp_mpu6050.h"
#include "stdio.h"

void DMP_Init(void)
{
	uint8_t i=1,dmp_error;//记录dmp错误原因
	OLED_ShowString(0,9,"DMP ing...",8,1);
	OLED_ShowString(0,18,"Attempts:",8,1); //尝试次数
	OLED_ShowString(0,0,"Error:",8,1);    //错误原因
	OLED_Refresh();
	do//DMP初始化
	{
#if DEBUG
		printf("dmp_error: %d  \r\n",dmp_error);
		i++;
		printf("Attempts: %d   \r\n",i);
#endif
		dmp_error = mpu_dmp_init();
		OLED_ShowNum(60,18,i++,2,8,1);
		OLED_ShowNum(42,0,dmp_error,3,8,1);
		OLED_Refresh();
		
		delay_ms(200);
	}while(dmp_error);
	OLED_ShowString(60,10,"OK!",8,1);
	OLED_Refresh();
	delay_ms(1000);
	OLED_Clear(); //清屏
}


void MPU_Start(void)
{
    SDA_OUT();      // SDA设为输出
    SCL(1);         // ① 时钟拉高
    SDA(1);         // ② 数据线拉高（空闲状态）
    delay_us(5);
    SDA(0);         // ③ 在SCL高电平期间，SDA产生下降沿 → 起始条件
    delay_us(5);
    SCL(0);         // ④ 时钟拉低，准备传输数据
}

void MPU_Stop(void)
{
    SDA_OUT();
    SCL(0);         // ① 确保时钟为低
    SDA(0);         // ② 数据线拉低
    SCL(1);         // ③ 时钟拉高
    delay_us(5);
    SDA(1);         // ④ 在SCL高电平期间，SDA产生上升沿 → 停止条件
    delay_us(5);
}

void MPU_Send_Byte(uint8_t dat)
{
    int i;
    SDA_OUT();
    SCL(0);                         // ① 起始时时钟为低
    for(i = 0; i < 8; i++) {
        SDA((dat & 0x80) >> 7);     // ② 在SCL低时改变SDA（准备数据位）
        delay_us(1);
        SCL(1);                     // ③ 时钟上升沿 → 从机采样SDA
        delay_us(5);
        SCL(0);                     // ④ 时钟拉低，准备下一位
        delay_us(5);
        dat <<= 1;
    }
}

unsigned char Read_Byte(void)
{
    unsigned char i, receive = 0;
    SDA_IN();                       // 释放SDA，主机转为输入
    for(i = 0; i < 8; i++) {
        SCL(0);                     // ① 时钟拉低，从机准备数据
        delay_us(5);
        SCL(1);                     // ② 时钟上升沿，从机将数据放到SDA上
        delay_us(5);
        receive <<= 1;
        if(SDA_GET()) receive |= 1; // ③ 在SCL高电平期间读取SDA
        delay_us(5);
    }
    SCL(0);
    return receive;
}

unsigned char MPU6050_WaitAck(void)
{
    char ack = 0;
    unsigned char ack_flag = 10;
    SCL(0);                 // ① 时钟拉低，准备应答位
    SDA(1);                 // ② 释放SDA（主机输出高，等效于释放）
    SDA_IN();               // ③ 设为输入模式
    SCL(1);                 // ④ 时钟上升沿，从机在此时拉低SDA表示应答
    //最多等待50us，没有应答则停止数据运输
    while((SDA_GET() == 1) && (ack_flag--)) {
        delay_us(5);        // 等待从机拉低SDA
    }
    if(ack_flag <= 0) {
        MPU_Stop();
        return 1;
    }
    SCL(0);                 // ⑤ 应答位结束，时钟拉低
    SDA_OUT();
    return ack;
}
//ack=0,应答    ack=1，不应答
void MPU_Send_Ack(unsigned char ack)
{
    SDA_OUT();
    SCL(0);             // ① 时钟为低
    if(!ack) SDA(0);
    else     SDA(1);    // ② 在SCL低时设置应答电平
    delay_us(5);
    SCL(1);             // ③ 时钟上升沿，从机采样
    delay_us(5);
    SCL(0);             // ④ 结束
    SDA(1);
}

//reg：要读取的 MPU6050 寄存器地址
uint8_t MPU_Read_Byte(uint8_t reg)
{
    uint8_t res;
    MPU_Start();
    MPU_Send_Byte((MPU_ADDR<<1)|0);   // 设备地址 + 写标志
    MPU6050_WaitAck();
    MPU_Send_Byte(reg);               // 寄存器地址
    MPU6050_WaitAck();
    MPU_Start();                      // 重复起始
    MPU_Send_Byte((MPU_ADDR<<1)|1);   // 设备地址 + 读标志
    MPU6050_WaitAck();
    res = Read_Byte();                // 读取一个字节
    MPU_Stop();
    return res;
}
/**
 * @brief  向 MPU6050 的指定寄存器连续写入多个字节数据。
 * @details 该函数通过 I2C 总线执行一次连续写操作。MPU6050 支持地址自动递增，
 *          因此可以在一次事务中写入多个连续寄存器。
 * @param  addr     设备 I2C 地址（通常为 0x68 或 0x69，取决于 AD0 引脚电平）。
 * @param  regaddr  要写入的起始寄存器地址（0x00 ~ 0x7F）。
 * @param  num      要写入的字节数量。
 * @param  regdata  指向待写入数据缓冲区的指针，缓冲区长度至少为 num 字节。
 * @return 
 *   - `0`：所有数据成功写入。
 *   - `1`：发送设备地址（写模式）后从机无应答（可能设备未连接或地址错误）。
 *   - `2`：发送寄存器地址后从机无应答（可能寄存器地址无效或设备忙）。
 *   - `3 + i`：发送第 `i` 个数据字节（从 0 计数）后从机无应答。
 * @note   若发生错误，函数会在返回前调用 MPU_Stop() 释放 I2C 总线。
 */
char MPU6050_WriteReg(uint8_t addr, uint8_t regaddr, uint8_t num, uint8_t *regdata)
{
    uint16_t i;
    MPU_Start();
    MPU_Send_Byte((addr<<1)|0);       // 设备地址 + 写标志
    if(MPU6050_WaitAck() == 1) { MPU_Stop(); return 1; }
    MPU_Send_Byte(regaddr);           // 起始寄存器地址
    if(MPU6050_WaitAck() == 1) { MPU_Stop(); return 2; }
    for(i=0; i<num; i++) {
        MPU_Send_Byte(regdata[i]);    // 连续写入数据
        if(MPU6050_WaitAck() == 1) return (3+i);
    }
    MPU_Stop();
    return 0;
}
/**
 * @brief  从 MPU6050 的指定寄存器开始连续读取多个字节数据。
 * @details 该函数通过 I2C 总线执行一次连续读操作。首先发送写命令和寄存器地址，
 *          然后发送重复起始信号并切换到读模式，连续读取 num 个字节。对于前
 *          num-1 个字节，主机回应 ACK；对于最后一个字节，主机回应 NACK 以结束传输。
 * @param  addr     设备 I2C 地址（通常为 0x68 或 0x69，取决于 AD0 引脚电平）。
 * @param  regaddr  要读取的起始寄存器地址（0x00 ~ 0x7F）。
 * @param  num      要读取的字节数量。
 * @param  Read     指向存储读出数据缓冲区的指针，缓冲区长度至少为 num 字节。
 * @return 
 *   - `0`：所有数据成功读取。
 *   - `1`：发送设备地址（写模式）后从机无应答。
 *   - `2`：发送寄存器地址后从机无应答。
 *   - `3`：发送重复起始信号后的设备地址（读模式）无应答。
 * @note   若发生错误，函数会在返回前调用 MPU_Stop() 释放 I2C 总线。
 */
char MPU6050_ReadData(uint8_t addr, uint8_t regaddr, uint8_t num, uint8_t* Read)
{
    uint8_t i;
    MPU_Start();
    MPU_Send_Byte((addr<<1)|0);       // 设备地址 + 写
    if(MPU6050_WaitAck() == 1) { MPU_Stop(); return 1; }
    MPU_Send_Byte(regaddr);           // 起始寄存器地址
    if(MPU6050_WaitAck() == 1) { MPU_Stop(); return 2; }

    MPU_Start();                      // 重复起始
    MPU_Send_Byte((addr<<1)|1);       // 设备地址 + 读
    if(MPU6050_WaitAck() == 1) { MPU_Stop(); return 3; }

    for(i=0; i<(num-1); i++) {
        Read[i] = Read_Byte();        // 读取一个字节
        MPU_Send_Ack(0);              // 主机发送 ACK（0），表示继续读
    }
    Read[i] = Read_Byte();            // 读取最后一个字节
    MPU_Send_Ack(1);                  // 发送 NACK（1），表示读取结束
    MPU_Stop();
    return 0;
}

/**
 * @brief  设置 MPU6050 陀螺仪的满量程范围。
 * 
 * @details 向陀螺仪配置寄存器（0x1B）的 FS_SEL 位写入对应值。
 *          量程越大，测量范围越宽，但灵敏度越低。
 * 
 * @param  fsr  量程选择，可选值为：
 *              - 0：±250 dps
 *              - 1：±500 dps
 *              - 2：±1000 dps
 *              - 3：±2000 dps
 * 
 * @return 0 表示设置成功，非 0 表示 I2C 通信失败（错误码来自底层写函数）。
 */
uint8_t MPU_Set_Gyro_Fsr(uint8_t fsr)
{
    uint8_t data = fsr << 3;                                 // 修正：定义临时变量
    return MPU6050_WriteReg(MPU_ADDR, MPU_GYRO_CFG_REG, 1, &data);
}

/**
 * @brief  设置 MPU6050 加速度计的满量程范围。
 * 
 * @details 向加速度计配置寄存器（0x1C）的 AFS_SEL 位写入对应值。
 *          量程越大，测量范围越宽，但灵敏度越低。
 * 
 * @param  fsr  量程选择，可选值为：
 *              - 0：±2 g
 *              - 1：±4 g
 *              - 2：±8 g
 *              - 3：±16 g
 * 
 * @return 0 表示设置成功，非 0 表示 I2C 通信失败。
 */
uint8_t MPU_Set_Accel_Fsr(uint8_t fsr)
{
    uint8_t data = fsr << 3;                                 // 修正：定义临时变量
    return MPU6050_WriteReg(MPU_ADDR, MPU_ACCEL_CFG_REG, 1, &data);
}

/**
 * @brief  设置 MPU6050 的数字低通滤波器（DLPF）截止频率。
 * 
 * @details 向配置寄存器（0x1A）的 DLPF_CFG 位写入对应值。
 *          该滤波器同时作用于陀螺仪和加速度计的输出。
 * 
 * @param  lpf  期望的低通滤波截止频率（单位 Hz）。
 *              函数会自动向下取整到最接近的硬件支持频率（5、10、20、42、98、188 Hz）。
 * 
 * @return 0 表示设置成功，非 0 表示 I2C 通信失败。
 */
uint8_t MPU_Set_LPF(uint16_t lpf)
{
    uint8_t data = 0;
    if(lpf >= 188) data = 1;
    else if(lpf >= 98) data = 2;
    else if(lpf >= 42) data = 3;
    else if(lpf >= 20) data = 4;
    else if(lpf >= 10) data = 5;
    else data = 6; 
    return data = MPU6050_WriteReg(MPU_ADDR, MPU_CFG_REG, 1, &data);
}

/**
 * @brief  设置 MPU6050 的采样率，并自动配置匹配的低通滤波器。
 * 
 * @details 该函数计算并写入采样率分频寄存器（0x19），然后调用 MPU_Set_LPF()
 *          将数字低通滤波器的截止频率设为采样率的一半，以防止频率混叠。
 *          内部采样频率假定为 1 kHz。
 * 
 * @param  rate  期望的采样率（单位 Hz），允许范围 4 ~ 1000 Hz。
 *               若输入超出范围，函数会自动限幅。
 * 
 * @return 0 表示设置成功，非 0 表示 I2C 通信失败。
 */
uint8_t MPU_Set_Rate(uint16_t rate)
{
    uint8_t data;
    if(rate > 1000) rate = 1000;
    if(rate < 4)    rate = 4;
    data = 1000 / rate - 1;
    data = MPU6050_WriteReg(MPU_ADDR, MPU_SAMPLE_RATE_REG, 1, &data);
    return MPU_Set_LPF(rate / 2);
}

/**
 * @brief  读取 MPU6050 三轴陀螺仪的原始数据。
 * 
 * @details 从陀螺仪数据寄存器（起始地址 0x43）连续读取 6 字节，
 *          并组合为 3 个 16 位有符号整数。数据为大端格式（高字节在前）。
 * 
 * @param[out] gyroDatax  指向存储 X 轴原始数据的指针。
 * @param[out] gyroDatay  指向存储 Y 轴原始数据的指针。
 * @param[out] gyroDataz  指向存储 Z 轴原始数据的指针。
 * 
 * @return 0 表示读取成功，非 0 表示 I2C 通信失败。
 */
uint8_t MPU6050ReadGyro(short *gyroDatax, short *gyroDatay, short *gyroDataz)
{
    uint8_t buf[6];
    uint8_t reg;
    // 从寄存器 0x43 开始连续读 6 字节
    reg = MPU6050_ReadData(MPU_ADDR, MPU6050_GYRO_OUT, 6, buf);
    if (reg == 0) {
        *gyroDatax = (buf[0] << 8) | buf[1];   // X轴：高8位 + 低8位
        *gyroDatay = (buf[2] << 8) | buf[3];   // Y轴
        *gyroDataz = (buf[4] << 8) | buf[5];   // Z轴
    }
    return reg;
}

/**
 * @brief  读取 MPU6050 三轴加速度计的原始数据。
 * 
 * @details 从加速度计数据寄存器（起始地址 0x3B）连续读取 6 字节，
 *          并组合为 3 个 16 位有符号整数。数据为大端格式（高字节在前）。
 * 
 * @param[out] accData_x  指向存储 X 轴原始数据的指针。
 * @param[out] accData_y  指向存储 Y 轴原始数据的指针。
 * @param[out] accData_z  指向存储 Z 轴原始数据的指针。
 * 
 * @return 0 表示读取成功，非 0 表示 I2C 通信失败。
 */
uint8_t MPU6050ReadAcc(short *accData_x, short *accData_y, short *accData_z)
{
    uint8_t buf[6];
    uint8_t reg;
    // 从寄存器 0x3B 开始连续读 6 字节
    reg = MPU6050_ReadData(MPU_ADDR, MPU6050_ACC_OUT, 6, buf);
    if (reg == 0) {
        *accData_x = (buf[0] << 8) | buf[1];
        *accData_y = (buf[2] << 8) | buf[3];
        *accData_z = (buf[4] << 8) | buf[5];
    }
    return reg;
}

float MPU6050_GetTemp(void)
{
    short temp3;
    uint8_t buf[2];
    float Temperature = 0;
    MPU6050_ReadData(MPU_ADDR, MPU6050_RA_TEMP_OUT_H, 2, buf);
    temp3 = (buf[0] << 8) | buf[1];
    Temperature = ((double)temp3 / 340.0) + 36.53;
    return Temperature;
}

uint8_t MPU6050ReadID(void)
{
    unsigned char Re[2] = {0};
    printf("mpu=%d\r\n", MPU6050_ReadData(MPU_ADDR, 0X75, 1, Re));
    printf("Re %d\r\n", Re[0]);
    if (Re[0] != MPU_ADDR) {
        printf("Not Found MPU6050 Model");
        return 1;
    } else {
        printf("MPU6050 ID = %x\r\n", Re[0]);
        return 0;
    }
    return 0;
}

char MPU6050_Init(void)
{	
    uint8_t res;
    // 修正：定义临时变量用于传递寄存器值
    uint8_t reset_val = 0x80;
    uint8_t wake_val  = 0x00;
    uint8_t zero      = 0x00;
    uint8_t int_cfg   = 0x80;
    uint8_t pll_clk   = 0x01;
    uint8_t pwr_mgmt2 = 0x00;

    SDA_OUT();
    delay_ms(10);

    // 1. 复位 MPU6050
    MPU6050_WriteReg(0x68, MPU6050_RA_PWR_MGMT_1, 1, &reset_val);
    delay_ms(100);

    // 2. 唤醒设备，选择时钟源为 PLL（X轴陀螺参考）
    MPU6050_WriteReg(0x68, MPU6050_RA_PWR_MGMT_1, 1, &wake_val);

    // 3. 设置量程和采样率
    MPU_Set_Gyro_Fsr(3);    // 陀螺仪 ±2000dps
    MPU_Set_Accel_Fsr(0);   // 加速度计 ±2g
    MPU_Set_Rate(50);       // 采样率 50Hz，同时设置 LPF

    // 4. 关闭不需要的功能（中断、FIFO、I2C主模式）
    MPU6050_WriteReg(0x68, MPU_INT_EN_REG, 1, &zero);        // 关闭所有中断
    MPU6050_WriteReg(0x68, MPU_USER_CTRL_REG, 1, &zero);     // 关闭 I2C 主模式
    MPU6050_WriteReg(0x68, MPU_FIFO_EN_REG, 1, &zero);       // 关闭 FIFO
    MPU6050_WriteReg(0x68, MPU_INTBP_CFG_REG, 1, &int_cfg);  // INT 引脚低电平有效

    // 5. 读取 WHO_AM_I 检查设备是否存在
    res = MPU_Read_Byte(MPU6050_WHO_AM_I);
    if (MPU6050ReadID() == 0) { // 检查是否有 MPU6050（ID=0x68）
        // 设备存在，进一步配置
        MPU6050_WriteReg(0x68, MPU6050_RA_PWR_MGMT_1, 1, &pll_clk); // 设置时钟源
        MPU6050_WriteReg(0x68, MPU_PWR_MGMT2_REG, 1, &pwr_mgmt2);   // 加速度与陀螺仪都工作
        MPU_Set_Rate(50);
        return 0; // 注意：原代码此处返回 1，但通常成功应返回 0，可能是笔误
    } else {
        // 设备未找到，显示错误信息
        OLED_ShowString(0,0,"MPU6050 err:",8,1);
        OLED_ShowNum(72,0,res,2,8,1);
        OLED_Refresh();
        return 1;
    }
    OLED_ShowString(0,0,"MPU6050 OK!",8,1);
    delay_ms(200);
    OLED_Refresh();
    return 0;
}