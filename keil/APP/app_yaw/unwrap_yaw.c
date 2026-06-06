#include "unwrap_yaw.h"
#include "AllHeader.h"

void Wrap_Process(float *angle)
{
    *angle = fmodf(*angle, 360.0f);//浮动数取余，将角度化为0-360度
    //将角度映射到(-180,180)度
    if (*angle > 180.0f) 
    {
        *angle -= 360.0f;
    }
    else if (*angle < -180.0f) 
    {
        *angle += 360.0f;
    }
}


void unwrap_yaw_Init(Unwrap_yaw_t* uw)
{
    uw->unwrap_yaw = 0.0f;
    uw->init = 0;
}

/**
 * @brief 重置init标志位
 */
void unwrap_yaw_Reset(Unwrap_yaw_t* uw)
{
    uw->init = 0;
}
/**
 * @brief 连续角度处理,获取变化后的连续角度
 * @return float 连续角度，需要映射到(-180,180)
 */
float unwrap_yaw_Process(Unwrap_yaw_t* uw, float yaw)
{
    if (!uw->init)//初次执行，赋值为原始值
    {
        uw->unwrap_yaw = yaw;
        uw->init = 1;
        return yaw;
    }

    //将连续值化为环绕值wrap
    float wrap=fmodf(uw->unwrap_yaw,360.0f);//化为0-360度
    Wrap_Process(&wrap);//映射到(-180,180)

    float diff=yaw-wrap;//计算角度差
    Wrap_Process(&diff);//映射到(-180,180)

    //更新连续角度
    uw->unwrap_yaw+=diff;
    
    return uw->unwrap_yaw;
}