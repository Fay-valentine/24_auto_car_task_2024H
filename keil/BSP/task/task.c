#include "task.h"
#include "AllHeader.h"

extern uint8_t Cur_Mode;//在main中定义的

volatile int Stop_Num=0;//在第几个点停车，全局变量，用于mode切换时进行赋值，在Black_Check()中使用

uint8_t point_count=1;//记录经过了几个点，有A,B,C,D四个点
uint8_t is_black=WHITE_LINE;//是否在黑线上
uint8_t is_black_count=0;//黑线计数，满三次则认为在黑线上
uint8_t cur_blackLine_flag=0;//当前状态是否为黑线，0:白线 1:黑线
uint8_t pre_blackLine_flag=0;//上次状态是否为黑线，0:白线 1:黑线


static uint32_t stateEnterTime=0;//记录上次 点状态 切换的时间戳

void try_point_change_Reset(void)
{
    stateEnterTime=0;
}

/***
 * @brief 使得某个状态至少运行n秒以上
 * @param seconds 运行的秒数
 * @return 1:状态切换成功 0:状态切换失败
 */
uint8_t try_point_change(uint8_t seconds)
{
    if(Get_Time()-stateEnterTime>=seconds*1000)//运行满n秒后，允许point_count切换为新状态
    {
        point_count++;
        stateEnterTime=Get_Time();//更新状态进入时间
        return 1;
    }
    return 0;
}

/**
 * @brief 点数为1，设置上一次在白线，即设置为A点状态
 * 
 */
void Black_Check_Reset(void)
{
    cur_blackLine_flag=0;//重置黑线标志位
    is_black_count=0;//重置黑线消抖计数器
    point_count=1;
    try_point_change_Reset();
}

/**
 * @brief 黑白线转换 亮RGB灯,并记录次数(point_count)，可根据次数停车
 * @param num 第几个点停下，初始为第一个点，即point_count=1，当num=point_count时停下
 */
void Black_Check(uint8_t num)
{
    //转弯过程中红外传感器会扫过黑线产生误边沿，此时禁止 point_count 切换，
    //避免与 turnByAngle 抢夺电机控制权
    if(turnByAngle_IsBusy())
    {
        return;
    }
    //保存旧的黑线标志状态，用于边沿检测
    pre_blackLine_flag = cur_blackLine_flag;

    //is_black：本次调用是否在黑线上
    is_black = LineCheck();
    //消抖处理：连续3次检测到黑线才确认
    if(is_black == BLACK_LINE)
    {
        if(is_black_count < 3)
        {
            is_black_count++;
        }
        else
        {
            cur_blackLine_flag = 1;
        }
    }
    else//检测到白线，逐步减少计数器
    {
        
        if(is_black_count > 0)
        {
            is_black_count--;
        }
        else
        {
            //确认离开黑线
            cur_blackLine_flag = 0;
        }
    }
    

    //边沿检测：白→黑（0→1）或 黑→白（1→0）时触发点切换
    if((pre_blackLine_flag == 0 && cur_blackLine_flag == 1) || 
       (pre_blackLine_flag == 1 && cur_blackLine_flag == 0))
    {
        
        if(try_point_change(3))//尝试点数切换
        {
            Set_RGB(true, Red_RGB);//切换成功，则RGB灯亮红
        }
        
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
            case Mode4:
                Mode4_Exit();
                break;
            default:
                break;
            }
        }
    }

}
uint8_t get_is_black(void)
{
    return cur_blackLine_flag;
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

#define TURN_SPEED           800

static uint8_t  turn_state = TURN_STATE_IDLE;
static float  turn_target_angle;//转向目标角
static int16_t  turn_spin_speed;//转向速度
static uint32_t turn_start_time;//转向开始时间


uint8_t turnByAngle_IsBusy(void)
{
    return (turn_state != TURN_STATE_IDLE);
}

void turnByAngle_Reset(void)
{
    turn_state = TURN_STATE_IDLE;
}

/**
 * @brief 掉头函数（非阻塞）,会设置yaw_pid的目标角度
 * @param direction 1: 顺时针右转掉头, -1: 逆时针左转掉头
 * @param angle 旋转的角度
 * @return 0: 正在转向中, 1: 转向完成
 */
uint8_t turnByAngle(struct YawPID_t *pid,int8_t direction, float angle)
{
    float diff;
    static float prev_diff=0.0f;       /* 上一帧角度差，用于穿越检测,初始值为0.0f，第一次不进行穿越检测 */
    switch (turn_state)
    {
    case TURN_STATE_IDLE://空闲状态
        Motion_Stop(STOP_BRAKE);//优先刹车,等会再加直行函数的重置

        /* 重置PID运行时状态（保留Kp/Ki/Kd/限幅/target/locked） */
        pid->p_out = 0.0f;
        pid->i_out = 0.0f;
        pid->d_out = 0.0f;
        pid->pid_out = 0.0f;
        pid->err = 0.0f;
        pid->prev_error = 0.0f;
        pid->integral = 0.0f;

        turn_target_angle = YawPID_GetTarget(pid) - (direction * angle);//计算目标角度
        Wrap_Process(&turn_target_angle);//环绕处理
        YawPID_SetTarget(pid, turn_target_angle);

        turn_spin_speed = TURN_SPEED * direction;//设置旋转速度
        turn_start_time = Get_Time();//记录转向开始时间
        prev_diff = 0.0f;           //重置穿越检测

        turn_state = TURN_STATE_STOP_WAIT;//进入等待状态
        return 0;

    case TURN_STATE_STOP_WAIT://等待状态，50ms
        if (Get_Time() - turn_start_time < 50)
        {
            return 0;
        }
        Motion_Car_Control(0, 0, turn_spin_speed);//设置旋转速度

        turn_state = TURN_STATE_SPINNING;
        return 0;

    case TURN_STATE_SPINNING://旋转状态
        {
            Yaw_Update();                // 主动刷新yaw，绕过20ms调度，减少滞后
            diff = Get_Yaw() - turn_target_angle;//计算角度差
            Wrap_Process(&diff);//环绕处理
            
            /* 比例减速：越靠近目标速度越慢，减少惯性过冲 */
            float abs_diff = fabs(diff);
            if (abs_diff < 60.0f)
            {
                int16_t slow_speed = (int16_t)(800 * (abs_diff / 60.0f));//线性减速
                if (slow_speed < 400)//旋转最低速度 
                {
                    slow_speed = 400;
                }
                Motion_Car_Control(0, 0, slow_speed * direction);
            }
            else
            {
                Motion_Car_Control(0, 0, turn_spin_speed);
            }
        
            /* 检测到达目标：1)进入5°窗口  2)穿越目标(避免因采样间隔跳过窗口) */
            if (fabs(diff) < 5.0f ||
                (prev_diff != 0.0f && prev_diff * diff <= 0.0f && fabs(diff) < 20.0f))
            {
                Motion_Stop(STOP_BRAKE);
                
                turn_state = TURN_STATE_IDLE;
                return 1;
            }
            prev_diff = diff;
            return 0;
        }
        

    default:
        turn_state = TURN_STATE_IDLE;
        return 1;
    }
}

/**
 * @brief 测试 turnByAngle 转弯函数
 *         KEY1: 启动转向  KEY2: 紧急停止  KEY3: 切换测试角度
 *         使用while循环调用即可
 */
void TurnByAngle_Test(void)
{
    static int turn_angle = 0;

    Global_Loop();

    uint8_t key = Key_Scan();

    /* 按键处理（仅触发，不驱动状态机） */
    if(key == KEY1 && !turnByAngle_IsBusy())
    {
        sampleYaw(&yaw_pid);
        turnByAngle(&yaw_pid, 1, turn_angle);
    }
    if(key == KEY2)
    {
        turnByAngle_Reset();
        YawPID_Reset(&yaw_pid);
        Motion_Stop(STOP_BRAKE);
    }
    if(key == KEY3)
    {
        turn_angle += 10;
        if(turn_angle > 180) 
        {
            turn_angle = -180;
        }
        OLED_ShowSNum_Grid(4, 16, turn_angle, 3, 1, 0, 1);
    }

    /* 持续驱动转向状态机 */
    if(turnByAngle_IsBusy())
    {
        if(turnByAngle(&yaw_pid, 1, turn_angle))
        {
            Motion_Stop(STOP_BRAKE);
        }
    }
}

void IRTracking_test(void)
{
    Global_Loop();
	static uint32_t last_time = 0;
	uint8_t key = Key_Scan();

	if(key == KEY1)//直走
	{
	    sampleYaw(&yaw_pid);        // 采样锁定目标航向角
	    while(1)
	    {
	        Global_Loop();
	        SCHEDULE(last_time, 20, IRTracking(&track_pid));
            
	        if(Key_Scan() == KEY3) // 再次按KEY3退出
	        {
                Motion_Stop(STOP_BRAKE);		//优先刹车
				IRTracking_ResetPID(&track_pid);	//重置循迹函数
				
	            break;
	        }
	    }
	}
    if(key == KEY2)//调整KP
    {
        track_pid.Kp+=5.0f;
        OLED_ShowNum_Grid(2,2,track_pid.Kp,3,1,0,1);
        if(track_pid.Kp>121.0f)
        {
            track_pid.Kp=30.0f;
        }
    }
    if(key == KEY3)//调整KI
    {
        track_pid.Ki+=0.1f;
        OLED_ShowNum_Grid(2,7,track_pid.Ki*10,3,1,0,1);
        if(track_pid.Ki>2.1f)
        {
            track_pid.Ki=0.0f;
        }
    }
    
}

void walkStraight_test(void)
{
    Global_Loop();
	static uint32_t last_time = 0;
	uint8_t key = Key_Scan();

    if(key == KEY1)//运行循迹
	{
		while(1)
		{
			Global_Loop();
			//20ms调用一次LineWalking
            SCHEDULE(last_time, 20, walkStraight_Yaw(&yaw_pid));
            if(Key_Scan() == KEY3) // 再次按KEY3退出
	        {
				Motion_Stop(STOP_BRAKE);		//优先刹车
				walkStraight_Yaw_Reset(&yaw_pid);	//重置直行函数
				YawPID_Unlock(&yaw_pid);//解锁朝向
	            break;
	        }
		}
	}
}

/**
 * @brief 测试 walkStraight_Yaw 直行函数
 *         KEY1: 启动直行  KEY2: 紧急停止  KEY3: 退出测试
 *         使用while循环调用即可
 * 
 */
void line_yaw_walk_test(void)
{
    Global_Loop();
	static uint32_t last_time = 0;
	uint8_t key = Key_Scan();

	if(key == KEY1)//直走
	{
	    sampleYaw(&yaw_pid);        // 采样锁定目标航向角
	    while(1)
	    {
	        Global_Loop();
	        
            SCHEDULE(last_time, 20, walkStraight_Yaw(&yaw_pid));
	        if(Key_Scan() == KEY3) // 再次按KEY3退出
	        {
				Motion_Stop(STOP_BRAKE);		//优先刹车
				walkStraight_Yaw_Reset(&yaw_pid);	//重置直行函数
				YawPID_Unlock(&yaw_pid);//解锁朝向
	            break;
	        }
	    }
	}
	if(key == KEY2)//运行循迹
	{
		while(1)
		{
			Global_Loop();
			//20ms调用一次LineWalking
            SCHEDULE(last_time, 20, IRTracking(&track_pid));
            if(Key_Scan() == KEY3) // 再次按KEY3退出
	        {
				Motion_Stop(STOP_BRAKE);		//优先刹车
				IRTracking_ResetPID(&track_pid);	//重置循迹函数
	            break;
	        }
		}
	}
	if(key == KEY3)
	{
		
	}
}

/**
 * @brief key2调整目标里程，key1启动，key3退出
 * 
 */
void odometry_test(void)
{
    Global_Loop();
    static uint32_t last_yaw_time = 0;
    static uint32_t last_odom_time = 0;
    static uint32_t last_oled_time = 0;
    uint8_t key = Key_Scan();

    odometry_t od=get_odometry_cm();
    static float target_od = 0.0f;

    if(key == KEY1)//启动
    {
        sampleYaw(&yaw_pid);// 采样锁定目标航向角
        while(1)
        {
            Global_Loop();
            SCHEDULE(last_odom_time, 20, od = get_odometry_cm());
            SCHEDULE(last_oled_time, 200,
                    {
                        OLED_ShowSNum_Grid(2,2,od.left_cm,3,1,0,1);
                        OLED_ShowSNum_Grid(2,8,od.right_cm,3,1,0,1);
                    });

            if(od.left_cm<target_od)
            {
                SCHEDULE(last_yaw_time, 20, walkStraight_Yaw(&yaw_pid));
            }
            else
            {
                Motion_Stop(STOP_BRAKE);
                walkStraight_Yaw_Reset(&yaw_pid);
                YawPID_Unlock(&yaw_pid);//解锁朝向
                reset_odometry();
                break;
            }
            if(Key_Scan() == KEY3) // 再次按KEY3退出
            {
                Motion_Stop(STOP_BRAKE);
                walkStraight_Yaw_Reset(&yaw_pid);
                YawPID_Unlock(&yaw_pid);//解锁朝向
                reset_odometry();
                break;
            }
        }
    }
    if(key==KEY2)//设定指定里程（cm）
    {
        target_od+=10.0f;//增加10cm里程
        OLED_ShowSNum_Grid(2,13,target_od,3,1,0,1);
        if(target_od>101.0f)//最多1米
        {
            target_od=0.0f;
        }
    }
}