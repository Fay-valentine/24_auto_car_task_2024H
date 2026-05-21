#include "task.h"

extern uint8_t Cur_Mode;//在main中定义的

volatile int Stop_Num=0;//在第几个点停车，全局变量，用于mode切换时进行赋值，在Black_Check()中使用
uint8_t point_count=1;//记录经过了几个点，有A,B,C,D四个点
uint8_t last_line=WHITE_LINE;//一开始在白线上
uint8_t is_black=WHITE_LINE;//是否在黑线上
uint8_t is_black_count=0;//黑线计数，满三次则认为在黑线上
uint8_t is_black_flag=0;//是否在黑线上



/**
 * @brief 点数为1，设置上一次在白线，即设置为A点状态
 * 
 */
void Black_Check_Reset(void)
{
    is_black_flag=0;//重置黑线标志位
    point_count=1;
    last_line=WHITE_LINE;
}

/**
 * @brief 黑白线转换 亮RGB灯,并记录次数(point_count)，可根据次数停车
 * @param num 第几个点停下，初始为第一个点，即point_count=1，当num=point_count时停下
 */
void Black_Check(uint8_t num)
{
    //更新八路数据
    static uint8_t x1,x2,x3,x4,x5,x6,x7,x8;
	deal_IRdata(&x1,&x2,&x3,&x4,&x5,&x6,&x7,&x8);
    
    //is_black：是否在黑线上
    is_black = (x1==0 || x2==0 ||x3==0 ||x4==0 ||x5==0 ||x6==0 ||x7==0 ||x8==0 ) ? BLACK_LINE : WHITE_LINE;
    //消抖处理：连续3次检测到黑线才确认
    if(is_black == BLACK_LINE)
    {
        if(is_black_count < 3)
        {
            is_black_count++;
        }
        
        //连续3次检测到黑线，且之前不在黑线上，才计数
        if(is_black_count >= 3 && is_black_flag == 0)
        {
            is_black_flag = 1;
            Set_RGB(true, Red_RGB);
            point_count++;
            
            //检查是否达到停车点
            if(point_count >= num)
            {
                switch (Cur_Mode)
                {
                case Mode1:
                    Mode1_Exit();
                    break;
                case Mode2:
                    Mode2_Exit();
                    break;
                case Mode3:
                    Mode3_Exit();
                    break;
                default:
                    break;
                }
            }
        }
    }
    else
    {
        //检测到白线，逐步减少计数器
        if(is_black_count > 0)
        {
            is_black_count--;
        }
        else
        {
            //确认离开黑线
            is_black_flag = 0;
        }
    }
    
    last_line = is_black;//更新上一次的线状态

}
uint8_t get_is_black(void)
{
    return is_black_flag;
}

//任务数组进行黑线计数控制
// Task tasks[]=
// {
//     {10,0,Black_Check,2},
    
// };

// void Schedule_Run(void)
// {
//     uint32_t now=Get_Time();//获取当前时间
//     //for循环遍历执行任务
//     for(int i=0;i<sizeof(tasks)/sizeof(Task);i++)
//     {
//         if(now-tasks[i].last_call>=tasks[i].interval)//到达期望间隔时间
//         {
//             tasks[i].task_func(tasks[i].arg);//执行任务
//             tasks[i].last_call=now;//更新last_call（上次执行任务的时间）
            
//         }
//     }
// }

volatile int8_t turn_adjust=0;//转头后的调整角度
/**
 * @brief 掉头函数
 * @param direction 1: 顺时针右转掉头, -1: 逆时针左转掉头
 * @param angle 旋转的角度
 */
void turnByAngle(int8_t direction, float angle)
{
    // 停止当前运动
    Motion_Stop(STOP_BRAKE);
    delay_ms(50);

    // 旋转速度（绝对值，单位 mm/s，对应 Motion_Car_Control 的 V_z 参数）
    int16_t spin_speed = 800 * direction;   // 800 是速度值，可调整
    
    float target_angle=target_yaw-(direction*angle);
    if (target_angle > 180.0f)  
    {
        target_angle -= 360.0f;
    }
    if (target_angle < -180.0f) 
    {
        target_angle += 360.0f;
    }

    // 旋转
    Motion_Car_Control(0, 0, spin_speed);

    while(true)
    {
        mpu_dmp_get_data(&pitch,&roll,&yaw);               // 更新 yaw

        // 计算最短角度差
        float diff = yaw - target_angle;

        if (diff > 180.0f)  
        {
            diff -= 360.0f;
        }
        if (diff < -180.0f) 
        {
            diff += 360.0f;
        }

        if (fabs(diff) < 8.0f)            // 误差 < 8° 即停止
        {
            break;
        }
            
        delay_ms(3);                     // 避免空转
    }
    
    // 停止
    Motion_Stop(STOP_BRAKE);
    target_yaw = target_angle + turn_adjust;
    if (target_yaw > 180.0f)  
    {
        target_yaw -= 360.0f;
    }
    if (target_yaw < -180.0f) 
    {
        target_yaw += 360.0f;
    }
    OLED_ShowSNum_Grid(3,6,target_yaw,4,1,0,1);//更新显示一次target_yaw
    StraightLineWalk_IMU_Reset();         // 下次直行重新锁定
}