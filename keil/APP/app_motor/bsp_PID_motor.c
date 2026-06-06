#include "bsp_PID_motor.h"
#include "AllHeader.h"



PID_TypeDef  veer_pid;
int8_t veer = 0;

static int get_offset_yaw = 0;

volatile float l_pid_out;
volatile float r_pid_out;

volatile float kal_L_out;
volatile float kal_R_out;
PID_t pid_motor[2];


volatile	float Yawpid_out ;
//	float SetPoint ;
//volatile	float err;
volatile	float now_yawout;
//volatile	int16_t target_yaw;

//extern volatile float pitch,roll,yaw; 

extern volatile float my_yaw;



float get_yaw = 0;                    //The target heading_Angle of the car movement(车辆运动的目标航向角)

float first_yaw = 0;
// YAW偏航角
//YAW yaw angle
PID pid_Yaw = {0, 0.4, 0, 0.1, 0, 0, 0};

void PID_param_init(PID_TypeDef *pid)
{
	pid->Target = 0;//目标值
	pid->PID_out = 0;
    pid->Kp = 0;
    pid->Ki= 0;
	pid->Kd = 0;
    pid->Err = 0.0f;
    pid->LastErr = 0.0f;
	pid->PenultErr = 0.0f;
    pid->Integral = 0.0f;//积分值
	
	pid->KP_polarity = 1;
	pid->KI_polarity = 1;
	pid->KD_polarity = 1;
}




/**
 * @brief       pid闭环控制计算
 * @param       *PID：PID结构体变量地址
 * @param       CurrentValue：当前测量值
 * @retval      期望输出值
 */
float PID_Calculate(PID_TypeDef *PID,float CurrentValue)
{
	PID->Integral += PID->Err;
	/*积分分离*/
//	if( (PID->Err > 36) || (PID->Err < -36) ){
//		PID->Integral = 0;
//		//PID->PID_out += PID->IntegralConstant * PID->Integral;
//	}
	
    PID->Err =  PID->Target - CurrentValue;
    PID->PID_out = PID->Kp * PID->Err 										/*比例*/
				 + PID->Ki * PID->Integral  								/*积分*/
			     + PID->Kd * (PID->Err - PID->LastErr);						/*微分*/
	
	/*积分限幅*/
	if(PID->Integral > 1000){
		PID->Integral = 1000;
	}
	if(PID->Integral < -1000){
		PID->Integral = -1000;
	}
	PID->LastErr = PID->Err;
    return PID->PID_out;
}

// 初始化PID参数
//Initialize PID parameters
void PID_Param_Init(void)
{
    /* 速度相关初始化参数 */
	//Speed dependent initialization parameters
    for (int i = 0; i < MAX_MOTOR; i++)
    {
        pid_motor[i].target_val = 0.0;
        pid_motor[i].pwm_output = 0.0;
        pid_motor[i].err = 0.0;
        pid_motor[i].err_last = 0.0;
        pid_motor[i].err_next = 0.0;
        pid_motor[i].integral = 0.0;

        pid_motor[i].Kp = PID_DEF_KP;
        pid_motor[i].Ki = PID_DEF_KI;
        pid_motor[i].Kd = PID_DEF_KD;
    }

    pid_Yaw.Proportion = PID_YAW_DEF_KP;
    pid_Yaw.Integral = PID_YAW_DEF_KI;
    pid_Yaw.Derivative = PID_YAW_DEF_KD;
}

// Set PID parameters 设置PID参数
void PID_Set_Parm(PID_t *pid, float p, float i, float d)
{
    pid->Kp = p; // Set Scale Factor 设置比例系数 P
    pid->Ki = i; // Set integration coefficient 设置积分系数 I
    pid->Kd = d; // Set differential coefficient 设置微分系数 D
}

// Set the target value of PID 设置PID的目标值
void PID_Set_Target(PID_t *pid, float temp_val)
{
    pid->target_val = temp_val; // Set the current target value 设置当前的目标值
}

// Obtain PID target value 获取PID目标值
float PID_Get_Target(PID_t *pid)
{
    return pid->target_val; // Set the current target value 设置当前的目标值
}

// Incremental PID calculation formula 增量式PID计算公式
float PID_Incre_Calc(PID_t *pid, float actual_val)
{
    /*计算目标值与实际值的误差*/
	//Calculate the error between the target value and the actual value
    pid->err = pid->target_val - actual_val;
    /*PID算法实现*/
    //PID algorithm implementation
    pid->pwm_output += pid->Kp * (pid->err - pid->err_next) + pid->Ki * pid->err + pid->Kd * (pid->err - 2 * pid->err_next + pid->err_last);
    /*传递误差*/
    //transmission error
    pid->err_last = pid->err_next;
    pid->err_next = pid->err;

    /*返回PWM输出值*/
    /*Return PWM output value*/

    if (pid->pwm_output > (MAX_SPEED - MOTOR_DEAD_ZONE))
        pid->pwm_output = (MAX_SPEED - MOTOR_DEAD_ZONE);
    if (pid->pwm_output < (MOTOR_DEAD_ZONE - MAX_SPEED))
        pid->pwm_output = (MOTOR_DEAD_ZONE - MAX_SPEED);

    return pid->pwm_output;
}

// 位置式PID计算方式
//Position PID calculation method
float PID_Location_Calc(PID_t *pid, float actual_val)
{
    /*计算目标值与实际值的误差*/
	/*Calculate the error between the target value and the actual value*/
    pid->err = pid->target_val - actual_val;

    /* 限定闭环死区 */
    /*Limited closed-loop dead zone*/
    if ((pid->err >= -36) && (pid->err <= 36))
    {
        pid->err = 0;
        pid->integral = 0;
    }

    /* 积分分离，偏差较大时去掉积分作用 */
    /*Integral separation, removing the integral effect when the deviation is large*/
    if (pid->err > -1000 && pid->err < 1000)
    {
        pid->integral += pid->err; // error accumulation 误差累积

        /* Limit the integration range to prevent integration saturation 限定积分范围，防止积分饱和 */
        if (pid->integral > 2000)
            pid->integral = 2000;
        else if (pid->integral < -2000)
            pid->integral = -2000;
    }

    /*PID算法实现*/ /*PID algorithm implementation*/
    pid->output_val = pid->Kp * pid->err +
                      pid->Ki * pid->integral +
                      pid->Kd * (pid->err - pid->err_last);
		

    /*误差传递*/ /*Error transmission*/
    pid->err_last = pid->err;

    /*返回当前实际值*/ /*Returns the current actual value*/
    return pid->output_val;
}

// PID计算输出值 PID calculation output value
void PID_Calc_Motor(motor_data_t *motor)
{
    int i;


    for (i = 0; i < MAX_MOTOR; i++)
    {
        motor->speed_pwm[i] = PID_Incre_Calc(&pid_motor[i], motor->speed_mm_s[i]);
    }
}

// PID单独计算一条通道 PID calculates one channel separately
float PID_Calc_One_Motor(uint8_t motor_id, float now_speed)
{
    if (motor_id >= MAX_MOTOR)
        return 0;

		return PID_Location_Calc(&pid_motor[motor_id], now_speed);
		
		
}

// 设置PID参数，motor_id=4设置所有，=0123设置对应电机的PID参数。
//Set PID parameters, motor_ Id=4 Set all,=0123 Set the PID parameters of the corresponding motor.
void PID_Set_Motor_Parm(uint8_t motor_id, float kp, float ki, float kd)
{
    if (motor_id > MAX_MOTOR)
        return;

    if (motor_id == MAX_MOTOR)
    {
        for (int i = 0; i < MAX_MOTOR; i++)
        {
            pid_motor[i].Kp = kp;
            pid_motor[i].Ki = ki;
            pid_motor[i].Kd = kd;
        }
    }
    else
    {
        pid_motor[motor_id].Kp = kp;
        pid_motor[motor_id].Ki = ki;
        pid_motor[motor_id].Kd = kd;
    }
}

void Set_PID_motor(void)
{
		PID_Set_Motor_Parm(0, PID_DEF_KP,PID_DEF_KI, PID_DEF_KD);
		PID_Set_Motor_Parm(1, PID_DEF_KP,PID_DEF_KI, PID_DEF_KD);
}

// 清除PID数据
//Clear PID data
void PID_Clear_Motor(uint8_t motor_id)
{
    if (motor_id > MAX_MOTOR)
        return;

    if (motor_id == MAX_MOTOR)
    {
        for (int i = 0; i < MAX_MOTOR; i++)
        {
            pid_motor[i].pwm_output = 0.0;
            pid_motor[i].err = 0.0;
            pid_motor[i].err_last = 0.0;
            pid_motor[i].err_next = 0.0;
            pid_motor[i].integral = 0.0;
        }
    }
    else
    {
        pid_motor[motor_id].pwm_output = 0.0;
        pid_motor[motor_id].err = 0.0;
        pid_motor[motor_id].err_last = 0.0;
        pid_motor[motor_id].err_next = 0.0;
        pid_motor[motor_id].integral = 0.0;
    }
}


//带偏航角校准的PID //PID with yaw angle calibration
 
void Wheel_Yaw_PID(float yaw,float l_motor,float r_motor)
{
    float yaw_offset = PID_Yaw_Calc(yaw);
  

	
		
    float speed_L1 = l_motor - yaw_offset;
    float speed_R1 = r_motor + yaw_offset;

 
   Set_PID_Motor(speed_L1 ,speed_R1,0);

}

// 设置PID目标速度，单位为：mm/s
//Set PID target speed in mm/s
void PID_Set_Motor_Target(uint8_t motor_id, float target)
{
    if (motor_id > MAX_MOTOR)
        return;

    if (motor_id == MAX_MOTOR)
    {
        for (int i = 0; i < MAX_MOTOR; i++)
        {
            pid_motor[i].target_val = target;
        }
    }
    else
    {
        pid_motor[motor_id].target_val = target;
    }
}

// 返回PID结构体数组
//Returns an array of PID structures
PID_t *Pid_Get_Motor(void)
{
    return pid_motor;
}

/*
 ****************结合imu的PID***********
 */
// 重置偏航角的目标值
//Reset the target value of yaw angle
void PID_Yaw_Reset(float yaw)
{
    pid_Yaw.SetPoint = yaw;
    pid_Yaw.SumError = 0;
    pid_Yaw.LastError = 0;
    pid_Yaw.PrevError = 0;
}

// 计算偏航角的输出值
//Calculate the output value of yaw angle
float PID_Yaw_Calc(float NextPoint)
{
    float dError, Error;
    Error = pid_Yaw.SetPoint - NextPoint;           // deviation 偏差
    pid_Yaw.SumError += Error;                      // integral 积分
    dError = pid_Yaw.LastError - pid_Yaw.PrevError; // Current differential 当前微分
    pid_Yaw.PrevError = pid_Yaw.LastError;
    pid_Yaw.LastError = Error;

    double omega_rad = pid_Yaw.Proportion * Error            // proportional 比例项
                       + pid_Yaw.Integral * pid_Yaw.SumError // Integral term 积分项
                       + pid_Yaw.Derivative * dError;        // differential term 微分项

    if (omega_rad > PI / 6)
        omega_rad = PI / 6;
    if (omega_rad < -PI / 6)
        omega_rad = -PI / 6;
    return omega_rad;
}

float get_pid_target(PID_TypeDef *pid)
{
  return pid->Target;    // 设置当前的目标值
}

void set_pid_target(PID_TypeDef *pid, float target)
{
	pid->Target = target;
}

/**
  * @brief  设置比例、积分、微分系数
  * @param  p：比例系数 P
  * @param  i：积分系数 i
  * @param  d：微分系数 d
  *	@note 	无
  * @retval 无
  */
void set_p_i_d(PID_TypeDef *pid, float p, float i, float d)
{
  	pid->Kp = p * (pid->KP_polarity);    // 设置比例系数 P
	pid->Ki = i * (pid->KI_polarity);    // 设置积分系数 I
	pid->Kd = d * (pid->KD_polarity);    // 设置微分系数 D
}

void set_pid_polarity(PID_TypeDef *pid, int8_t p_polarity, int8_t i_polarity, int8_t d_polarity)
{
	pid->KP_polarity = p_polarity;
	pid->KI_polarity = i_polarity;
	pid->KD_polarity = d_polarity;
}




// Set parameters for yaw angle PID 设置偏航角PID的参数
void PID_Yaw_Set_Parm(float kp, float ki, float kd)
{
    pid_Yaw.Proportion = kp;
    pid_Yaw.Integral = ki;
    pid_Yaw.Derivative = kd;
}

// Set the PID encoder target value and the steering ring output value 设置PID编码器目标值以及转向环输出值
void Set_PID_Motor(float set_l ,float set_r,float turn_out)
{
		
		
		l_pid_out =	PID_Calc_One_Motor(0, -set_l);
		r_pid_out =	PID_Calc_One_Motor(1, -set_r);
		kal_L_out=KalmanFilter(&kfp_motor,l_pid_out);
		kal_R_out=KalmanFilter(&kfp_motor,r_pid_out);
		PWM_Control_Car(kal_L_out+turn_out , kal_R_out-turn_out );

}






