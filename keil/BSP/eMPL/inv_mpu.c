/*
 * ============================================================
 * inv_mpu.c — MPU6050 底层驱动 (精简版)
 *
 * 只保留 mpu_dmp_init / mpu_dmp_get_data 调用链所需函数.
 * 已移除: MPU6500支持 / 磁力计 / 自检 / 运动中断 / 低功耗 /
 *         FIFO数据读取 / 寄存器dump / 等功能.
 *
 * 依赖说明:
 *   - bsp_mpu6050.h   (I2C 底层接口)
 *   - inv_mpu_dmp_motion_driver.h  (DMP 固件接口)
 *   - AllHeader.h     (提供 delay_ms 等平台函数)
 * ============================================================
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "bsp_mpu6050.h"
#include "AllHeader.h"

#define MPU6050
#define MOTION_DRIVER_TARGET_MSP430

/* ---- 平台映射 ---- */
#if defined MOTION_DRIVER_TARGET_MSP430
#define i2c_write   MPU6050_WriteReg
#define i2c_read    MPU6050_ReadData
#define get_ms      mget_ms
#define log_i       printf
#define log_e       printf
#define fabs        fabsf
#define min(a,b)    ((a<b)?a:b)
#endif

/* ============================================================
 * 硬件寄存器地址结构体 (仅 MPU6050)
 * ============================================================ */
struct gyro_reg_s {
    unsigned char who_am_i;
    unsigned char rate_div;
    unsigned char lpf;
    unsigned char prod_id;
    unsigned char user_ctrl;
    unsigned char fifo_en;
    unsigned char gyro_cfg;
    unsigned char accel_cfg;
    unsigned char motion_thr;
    unsigned char motion_dur;
    unsigned char fifo_count_h;
    unsigned char fifo_r_w;
    unsigned char raw_gyro;
    unsigned char raw_accel;
    unsigned char temp;
    unsigned char int_enable;
    unsigned char dmp_int_status;
    unsigned char int_status;
    unsigned char pwr_mgmt_1;
    unsigned char pwr_mgmt_2;
    unsigned char int_pin_cfg;
    unsigned char mem_r_w;
    unsigned char accel_offs;
    unsigned char i2c_mst;
    unsigned char bank_sel;
    unsigned char mem_start_addr;
    unsigned char prgm_start_h;
};

struct hw_s {
    unsigned char addr;
    unsigned short max_fifo;
    unsigned char num_reg;
    unsigned short temp_sens;
    short temp_offset;
    unsigned short bank_size;
};

struct chip_cfg_s {
    unsigned char gyro_fsr;
    unsigned char accel_fsr;
    unsigned char sensors;
    unsigned char lpf;
    unsigned char clk_src;
    unsigned short sample_rate;
    unsigned char fifo_enable;
    unsigned char int_enable;
    unsigned char bypass_mode;
    unsigned char accel_half;
    unsigned char lp_accel_mode;
    unsigned char int_motion_only;
    struct motion_int_cache_s {
        unsigned short gyro_fsr;
        unsigned char accel_fsr;
        unsigned short lpf;
        unsigned short sample_rate;
        unsigned char sensors_on;
        unsigned char fifo_sensors;
        unsigned char dmp_on;
    } cache;
    unsigned char active_low_int;
    unsigned char latched_int;
    unsigned char dmp_on;
    unsigned char dmp_loaded;
    unsigned short dmp_sample_rate;
};

struct gyro_state_s {
    const struct gyro_reg_s *reg;
    const struct hw_s *hw;
    struct chip_cfg_s chip_cfg;
};

/* ---- 滤波器配置 ---- */
enum lpf_e {
    INV_FILTER_256HZ_NOLPF2 = 0,
    INV_FILTER_188HZ,
    INV_FILTER_98HZ,
    INV_FILTER_42HZ,
    INV_FILTER_20HZ,
    INV_FILTER_10HZ,
    INV_FILTER_5HZ,
    INV_FILTER_2100HZ_NOLPF,
    NUM_FILTER
};

enum gyro_fsr_e {
    INV_FSR_250DPS = 0,
    INV_FSR_500DPS,
    INV_FSR_1000DPS,
    INV_FSR_2000DPS,
    NUM_GYRO_FSR
};

enum accel_fsr_e {
    INV_FSR_2G = 0,
    INV_FSR_4G,
    INV_FSR_8G,
    INV_FSR_16G,
    NUM_ACCEL_FSR
};

enum clock_sel_e {
    INV_CLK_INTERNAL = 0,
    INV_CLK_PLL,
    NUM_CLK
};

/* ---- 位定义 ---- */
#define BIT_FIFO_EN         (0x40)
#define BIT_DMP_EN          (0x80)
#define BIT_FIFO_RST        (0x04)
#define BIT_DMP_RST         (0x08)
#define BIT_FIFO_OVERFLOW   (0x10)
#define BIT_DATA_RDY_EN     (0x01)
#define BIT_DMP_INT_EN      (0x02)
#define BIT_MOT_INT_EN      (0x40)
#define BITS_FSR            (0x18)
#define BITS_LPF            (0x07)
#define BITS_HPF            (0x07)
#define BITS_CLK            (0x07)
#define BIT_RESET           (0x80)
#define BIT_SLEEP           (0x40)
#define BIT_SLAVE_EN        (0x80)
#define BIT_I2C_READ        (0x80)
#define BIT_AUX_IF_EN       (0x20)
#define BIT_ACTL            (0x80)
#define BIT_LATCH_EN        (0x20)
#define BIT_ANY_RD_CLR      (0x10)
#define BIT_BYPASS_EN       (0x02)
#define BIT_LPA_CYCLE       (0x20)
#define BIT_STBY_XA         (0x20)
#define BIT_STBY_YA         (0x10)
#define BIT_STBY_ZA         (0x08)
#define BIT_STBY_XG         (0x04)
#define BIT_STBY_YG         (0x02)
#define BIT_STBY_ZG         (0x01)
#define BIT_STBY_XYZA       (BIT_STBY_XA | BIT_STBY_YA | BIT_STBY_ZA)
#define BIT_STBY_XYZG       (BIT_STBY_XG | BIT_STBY_YG | BIT_STBY_ZG)

/* ============================================================
 * 全局设备状态 (仅 MPU6050)
 * ============================================================ */
static const struct gyro_reg_s reg = {
    0x75,   /* who_am_i       */
    0x19,   /* rate_div       */
    0x1A,   /* lpf            */
    0x0C,   /* prod_id        */
    0x6A,   /* user_ctrl      */
    0x23,   /* fifo_en        */
    0x1B,   /* gyro_cfg       */
    0x1C,   /* accel_cfg      */
    0x1F,   /* motion_thr     */
    0x20,   /* motion_dur     */
    0x72,   /* fifo_count_h   */
    0x74,   /* fifo_r_w       */
    0x43,   /* raw_gyro       */
    0x3B,   /* raw_accel      */
    0x41,   /* temp           */
    0x38,   /* int_enable     */
    0x39,   /* dmp_int_status */
    0x3A,   /* int_status     */
    0x6B,   /* pwr_mgmt_1     */
    0x6C,   /* pwr_mgmt_2     */
    0x37,   /* int_pin_cfg    */
    0x6F,   /* mem_r_w        */
    0x06,   /* accel_offs     */
    0x24,   /* i2c_mst        */
    0x6D,   /* bank_sel       */
    0x6E,   /* mem_start_addr */
    0x70    /* prgm_start_h   */
};

static const struct hw_s hw = {
    0x68,       /* addr           */
    1024,       /* max_fifo       */
    118,        /* num_reg        */
    340,        /* temp_sens      */
    -521,       /* temp_offset    */
    256         /* bank_size      */
};

static struct gyro_state_s st = {
    &reg,
    &hw,
    {0}
};

/* ============================================================
 * 内部函数声明
 * ============================================================ */
static int set_int_enable(unsigned char enable);

/* ============================================================
 * 中断控制
 * ============================================================ */

/**
 * @brief  使能/禁用数据就绪中断
 *         DMP 开启时用 DMP 中断, 否则用数据就绪中断
 */
static int set_int_enable(unsigned char enable)
{
    unsigned char tmp;

    if (st.chip_cfg.dmp_on) {
        if (enable)
            tmp = BIT_DMP_INT_EN;
        else
            tmp = 0x00;
        if (i2c_write(st.hw->addr, st.reg->int_enable, 1, &tmp))
            return -1;
        st.chip_cfg.int_enable = tmp;
    } else {
        if (!st.chip_cfg.sensors)
            return -1;
        if (enable && st.chip_cfg.int_enable)
            return 0;
        if (enable)
            tmp = BIT_DATA_RDY_EN;
        else
            tmp = 0x00;
        if (i2c_write(st.hw->addr, st.reg->int_enable, 1, &tmp))
            return -1;
        st.chip_cfg.int_enable = tmp;
    }
    return 0;
}

/* ============================================================
 * 硬件初始化
 * ============================================================ */

/**
 * @brief  初始化 MPU6050 硬件
 *         陀螺仪: +/-2000dps, 加速度计: +/-2g, DLPF: 42Hz,
 *         FIFO: 50Hz, 时钟源: Gyro PLL
 */
int mpu_init(void)
{
    unsigned char data[6], rev;

    /* 复位设备 */
    data[0] = BIT_RESET;
    if (i2c_write(st.hw->addr, st.reg->pwr_mgmt_1, 1, data))
        return -1;
    delay_ms(100);

    /* 唤醒芯片 */
    data[0] = 0x00;
    if (i2c_write(st.hw->addr, st.reg->pwr_mgmt_1, 1, data))
        return -1;

    /* 检查产品版本 (MPU6050) */
    if (i2c_read(st.hw->addr, st.reg->accel_offs, 6, data))
        return -1;
    rev = ((data[5] & 0x01) << 2) | ((data[3] & 0x01) << 1) |
        (data[1] & 0x01);

    if (rev) {
        if (rev == 1)
            st.chip_cfg.accel_half = 1;
        else if (rev == 2)
            st.chip_cfg.accel_half = 0;
        else {
            log_e("Unsupported software product rev %d.\n", rev);
            return -1;
        }
    } else {
        if (i2c_read(st.hw->addr, st.reg->prod_id, 1, data))
            return -1;
        rev = data[0] & 0x0F;
        if (!rev) {
            log_e("Product ID read as 0 indicates device may be incompatible.\n");
            return -1;
        } else if (rev == 4) {
            log_i("Half sensitivity part found.\n");
            st.chip_cfg.accel_half = 1;
        } else
            st.chip_cfg.accel_half = 0;
    }

    /* 设置默认值以确保不跳过 I2C 写入 */
    st.chip_cfg.sensors = 0xFF;
    st.chip_cfg.gyro_fsr = 0xFF;
    st.chip_cfg.accel_fsr = 0xFF;
    st.chip_cfg.lpf = 0xFF;
    st.chip_cfg.sample_rate = 0xFFFF;
    st.chip_cfg.fifo_enable = 0xFF;
    st.chip_cfg.bypass_mode = 0xFF;
    st.chip_cfg.clk_src = INV_CLK_PLL;
    st.chip_cfg.active_low_int = 1;
    st.chip_cfg.latched_int = 0;
    st.chip_cfg.int_motion_only = 0;
    st.chip_cfg.lp_accel_mode = 0;
    memset(&st.chip_cfg.cache, 0, sizeof(st.chip_cfg.cache));
    st.chip_cfg.dmp_on = 0;
    st.chip_cfg.dmp_loaded = 0;
    st.chip_cfg.dmp_sample_rate = 0;

    if (mpu_set_gyro_fsr(2000))
        return -1;
    if (mpu_set_accel_fsr(2))
        return -1;
    if (mpu_set_lpf(42))
        return -1;
    if (mpu_set_sample_rate(50))
        return -1;
    if (mpu_configure_fifo(0))
        return -1;

    if (mpu_set_bypass(0))
        return -1;

    mpu_set_sensors(0);
    return 0;
}

/* ============================================================
 * 传感器配置
 * ============================================================ */

/**
 * @brief  设置陀螺仪满量程范围
 * @param  fsr  250 / 500 / 1000 / 2000 (dps)
 */
int mpu_set_gyro_fsr(unsigned short fsr)
{
    unsigned char data;

    if (!(st.chip_cfg.sensors))
        return -1;

    switch (fsr) {
    case 250:  data = INV_FSR_250DPS << 3;  break;
    case 500:  data = INV_FSR_500DPS << 3;  break;
    case 1000: data = INV_FSR_1000DPS << 3; break;
    case 2000: data = INV_FSR_2000DPS << 3; break;
    default:   return -1;
    }

    if (st.chip_cfg.gyro_fsr == (data >> 3))
        return 0;
    if (i2c_write(st.hw->addr, st.reg->gyro_cfg, 1, &data))
        return -1;
    st.chip_cfg.gyro_fsr = data >> 3;
    return 0;
}

/**
 * @brief  设置加速度计满量程范围
 * @param  fsr  2 / 4 / 8 / 16 (g)
 */
int mpu_set_accel_fsr(unsigned char fsr)
{
    unsigned char data;

    if (!(st.chip_cfg.sensors))
        return -1;

    switch (fsr) {
    case 2:  data = INV_FSR_2G << 3;  break;
    case 4:  data = INV_FSR_4G << 3;  break;
    case 8:  data = INV_FSR_8G << 3;  break;
    case 16: data = INV_FSR_16G << 3; break;
    default: return -1;
    }

    if (st.chip_cfg.accel_fsr == (data >> 3))
        return 0;
    if (i2c_write(st.hw->addr, st.reg->accel_cfg, 1, &data))
        return -1;
    st.chip_cfg.accel_fsr = data >> 3;
    return 0;
}

/**
 * @brief  设置数字低通滤波器 (DLPF)
 * @param  lpf  5 / 10 / 20 / 42 / 98 / 188 (Hz)
 */
int mpu_set_lpf(unsigned short lpf)
{
    unsigned char data;

    if (!(st.chip_cfg.sensors))
        return -1;

    if (lpf >= 188)
        data = INV_FILTER_188HZ;
    else if (lpf >= 98)
        data = INV_FILTER_98HZ;
    else if (lpf >= 42)
        data = INV_FILTER_42HZ;
    else if (lpf >= 20)
        data = INV_FILTER_20HZ;
    else if (lpf >= 10)
        data = INV_FILTER_10HZ;
    else
        data = INV_FILTER_5HZ;

    if (st.chip_cfg.lpf == data)
        return 0;
    if (i2c_write(st.hw->addr, st.reg->lpf, 1, &data))
        return -1;
    st.chip_cfg.lpf = data;
    return 0;
}

/**
 * @brief  设置采样率
 * @param  rate  4 ~ 1000 Hz
 */
int mpu_set_sample_rate(unsigned short rate)
{
    unsigned char data;

    if (!(st.chip_cfg.sensors))
        return -1;

    if (st.chip_cfg.dmp_on)
        return -1;
    else {
        if (st.chip_cfg.lp_accel_mode) {
            if (rate && (rate <= 40)) {
                return 0;
            }
        }
        if (rate < 4)
            rate = 4;
        else if (rate > 1000)
            rate = 1000;

        data = 1000 / rate - 1;
        if (i2c_write(st.hw->addr, st.reg->rate_div, 1, &data))
            return -1;

        st.chip_cfg.sample_rate = 1000 / (1 + data);

        mpu_set_lpf(st.chip_cfg.sample_rate >> 1);
        return 0;
    }
}

/**
 * @brief  设置旁路模式 (直连辅助 I2C)
 */
int mpu_set_bypass(unsigned char bypass_on)
{
    unsigned char tmp;

    if (st.chip_cfg.bypass_mode == bypass_on)
        return 0;

    if (bypass_on) {
        if (i2c_read(st.hw->addr, st.reg->user_ctrl, 1, &tmp))
            return -1;
        tmp &= ~BIT_AUX_IF_EN;
        if (i2c_write(st.hw->addr, st.reg->user_ctrl, 1, &tmp))
            return -1;
        delay_ms(3);
        tmp = BIT_BYPASS_EN;
        if (st.chip_cfg.active_low_int)
            tmp |= BIT_ACTL;
        if (st.chip_cfg.latched_int)
            tmp |= BIT_LATCH_EN | BIT_ANY_RD_CLR;
        if (i2c_write(st.hw->addr, st.reg->int_pin_cfg, 1, &tmp))
            return -1;
    } else {
        if (i2c_read(st.hw->addr, st.reg->user_ctrl, 1, &tmp))
            return -1;
        tmp &= ~BIT_AUX_IF_EN;
        if (i2c_write(st.hw->addr, st.reg->user_ctrl, 1, &tmp))
            return -1;
        delay_ms(3);
        if (st.chip_cfg.active_low_int)
            tmp = BIT_ACTL;
        else
            tmp = 0;
        if (st.chip_cfg.latched_int)
            tmp |= BIT_LATCH_EN | BIT_ANY_RD_CLR;
        if (i2c_write(st.hw->addr, st.reg->int_pin_cfg, 1, &tmp))
            return -1;
    }
    st.chip_cfg.bypass_mode = bypass_on;
    return 0;
}

/**
 * @brief  开关特定传感器
 * @param  sensors  掩码 (INV_X_GYRO | INV_Y_GYRO | INV_Z_GYRO | INV_XYZ_ACCEL)
 */
int mpu_set_sensors(unsigned char sensors)
{
    unsigned char data;

    if (sensors & INV_XYZ_GYRO)
        data = INV_CLK_PLL;
    else if (sensors)
        data = 0;
    else
        data = BIT_SLEEP;
    if (i2c_write(st.hw->addr, st.reg->pwr_mgmt_1, 1, &data)) {
        st.chip_cfg.sensors = 0;
        return -1;
    }
    st.chip_cfg.clk_src = data & ~BIT_SLEEP;

    data = 0;
    if (!(sensors & INV_X_GYRO))
        data |= BIT_STBY_XG;
    if (!(sensors & INV_Y_GYRO))
        data |= BIT_STBY_YG;
    if (!(sensors & INV_Z_GYRO))
        data |= BIT_STBY_ZG;
    if (!(sensors & INV_XYZ_ACCEL))
        data |= BIT_STBY_XYZA;
    if (i2c_write(st.hw->addr, st.reg->pwr_mgmt_2, 1, &data)) {
        st.chip_cfg.sensors = 0;
        return -1;
    }

    st.chip_cfg.sensors = sensors;
    st.chip_cfg.lp_accel_mode = 0;
    delay_ms(50);
    return 0;
}

/**
 * @brief  配置 FIFO 传感器
 */
int mpu_configure_fifo(unsigned char sensors)
{
    unsigned char prev;
    int result = 0;

    if (st.chip_cfg.dmp_on)
        return 0;
    else {
        if (!(st.chip_cfg.sensors))
            return -1;
        prev = st.chip_cfg.fifo_enable;
        st.chip_cfg.fifo_enable = sensors & st.chip_cfg.sensors;
        if (st.chip_cfg.fifo_enable != sensors)
            result = -1;
        else
            result = 0;
        if (sensors || st.chip_cfg.lp_accel_mode)
            set_int_enable(1);
        else
            set_int_enable(0);
        if (sensors) {
            if (mpu_reset_fifo()) {
                st.chip_cfg.fifo_enable = prev;
                return -1;
            }
        }
    }

    return result;
}

/* ============================================================
 * FIFO 控制
 * ============================================================ */

/**
 * @brief  重置 FIFO 读写指针
 */
int mpu_reset_fifo(void)
{
    unsigned char data;

    if (!(st.chip_cfg.sensors))
        return -1;

    data = 0;
    if (i2c_write(st.hw->addr, st.reg->int_enable, 1, &data))
        return -1;
    if (i2c_write(st.hw->addr, st.reg->fifo_en, 1, &data))
        return -1;
    if (i2c_write(st.hw->addr, st.reg->user_ctrl, 1, &data))
        return -1;

    if (st.chip_cfg.dmp_on) {
        data = BIT_FIFO_RST | BIT_DMP_RST;
        if (i2c_write(st.hw->addr, st.reg->user_ctrl, 1, &data))
            return -1;
        delay_ms(50);
        data = BIT_DMP_EN | BIT_FIFO_EN;
        if (i2c_write(st.hw->addr, st.reg->user_ctrl, 1, &data))
            return -1;
        if (st.chip_cfg.int_enable)
            data = BIT_DMP_INT_EN;
        else
            data = 0;
        if (i2c_write(st.hw->addr, st.reg->int_enable, 1, &data))
            return -1;
        data = 0;
        if (i2c_write(st.hw->addr, st.reg->fifo_en, 1, &data))
            return -1;
    } else {
        data = BIT_FIFO_RST;
        if (i2c_write(st.hw->addr, st.reg->user_ctrl, 1, &data))
            return -1;
        if (st.chip_cfg.bypass_mode)
            data = BIT_FIFO_EN;
        else
            data = BIT_FIFO_EN;
        if (i2c_write(st.hw->addr, st.reg->user_ctrl, 1, &data))
            return -1;
        delay_ms(50);
        if (st.chip_cfg.int_enable)
            data = BIT_DATA_RDY_EN;
        else
            data = 0;
        if (i2c_write(st.hw->addr, st.reg->int_enable, 1, &data))
            return -1;
        if (i2c_write(st.hw->addr, st.reg->fifo_en, 1, &st.chip_cfg.fifo_enable))
            return -1;
    }
    return 0;
}

/**
 * @brief  从 DMP FIFO 读取原始数据流
 */
int mpu_read_fifo_stream(unsigned short length, unsigned char *data,
    unsigned char *more)
{
    unsigned char tmp[2];
    unsigned short fifo_count;

    if (!st.chip_cfg.dmp_on)
        return -1;
    if (!st.chip_cfg.sensors)
        return -1;

    if (i2c_read(st.hw->addr, st.reg->fifo_count_h, 2, tmp))
        return -1;
    fifo_count = (tmp[0] << 8) | tmp[1];
    if (fifo_count < length) {
        more[0] = 0;
        return -1;
    }
    if (fifo_count > (st.hw->max_fifo >> 1)) {
        if (i2c_read(st.hw->addr, st.reg->int_status, 1, tmp))
            return -1;
        if (tmp[0] & BIT_FIFO_OVERFLOW) {
            mpu_reset_fifo();
            return -2;
        }
    }

    if (i2c_read(st.hw->addr, st.reg->fifo_r_w, length, data))
        return -1;
    more[0] = fifo_count / length - 1;
    return 0;
}

/* ============================================================
 * DMP 内存 / 固件
 * ============================================================ */

/**
 * @brief  写入 DMP 内存
 */
int mpu_write_mem(unsigned short mem_addr, unsigned short length,
        unsigned char *data)
{
    unsigned char tmp[2];

    if (!data)
        return -1;
    if (!st.chip_cfg.sensors)
        return -1;

    tmp[0] = (unsigned char)(mem_addr >> 8);
    tmp[1] = (unsigned char)(mem_addr & 0xFF);

    if (tmp[1] + length > st.hw->bank_size)
        return -1;

    if (i2c_write(st.hw->addr, st.reg->bank_sel, 2, tmp))
        return -1;
    if (i2c_write(st.hw->addr, st.reg->mem_r_w, length, data))
        return -1;
    return 0;
}

/**
 * @brief  读取 DMP 内存
 */
int mpu_read_mem(unsigned short mem_addr, unsigned short length,
        unsigned char *data)
{
    unsigned char tmp[2];

    if (!data)
        return -1;
    if (!st.chip_cfg.sensors)
        return -1;

    tmp[0] = (unsigned char)(mem_addr >> 8);
    tmp[1] = (unsigned char)(mem_addr & 0xFF);

    if (tmp[1] + length > st.hw->bank_size)
        return -1;

    if (i2c_write(st.hw->addr, st.reg->bank_sel, 2, tmp))
        return -1;
    if (i2c_read(st.hw->addr, st.reg->mem_r_w, length, data))
        return -1;
    return 0;
}

/**
 * @brief  加载并校验 DMP 固件
 */
int mpu_load_firmware(unsigned short length, const unsigned char *firmware,
    unsigned short start_addr, unsigned short sample_rate)
{
    unsigned short ii;
    unsigned short this_write;
#define LOAD_CHUNK  (16)
    unsigned char cur[LOAD_CHUNK], tmp[2];

    if (st.chip_cfg.dmp_loaded)
        return -1;

    if (!firmware)
        return -1;
    for (ii = 0; ii < length; ii += this_write) {
        this_write = min(LOAD_CHUNK, length - ii);
        if (mpu_write_mem(ii, this_write, (unsigned char*)&firmware[ii]))
            return -1;
        if (mpu_read_mem(ii, this_write, cur))
            return -1;
        if (memcmp(firmware+ii, cur, this_write))
            return -2;
    }

    tmp[0] = start_addr >> 8;
    tmp[1] = start_addr & 0xFF;
    if (i2c_write(st.hw->addr, st.reg->prgm_start_h, 2, tmp))
        return -1;

    st.chip_cfg.dmp_loaded = 1;
    st.chip_cfg.dmp_sample_rate = sample_rate;
    return 0;
}

/**
 * @brief  启用/禁用 DMP
 */
int mpu_set_dmp_state(unsigned char enable)
{
    unsigned char tmp;
    if (st.chip_cfg.dmp_on == enable)
        return 0;

    if (enable) {
        if (!st.chip_cfg.dmp_loaded)
            return -1;
        set_int_enable(0);
        mpu_set_bypass(0);
        mpu_set_sample_rate(st.chip_cfg.dmp_sample_rate);
        tmp = 0;
        i2c_write(st.hw->addr, 0x23, 1, &tmp);
        st.chip_cfg.dmp_on = 1;
        set_int_enable(1);
        mpu_reset_fifo();
    } else {
        set_int_enable(0);
        tmp = st.chip_cfg.fifo_enable;
        i2c_write(st.hw->addr, 0x23, 1, &tmp);
        st.chip_cfg.dmp_on = 0;
        mpu_reset_fifo();
    }
    return 0;
}

/* ============================================================
 * 方向矩阵 & 时间戳
 * ============================================================ */

unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;
    return b;
}

unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx)
{
    unsigned short scalar;
    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;
    return scalar;
}

void mget_ms(unsigned long *time)
{
    /* 空函数 — 时间戳功能未使用, 可根据需要实现 */
}

/* ============================================================
 * 陀螺仪方向矩阵 (单位矩阵 = 默认方向)
 * ============================================================ */
static signed char gyro_orientation[9] = { 1, 0, 0,
                                            0, 1, 0,
                                            0, 0, 1};

/* ============================================================
 * Q30 格式转换常量
 * ============================================================ */
#define q30  1073741824.0f

/* ============================================================
 * 用户 API
 * ============================================================ */

/**
 * @brief  MPU6050 DMP 初始化
 * @return 0=成功, 其他=失败错误码
 */
u8 mpu_dmp_init(void)
{
    u8 res = 0;

    res = mpu_init();
    if (res == 0) {
        res = mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
        if (res) return 1;
        res = mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
        if (res) return 2;
        res = mpu_set_sample_rate(DEFAULT_MPU_HZ);
        if (res) return 3;
        res = dmp_load_motion_driver_firmware();
        if (res) return 4;
        res = dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));
        if (res) return 5;
        res = dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_GYRO_CAL);
        if (res) return 6;
        res = dmp_set_fifo_rate(DEFAULT_MPU_HZ);
        if (res) return 7;
        res = mpu_set_dmp_state(1);
        if (res) return 9;
    }

    return 0;
}

/**
 * @brief  获取 DMP 解算后的欧拉角
 * @param  pitch  俯仰角 (度), 范围 -90.0 ~ +90.0
 * @param  roll   横滚角 (度), 范围 -180.0 ~ +180.0
 * @param  yaw    航向角 (度), 范围 -180.0 ~ +180.0
 * @return 0=成功, 1=FIFO读取失败, 2=四元数无效
 */
u8 mpu_dmp_get_data(float *pitch, float *roll, float *yaw)
{
    float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
    unsigned long sensor_timestamp;
    short gyro[3], accel[3], sensors;
    unsigned char more;
    long quat[4];

    if (dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more))
        return 1;

    if (sensors & INV_WXYZ_QUAT) {
        q0 = quat[0] / q30;
        q1 = quat[1] / q30;
        q2 = quat[2] / q30;
        q3 = quat[3] / q30;

        *pitch = asin(-2 * q1 * q3 + 2 * q0 * q2) * 57.3;
        *roll  = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 57.3;
        *yaw   = atan2(2 * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 57.3;
    } else
        return 2;

    return 0;
}
