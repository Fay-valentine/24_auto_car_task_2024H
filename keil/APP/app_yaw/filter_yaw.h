#ifndef FILTER_YAW_H
#define FILTER_YAW_H
#include "std_types.h"
#include "lpf.h"
#include "unwrap_yaw.h"

typedef struct
{
    LPF_t lpf_yaw;
    Unwrap_yaw_t unwrap_yaw;
    float filtered_yaw;
}YawFilter_t;

void yaw_filter_Init(YawFilter_t* filter,float lpf_alpha);
void yaw_filter_Reset(YawFilter_t* filter);
float yaw_filter_Process(YawFilter_t* filter, float yaw);


#endif