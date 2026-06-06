#include "lpf.h"
#include "AllHeader.h"

void LPF_Init(LPF_t *lpf, float alpha) 
{ 
    lpf->alpha = alpha;
    lpf->lpf_out = 0.0f;
    lpf->init = 0;
}

void LPF_Reset(LPF_t *lpf)
{
    lpf->init = 0;
    lpf->lpf_out = 0.0f;
}

float LPF_Update(LPF_t *lpf, float input)
{ 
    if (!lpf->init)
    {
        lpf->init = 1;
        lpf->lpf_out = input;//初次执行，返回原始值
        return lpf->lpf_out;
    }
    lpf->lpf_out = lpf->lpf_out * lpf->alpha + input * (1.0f - lpf->alpha);
    return lpf->lpf_out;
}
