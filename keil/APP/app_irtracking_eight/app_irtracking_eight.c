#include "app_irtracking_eight.h"
#include "AllHeader.h"   

// 权重数组 (静态常量, 仅本文件可见)
//把它想象为一条数轴，黑线需要处在-1和1两个点上，小车以这个目标进行循线
static const int8_t weights[8] = {-4, -3, -2, -1, 1, 2, 3, 4};

int g_IR_track_speed = TRACK_SPEED;

TrackPID_t track_pid;           // 循迹PID实例

static float Calculate_Weighted_Error(void);

/**
 * @brief 初始化循迹模块
 */
void IRTracking_Init(TrackPID_t *pid, int speed)
{
    // 设置速度
    g_IR_track_speed = speed;
    // 初始化PID (KP, KI, KD, 积分限幅, 输出限幅)
    TrackPID_Init(pid, PID_TRACK_KP, PID_TRACK_KI, PID_TRACK_KD,
                  PID_TRACK_INTEGRAL_LIMIT, PID_TRACK_OUTPUT_LIMIT);
    // 目标偏差为0 (希望黑线居中)
    TrackPID_SetTarget(pid, 0.0f);
}

/**
 * @brief 重置循迹PID
 */
void IRTracking_ResetPID(TrackPID_t *pid)
{
    TrackPID_Reset(pid);
}

/**
 * @brief 加权平均法计算黑线重心偏差 (连续值)
 * @return float 偏差值, 范围 [-4.0f,4.0f], 正数表示黑线偏右, 负数偏左
 */
static float Calculate_Weighted_Error(void)
{
    int16_t weighted_sum = 0;
    uint8_t active_count = 0;
    static float last_err = 0.0f;   // 记忆上次偏差, 用于丢失寻线
    
    // 遍历8路传感器 (IR_Data_Number[] 由串口中断更新)
    for (uint8_t i = 0; i < 8; i++)
    {
        if (IR_Data_Number[i] == 0)   // 检测到黑线
        {
            weighted_sum += weights[i];
            active_count++;
        }
    }

    if (active_count > 0)
    {
        // 有黑线: 计算重心偏差
        last_err = (float)weighted_sum / (float)active_count;
        return last_err;
    }
    else
    {
        // 全白 (丢失黑线): 根据上次偏差方向强制给一个较大偏差
        if (last_err > 0.0f)//上次在黑线右侧，向左寻找
        {
            return -4.0f;
        }
        else if (last_err < 0.0f)//上次在黑线左侧，向右寻找
        {
            return 4.0f;
        }
        else
        {
            return 0.0f;
        }
    }
}

/**
 * @brief 获取八路红外传感器数据
 */
void deal_IRdata(u8 *x1, u8 *x2, u8 *x3, u8 *x4, u8 *x5, u8 *x6, u8 *x7, u8 *x8)
{
    *x1 = IR_Data_Number[0];
    *x2 = IR_Data_Number[1];
    *x3 = IR_Data_Number[2];
    *x4 = IR_Data_Number[3];
    *x5 = IR_Data_Number[4];
    *x6 = IR_Data_Number[5];
    *x7 = IR_Data_Number[6];
    *x8 = IR_Data_Number[7];
}

/**
 * @brief 检测小车当前是否在黑线上
 */
uint8_t LineCheck(void)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (IR_Data_Number[i] == 0)//检测到任意黑线，返回IR_BLACK
        {
            return IR_BLACK;
        }
    }
    return IR_WHITE;
}

/**
 * @brief 主循迹函数 (需周期性调用)
 */
void LineWalking(TrackPID_t *pid)
{
    float err;
    float turn;

    // 1. 计算当前黑线位置偏差
    err = Calculate_Weighted_Error();

    // 2. PID计算输出 (误差直接传入, target=0)
    turn = TrackPID_Compute(pid, err);//turn与error同号

    // 3. 设置电机速度
    //error<0,说明小车在黑线右侧，需要左转，即Vz<0; error>0,说明小车在黑线左侧，需要右转，即Vz>0;
    //所以这里直接传入turn,即PID计算出的输出值    
    Motion_Car_Control(g_IR_track_speed, 0, (int16_t)turn);
}