#include "filter_yaw.h"
#include "AllHeader.h"

void yaw_filter_Init(YawFilter_t* filter,float lpf_alpha)
{
    LPF_Init(&filter->lpf_yaw,lpf_alpha);
    unwrap_yaw_Init(&filter->unwrap_yaw);
}

void yaw_filter_Reset(YawFilter_t* filter)
{
    LPF_Reset(&filter->lpf_yaw);
    unwrap_yaw_Reset(&filter->unwrap_yaw);
}

float yaw_filter_Process(YawFilter_t* filter, float yaw)
{
    //先获取变化后的连续角度,即线性角度
    float linear=unwrap_yaw_Process(&filter->unwrap_yaw,yaw);

    //对连续角度进行低通滤波
    linear=LPF_Update(&filter->lpf_yaw,linear);

    Wrap_Process(&linear);//将线性角度映射为环绕角度
    filter->filtered_yaw=linear;
    
    return filter->filtered_yaw;
}