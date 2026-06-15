#include "bsp_beep_led.h"
#include "ti_msp_dl_config.h"

/**
 * @brief 打开MCU指示灯  Turn on MCU indicator LED
 */
void OPEN_MCULED(void)
{
    DL_GPIO_setPins(LED_PORT, LED_MCU_PIN);  // 置位LED引脚  Set LED pin
}

/**
 * @brief 关闭MCU指示灯 / Turn off MCU indicator LED
 */
void CLOSE_MCULED(void)
{
    DL_GPIO_clearPins(LED_PORT, LED_MCU_PIN);  // 清除LED引脚  Clear LED pin
}

/**
 * @brief 打开蜂鸣器  Turn on buzzer
 */
void Buzzer_open_state(void)
{
    DL_GPIO_setPins(BEEP_PORT, BEEP_Buzzer_PIN);  // 置位蜂鸣器引脚  Set buzzer pin
}

/**
 * @brief 关闭蜂鸣器 / Turn off buzzer
 */
void Buzzer_close_state(void)
{
    DL_GPIO_clearPins(BEEP_PORT, BEEP_Buzzer_PIN);  // 清除蜂鸣器引脚  Clear buzzer pin
}