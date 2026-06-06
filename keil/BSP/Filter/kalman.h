#ifndef __Kalman_H
#define __Kalman_H




typedef struct 
{
    float Last_P;//�ϴι���Э���� ������Ϊ0 ! ! ! ! ! 
    float Now_P;//��ǰ����Э����
    float out;//�������˲������
    float Kg;//����������
    float Q;//��������Э����
    float R;//�۲�����Э����
	float source;
	float after_kalman;
}Kalman;

void Kalman_Init(Kalman *kalman);
float KalmanFilter(Kalman *kfp,float input);
extern Kalman kfp_motor;


#endif
