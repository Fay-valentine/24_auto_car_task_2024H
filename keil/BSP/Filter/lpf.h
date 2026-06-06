#ifndef _LPF_H_
#define _LPF_H_

#include "std_types.h"

#define LPF_ALPHA    (0.2f)

typedef struct
{
    float alpha;//滤波系数，0-1之间,1-alpha表示本次的权重，alpha表示上次的权重
    float lpf_out;
    bool init;
} LPF_t;

void LPF_Init(LPF_t *lpf, float alpha);
void LPF_Reset(LPF_t *lpf);
float LPF_Update(LPF_t *lpf, float data);

#endif