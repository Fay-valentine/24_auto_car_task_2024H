#include "task.h"
#include "AllHeader.h"

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
    is_black_count=0;//重置黑线消抖计数器
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

#define TURN_STATE_IDLE       0
#define TURN_STATE_STOP_WAIT  1
#define TURN_STATE_SPINNING   2

static uint8_t  turn_state = TURN_STATE_IDLE;
static float  turn_target_angle;
static int16_t  turn_spin_speed;
static uint32_t turn_start_time;

uint8_t turnByAngle_IsBusy(void)
{
    return (turn_state != TURN_STATE_IDLE);
}

void turnByAngle_Reset(void)
{
    turn_state = TURN_STATE_IDLE;
}

/**
 * @brief 掉头函数（非阻塞版本）
 * @param direction 1: 顺时针右转掉头, -1: 逆时针左转掉头
 * @param angle 旋转的角度
 * @return 0: 正在转向中, 1: 转向完成
 */
uint8_t turnByAngle(struct YawPID_t *pid,int8_t direction, float angle)
{
    float diff;
    switch (turn_state)
    {
    case TURN_STATE_IDLE://空闲状态
        Motion_Stop(STOP_BRAKE);//优先刹车,等会再加直行函数的重置

        turn_target_angle = YawPID_GetTarget(pid) - (direction * angle);//计算目标角度
        Wrap_Process(&turn_target_angle);//环绕处理

        turn_spin_speed = 800 * direction;//设置旋转速度
        turn_start_time = Get_Time();//记录转向开始时间

        turn_state = TURN_STATE_STOP_WAIT;
        return 0;

    case TURN_STATE_STOP_WAIT://等待状态，50ms
        if (Get_Time() - turn_start_time < 50)
        {
            return 0;
        }
        Motion_Car_Control(0, 0, turn_spin_speed);

        turn_state = TURN_STATE_SPINNING;
        return 0;

    case TURN_STATE_SPINNING://旋转状态
        diff = Get_Yaw() - turn_target_angle;//计算角度差
        Wrap_Process(&diff);//环绕处理

        if (fabs(diff) < 8.0f)
        {
            Motion_Stop(STOP_BRAKE);
            pid->target = turn_target_angle + turn_adjust;//设置目标角度
            Wrap_Process(&pid->target);
            OLED_ShowSNum_Grid(3, 6, pid->target, 4, 1, 0, 1);
        
            turn_state = TURN_STATE_IDLE;
            return 1;
        }
        return 0;

    default:
        turn_state = TURN_STATE_IDLE;
        return 1;
    }
}