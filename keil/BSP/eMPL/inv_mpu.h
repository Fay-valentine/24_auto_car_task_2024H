/*
 * ============================================================
 * inv_mpu.h — MPU6050 DMP 驱动头文件 (精简版)
 *
 * 只保留 mpu_dmp_init 和 mpu_dmp_get_data 所需声明。
 * ============================================================
 */
#ifndef _INV_MPU_H_
#define _INV_MPU_H_

#include <stdint.h>

/* 采样率 (Hz) */
#define DEFAULT_MPU_HZ  (100)

/* 传感器使能掩码 */
#define INV_X_GYRO      (0x40)
#define INV_Y_GYRO      (0x20)
#define INV_Z_GYRO      (0x10)
#define INV_XYZ_GYRO    (INV_X_GYRO | INV_Y_GYRO | INV_Z_GYRO)
#define INV_XYZ_ACCEL   (0x08)

/* 类型别名 */
#ifndef u8
#define u8 uint8_t
#endif
#ifndef u16
#define u16 uint16_t
#endif
#ifndef u32
#define u32 uint32_t
#endif

/* ============================================================
 * 内部函数声明 (DMP 运动驱动依赖)
 * ============================================================ */
int mpu_init(void);
int mpu_set_sensors(unsigned char sensors);
int mpu_configure_fifo(unsigned char sensors);
int mpu_set_sample_rate(unsigned short rate);
int mpu_set_gyro_fsr(unsigned short fsr);
int mpu_set_accel_fsr(unsigned char fsr);
int mpu_set_lpf(unsigned short lpf);
int mpu_set_bypass(unsigned char bypass_on);
int mpu_set_dmp_state(unsigned char enable);
int mpu_reset_fifo(void);
int mpu_write_mem(unsigned short mem_addr, unsigned short length, unsigned char *data);
int mpu_read_mem(unsigned short mem_addr, unsigned short length, unsigned char *data);
int mpu_load_firmware(unsigned short length, const unsigned char *firmware,
    unsigned short start_addr, unsigned short sample_rate);
int mpu_read_fifo_stream(unsigned short length, unsigned char *data, unsigned char *more);

unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx);
unsigned short inv_row_2_scale(const signed char *row);
void mget_ms(unsigned long *time);

/* ============================================================
 * 核心 API (用户直接使用)
 * ============================================================ */

/**
 * @brief  MPU6050 DMP 初始化
 * @return 0=成功, 非0=失败
 */
u8 mpu_dmp_init(void);

/**
 * @brief  获取 DMP 解算后的欧拉角
 * @param  pitch  俯仰角 (度), 范围 -90.0 ~ +90.0
 * @param  roll   横滚角 (度), 范围 -180.0 ~ +180.0
 * @param  yaw    航向角 (度), 范围 -180.0 ~ +180.0
 * @return 0=成功, 1=FIFO读取失败, 2=四元数数据无效
 */
u8 mpu_dmp_get_data(float *pitch, float *roll, float *yaw);

#endif  /* #ifndef _INV_MPU_H_ */
