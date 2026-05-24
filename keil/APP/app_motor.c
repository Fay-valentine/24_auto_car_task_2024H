#include "app_motor.h"
#include "AllHeader.h"

static float speed_lr = 0;
static float speed_fb = 0;
static float speed_spin = 0;//��ת�ٶ�

static int speed_L1_setup = 0;
static int speed_R1_setup = 0;


static int g_offset_yaw = 0;
static uint16_t g_speed_setup = 0;

// ������10����ǰ������
//Encoder data before and after 10 milliseconds
int g_Encoder_All_Now[MAX_MOTOR] = {0};
int g_Encoder_All_Last[MAX_MOTOR] = {0};
int g_Encoder_All_Offset[MAX_MOTOR] = {0};

uint8_t g_start_ctrl = 0;

car_data_t car_data;
motor_data_t motor_data;

uint8_t g_yaw_adjust = 0;

void Motors_Init(void)
{
    Motor_PID_Init(&motor_pid[0], PID_MOTOR_KP, PID_MOTOR_KI, PID_MOTOR_KD, (float)(MAX_SPEED - MOTOR_DEAD_ZONE), -(float)(MAX_SPEED - MOTOR_DEAD_ZONE));
    Motor_PID_Init(&motor_pid[1], PID_MOTOR_KP, PID_MOTOR_KI, PID_MOTOR_KD, (float)(MAX_SPEED - MOTOR_DEAD_ZONE), -(float)(MAX_SPEED - MOTOR_DEAD_ZONE));
}

static float Motion_Get_Circle_Pulse(void)
{
    return ENCODER_CIRCLE_450;
}

// ���������ӵ���������ʾ���ݡ�
//Only used to display data when added to debugging.
void *Motion_Get_Data(uint8_t index)
{
    if (index == 1)
        return (int *)g_Encoder_All_Now;
    if (index == 2)
        return (int *)g_Encoder_All_Last;
    if (index == 3)
        return (int *)g_Encoder_All_Offset;
    return 0;
}

// ��ȡ����ٶ�
//Obtain motor speed
void Motion_Get_Motor_Speed(float *speed)
{
    for (int i = 0; i < MAX_MOTOR; i++)
    {
        speed[i] = motor_data.speed_mm_s[i];
    }
}

// ����ƫ����״̬�����ʹ����ˢ��targetĿ��Ƕȡ�
//Set the yaw angle status, and if enabled, refresh the target target angle.
void Motion_Set_Yaw_Adjust(uint8_t adjust)
{
    if (adjust == 0)
    {
        g_yaw_adjust = 0;
    }
    else
    {
        g_yaw_adjust = 1;
    }
    if (g_yaw_adjust)
    {
        //PID_Yaw_Reset(��ȡ��ǰIMUƫ����-yaw);
    }
}

// ����ƫ���ǵ���״̬��
//Return to yaw angle adjustment status.
uint8_t Motion_Get_Yaw_Adjust(void)
{
    return g_yaw_adjust;
}


// Car Stop С��ֹͣ
/**
 * @brief ɲ������
 * 
 * @param brake 1: ɲ��������·����Ϊ�ߵ�ƽ����0: ����ֹͣ�����0ռ�ձȣ�
 */
void Motion_Stop(uint8_t brake)
{
    Motion_Set_Speed(0, 0);
    g_start_ctrl = 0;
    g_yaw_adjust = 0;
    Motor_PID_Reset(&motor_pid[0]);
    Motor_PID_Reset(&motor_pid[1]);
    Motor_Stop(brake);
}

// speed_mX=[-1000, 1000], ��λΪ��mm/s
//speed_mX=[-10001000],Unit: mm/s
void Motion_Set_Speed(int16_t speed_m1, int16_t speed_m2)
{
    g_start_ctrl = 1;
    Motor_Set_TargetSpeed(&motor_pid[0],speed_m1); 
    Motor_Set_TargetSpeed(&motor_pid[1],speed_m2);
}

// ����ƫ����У׼С���˶�����
//Increase yaw angle to calibrate the direction of the car's movement
void Motion_Yaw_Calc(float yaw)
{
    Wheel_Yaw_Calc(yaw);
}


//��imuУ׼��
void Wheel_Yaw_Calc(float yaw)
{
    float yaw_offset = PID_Yaw_Calc(yaw);
    g_offset_yaw = yaw_offset * g_speed_setup;

    int speed_L1 = speed_L1_setup - g_offset_yaw;
    int speed_R1 = speed_R1_setup + g_offset_yaw;

    if (speed_L1 > 1000)
        speed_L1 = 1000;
    if (speed_L1 < -1000)
        speed_L1 = -1000;
   
    if (speed_R1 > 1000)
        speed_R1 = 1000;
    if (speed_R1 < -1000)
        speed_R1 = -1000;
   
    Motion_Set_Speed(speed_L1,  speed_R1);
}

// �ӱ�������ȡ��ǰ�������ٶȣ���λmm/s
//Read the current speed of each wheel from the encoder, in mm/s
void Motion_Get_Speed(car_data_t *car)
{
    int i = 0;
    float speed_mm[MAX_MOTOR] = {0};
    float circle_mm = Motion_Get_Circle_MM();
    float circle_pulse = Motion_Get_Circle_Pulse();
    float robot_APB = Motion_Get_APB();

    Motion_Get_Encoder();

    // ���������ٶȣ���λmm/s��
    //Calculate the wheel speed in mm/s.
    for (i = 0; i < MAX_MOTOR; i++)
    {
        speed_mm[i] = (g_Encoder_All_Offset[i]) * 50 * circle_mm / circle_pulse;
    }

    car->Vx = (speed_mm[0] + speed_mm[1] ) / 2;
    car->Vy = -(speed_mm[0] - speed_mm[1] ) / 2;
    car->Vz = -(speed_mm[0] - speed_mm[1]) / 2.0f / robot_APB * 1000;

    // ���µ����������ʵ���ٶ�
    Motor_SetActualSpeed(&motor_pid[0], speed_mm[0]);
    Motor_SetActualSpeed(&motor_pid[1], speed_mm[1]);
}

// ���ص�ǰС����������͵�һ��
//Returns half of the sum of the current wheel spacing of the small car
float Motion_Get_APB(void)
{
    return MSPM0Car_APB;
}

// ���ص�ǰС������תһȦ�Ķ��ٺ���
//Returns the number of millimeters the current car wheel has rotated once
float Motion_Get_Circle_MM(void)
{
    return MECANUM_CIRCLE_MM;
}

// ��ȡ���������ݣ�������ƫ��������
//Obtain encoder data and calculate the number of deviation pulses
void Motion_Get_Encoder(void)
{
    Encoder_Get_ALL(g_Encoder_All_Now);

    for (uint8_t i = 0; i < MAX_MOTOR; i++)
    {
        // ��¼���β���ʱ����������
    	//Record the number of pulses between two test times
        g_Encoder_All_Offset[i] = g_Encoder_All_Now[i] - g_Encoder_All_Last[i];
        // ��¼�ϴα���������
        //Record Last Encoder Data
        g_Encoder_All_Last[i] = g_Encoder_All_Now[i];
    }
}

// ����С���˶�
//Control the movement of the car
void Motion_Ctrl(int16_t V_x, int16_t V_y, int16_t V_z)
{
    wheel_Ctrl(V_x, V_y, V_z);
}

void Motion_Ctrl_State(uint8_t state, uint16_t speed, uint8_t adjust)
{
    uint16_t input_speed = speed * 10;
    wheel_State_YAW(state, input_speed, adjust);
}

// ����С���˶�״̬
// �ٶȿ��ƣ�speed=0~1000��
// ƫ���ǵ����˶���adjust=1������=0��������
//Control the movement status of the car.
//Speed control: speed=0-1000.
//Yaw angle adjustment motion: adjust=1 on,=0 not on.
void wheel_State_YAW(uint8_t state, uint16_t speed, uint8_t adjust)
{
    Motion_Set_Yaw_Adjust(adjust);
    g_speed_setup = speed;
    switch (state)
    {
    case MOTION_STOP:
        g_speed_setup = 0;
        Motion_Stop(speed == 0 ? STOP_FREE : STOP_BRAKE);
        break;
    case MOTION_RUN:
        wheel_Ctrl(speed, 0, 0);
        break;
    case MOTION_BACK:
        wheel_Ctrl(-speed, 0, 0);
        break;
    case MOTION_LEFT:
    	wheel_Ctrl(speed/2, 0, -speed*2);
        break;
    case MOTION_RIGHT:
    	wheel_Ctrl(speed/2, 0, speed*2);

        break;
    case MOTION_SPIN_LEFT:
        Motion_Set_Yaw_Adjust(0);
        wheel_Ctrl(0, 0, -speed * 5);
        break;
    case MOTION_SPIN_RIGHT:
        Motion_Set_Yaw_Adjust(0);
        wheel_Ctrl(0, 0, speed * 5);
        break;
    case MOTION_BRAKE:
        Motion_Stop(STOP_BRAKE);
        break;
    default:
        break;
    }
}

// ����С���˶�״̬
// �ٶȿ��ƣ�speed=0~1000��
//Control the movement status of the car.
//Speed control: speed=0-1000.
void wheel_State(uint8_t state, uint16_t speed)
{
    g_speed_setup = speed;
    switch (state)
    {
    case MOTION_STOP:
        g_speed_setup = 0;
        Motion_Stop(speed == 0 ? STOP_FREE : STOP_BRAKE);
        break;
    case MOTION_RUN:
        wheel_Ctrl(speed, 0, 0);
        break;
    case MOTION_BACK:
        wheel_Ctrl(-speed, 0, 0);
        break;
    case MOTION_LEFT:
    	wheel_Ctrl(speed/2, 0, -speed*2);
        break;
    case MOTION_RIGHT:
    	wheel_Ctrl(speed/2, 0, speed*2);

        break;
    case MOTION_SPIN_LEFT:
        Motion_Set_Yaw_Adjust(0);
        wheel_Ctrl(0, 0, -speed * 5);
        break;
    case MOTION_SPIN_RIGHT:
        Motion_Set_Yaw_Adjust(0);
        wheel_Ctrl(0, 0, speed * 5);
        break;
    case MOTION_BRAKE:
        Motion_Stop(STOP_BRAKE);
        break;
    default:
        break;
    }
}


void wheel_Ctrl(int16_t V_x, int16_t V_y, int16_t V_z)
{
    float robot_APB = Motion_Get_APB();
//    speed_lr = -V_y;
    speed_lr = 0;
    speed_fb = V_x;
    speed_spin = (V_z / 1000.0f) * robot_APB;
    if (V_x == 0 && V_y == 0 && V_z == 0)
    {
        Motion_Stop(STOP_BRAKE);
        return;
    }

    speed_L1_setup = speed_fb + speed_lr + speed_spin;//��λ�� mm/s
    speed_R1_setup = speed_fb - speed_lr - speed_spin;

    if (speed_L1_setup > 1000)
        speed_L1_setup = 1000;
    if (speed_L1_setup < -1000)
        speed_L1_setup = -1000;
   
    if (speed_R1_setup > 1000)
        speed_R1_setup = 1000;
    if (speed_R1_setup < -1000)
        speed_R1_setup = -1000;
    
    
//    printf("%d,%d\r\n",speed_L1_setup, speed_R1_setup);
    //����pid�Ŀ��ƣ���Ӧ����ֱ��pwm����
    Motion_Set_Speed(speed_L1_setup, speed_R1_setup); //pid
    //����ʹ�õĻ�Ҫ��speed_L1_setup,speed_R1_setupתΪ��Ӧ��PWMֵ����Ӧ��ֱ��ʹ��
    // PWM_Control_Car(speed_L1_setup,speed_R1_setup);//ֱ��pwm��
}

// �˶����ƾ����ÿ20ms����һ�Σ���Ҫ�����ٶ���ص�����
//Motion control handle, called every 10ms, mainly processing speed related data
extern PID_t pid_motor[2];
void Motion_Handle(void)
{
    
    Motion_Get_Speed(&car_data);

    if (g_start_ctrl)
    {
        // ���������� PWM
        int16_t pwm_L = (int16_t)Motor_PID_Cal(&motor_pid[0]);
        int16_t pwm_R = (int16_t)Motor_PID_Cal(&motor_pid[1]);
        PWM_Control_Car(pwm_L, pwm_R);
    }
    else
    {
        PWM_Control_Car(0, 0);
    }
    
}

/**
 * @brief �ϲ������ٶȵĺ�������λ��mm/s
 * 
 * @param V_x ǰ�����ٶȣ���ֵǰ������ֵ���ˣ�
 * @param V_y �����ƶ��ٶ�
 * @param V_z ת����ٶȣ���Ч���ٶȲ���Ƕ�/�룩
 * @note ����������ǰ���ٶȺ�ת���ٶȣ�����������ָ��Ե�Ŀ�����ٶȣ����������·�������ٶȻ�ȥִ��
 */
void Motion_Car_Control(int16_t V_x, int16_t V_y, int16_t V_z)
{
	float robot_APB = Motion_Get_APB();
	speed_lr = 0;
    speed_fb = V_x;
    speed_spin = (V_z / 1000.0f) * robot_APB;
    if (V_x == 0 && V_y == 0 && V_z == 0)
    {
        Motion_Set_Speed(0,0);
        return;
    }
    //ͨ���Ӽ���ת�ٶ�ʵ��ת���ٶȲ���յõ����ָ��Ե�Ŀ���ٶ�
    speed_L1_setup = speed_fb + speed_spin;

    speed_R1_setup = speed_fb  - speed_spin;

	//�ٶ��޷�	
    if (speed_L1_setup > 1000) speed_L1_setup = 1000;
    if (speed_L1_setup < -1000) speed_L1_setup = -1000;

    if (speed_R1_setup > 1000) speed_R1_setup = 1000;
    if (speed_R1_setup < -1000) speed_R1_setup = -1000;

    Motion_Set_Speed(speed_L1_setup,speed_R1_setup);
		
}