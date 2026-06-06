#ifndef _UNWRAP_YAW_H_
#define _UNWRAP_YAW_H_
#include "std_types.h"

typedef struct
{
    float unwrap_yaw;
    bool init;
}Unwrap_yaw_t;

void Wrap_Process(float *angle);
void unwrap_yaw_Init(Unwrap_yaw_t* unwrap);
void unwrap_yaw_Reset(Unwrap_yaw_t* unwrap);
float unwrap_yaw_Process(Unwrap_yaw_t* unwrap, float yaw);

#endif
