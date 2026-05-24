#ifndef _BSP_MPU6050_H_
#define _BSP_MPU6050_H_

#include "std_types.h"
#include "ti_msp_dl_config.h"

// 设置SDA为输出模式
//需要使用引脚复用，将引脚设置为数字输出模式
#define SDA_OUT()   {                                                  \
                        DL_GPIO_initDigitalOutput(MPU6050_SDA_IOMUX);     \
                        DL_GPIO_setPins(MPU6050_PORT, MPU6050_SDA_PIN);      \
                        DL_GPIO_enableOutput(MPU6050_PORT, MPU6050_SDA_PIN); \
                    }

// 设置SDA为输入模式
//配置为数字输入功能
#define SDA_IN()    { DL_GPIO_initDigitalInput(MPU6050_SDA_IOMUX); }

// 读取SDA引脚当前电平（返回1或0）
#define SDA_GET()   ( ( ( DL_GPIO_readPins(MPU6050_PORT,MPU6050_SDA_PIN) & MPU6050_SDA_PIN ) > 0 ) ? 1 : 0 )

// 控制SDA输出高低电平
#define SDA(x)      ( (x) ? (DL_GPIO_setPins(MPU6050_PORT,MPU6050_SDA_PIN)) : (DL_GPIO_clearPins(MPU6050_PORT,MPU6050_SDA_PIN)) )

// 控制SCL输出高低电平
#define SCL(x)      ( (x) ? (DL_GPIO_setPins(MPU6050_PORT,MPU6050_SCL_PIN)) : (DL_GPIO_clearPins(MPU6050_PORT,MPU6050_SCL_PIN)) )


//MPU6050的AD0是IIC地址引脚，接地则IIC地址为0x68,接VCC则IIC地址为0x69
//AD0 of MPU6050 is the IIC address pin. If it is grounded, the IIC address is 0x68. If it is connected to VCC, the IIC address is 0x69.


#define MPU6050_RA_SMPLRT_DIV       0x19        //陀螺仪采样率 地址  Gyro sampling rate address
#define MPU6050_RA_CONFIG           0x1A        //设置数字低通滤波器 地址 Set digital low-pass filter address
#define MPU6050_RA_GYRO_CONFIG      0x1B        //陀螺仪配置寄存器 Gyro configuration register
#define MPU6050_RA_ACCEL_CONFIG     0x1C        //加速度传感器配置寄存器 Acceleration sensor configuration register
#define MPU_INT_EN_REG              0X38        //中断使能寄存器 Interrupt enable register
#define MPU_USER_CTRL_REG           0X6A        //用户控制寄存器 User control register
#define MPU_FIFO_EN_REG             0X23        //FIFO使能寄存器 FIFO enable register
#define MPU_PWR_MGMT2_REG           0X6C        //电源管理寄存器2 Power management register 2
#define MPU_GYRO_CFG_REG            0X1B        //陀螺仪配置寄存器 Gyroscope configuration register
#define MPU_ACCEL_CFG_REG           0X1C        //加速度计配置寄存器 Accelerometer configuration register
#define MPU_CFG_REG                 0X1A        //配置寄存器 Configuration register
#define MPU_SAMPLE_RATE_REG         0X19        //采样频率分频器 Sampling frequency divider
#define MPU_INTBP_CFG_REG           0X37        //中断/旁路设置寄存器 Interrupt/bypass setting register

#define MPU6050_RA_PWR_MGMT_1       0x6B        //电源管理寄存器 1
#define MPU6050_RA_PWR_MGMT_2       0x6C        //电源管理寄存器 2

#define MPU6050_WHO_AM_I            0x75        //设备身份寄存器
#define MPU6050_SMPLRT_DIV          0            //8000Hz  
#define MPU6050_DLPF_CFG            0        
#define MPU6050_GYRO_OUT            0x43         //MPU6050陀螺仪数据寄存器地址 MPU6050 gyroscope data register address
#define MPU6050_ACC_OUT             0x3B         //MPU6050加速度数据寄存器地址 MPU6050 acceleration data register address
        
#define MPU6050_RA_TEMP_OUT_H       0x41        //温度高位 High temperature
#define MPU6050_RA_TEMP_OUT_L       0x42        //温度低位 Low temperature

#define MPU_ACCEL_XOUTH_REG         0X3B        //加速度值,X轴高8位寄存器 Acceleration value, X-axis high 8-bit register
#define MPU_ACCEL_XOUTL_REG         0X3C        //加速度值,X轴低8位寄存器 Acceleration value, X-axis low 8-bit register
#define MPU_ACCEL_YOUTH_REG         0X3D        //加速度值,Y轴高8位寄存器 Acceleration value, Y-axis high 8-bit register
#define MPU_ACCEL_YOUTL_REG         0X3E        //加速度值,Y轴低8位寄存器 Acceleration value, Y-axis low 8-bit register
#define MPU_ACCEL_ZOUTH_REG         0X3F        //加速度值,Z轴高8位寄存器 Acceleration value, Z-axis high 8-bit register
#define MPU_ACCEL_ZOUTL_REG         0X40        //加速度值,Z轴低8位寄存器 Acceleration value, Z-axis low 8-bit register

#define MPU_TEMP_OUTH_REG           0X41        //温度值高八位寄存器 Temperature value high eight bits register
#define MPU_TEMP_OUTL_REG           0X42        //温度值低8位寄存器 Temperature value lower 8 bits register

#define MPU_GYRO_XOUTH_REG          0X43        //陀螺仪值,X轴高8位寄存器 Gyroscope value, X-axis high 8-bit register
#define MPU_GYRO_XOUTL_REG          0X44        //陀螺仪值,X轴低8位寄存器 Gyroscope value, X-axis low 8-bit register
#define MPU_GYRO_YOUTH_REG          0X45        //陀螺仪值,Y轴高8位寄存器 Gyroscope value, Y-axis high 8-bit register
#define MPU_GYRO_YOUTL_REG          0X46        //陀螺仪值,Y轴低8位寄存器 Gyroscope value, Y-axis low 8-bit register
#define MPU_GYRO_ZOUTH_REG          0X47        //陀螺仪值,Z轴高8位寄存器 Gyroscope value, Z-axis high 8-bit register
#define MPU_GYRO_ZOUTL_REG          0X48        //陀螺仪值,Z轴低8位寄存器 Gyroscope value, Z-axis low 8-bit register

//如果AD0脚(9脚)接地,IIC地址为0X68(不包含最低位).
//如果接V3.3,则IIC地址为0X69(不包含最低位).
#define MPU_ADDR                    0x68

char MPU6050_Init(void);
void DMP_Init(void);

uint8_t MPU_Read_Byte(uint8_t reg);
char MPU6050_WriteReg(uint8_t addr,uint8_t regaddr,uint8_t num,uint8_t *regdata);
char MPU6050_ReadData(uint8_t addr, uint8_t regaddr,uint8_t num,uint8_t* Read);

uint8_t MPU6050ReadAcc(short *accData_x,short *accData_y,short *accData_z);
uint8_t MPU6050ReadGyro(short *gyroDatax,short *gyroDatay,short *gyroDataz);
float MPU6050_GetTemp(void);
uint8_t MPU6050ReadID(void);


#endif