/*
 * ============================================================
 * inv_mpu_dmp_motion_driver.h — DMP 运动驱动头文件 (精简版)
 * ============================================================
 */
#ifndef _INV_MPU_DMP_MOTION_DRIVER_H_
#define _INV_MPU_DMP_MOTION_DRIVER_H_

/* DMP 功能掩码 */
#define DMP_FEATURE_LP_QUAT         (0x004)
#define DMP_FEATURE_PEDOMETER       (0x008)
#define DMP_FEATURE_6X_LP_QUAT      (0x010)
#define DMP_FEATURE_GYRO_CAL        (0x020)

#define INV_WXYZ_QUAT               (0x100)

/* DMP 核心函数 */
int dmp_load_motion_driver_firmware(void);
int dmp_set_fifo_rate(unsigned short rate);
int dmp_enable_feature(unsigned short mask);
int dmp_set_orientation(unsigned short orient);
int dmp_read_fifo(short *gyro, short *accel, long *quat,
    unsigned long *timestamp, short *sensors, unsigned char *more);

#endif  /* #ifndef _INV_MPU_DMP_MOTION_DRIVER_H_ */
