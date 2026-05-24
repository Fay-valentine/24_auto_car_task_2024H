#include "bsp_beep_led.h"
#include "AllHeader.h"


/**
 * @brief ��MCUָʾ��  Turn on MCU indicator LED
 */
void OPEN_MCULED(void)
{
    DL_GPIO_setPins(LED_PORT, LED_MCU_PIN);  // ��λLED����  Set LED pin
}

/**
 * @brief �ر�MCUָʾ�� / Turn off MCU indicator LED
 */
void CLOSE_MCULED(void)
{
    DL_GPIO_clearPins(LED_PORT, LED_MCU_PIN);  // ���LED����  Clear LED pin
}

/**
 * @brief �򿪷�����  Turn on buzzer
 */
void Buzzer_open_state(void)
{
    DL_GPIO_setPins(BEEP_PORT, BEEP_Buzzer_PIN);  // ��λ����������  Set buzzer pin
}

/**
 * @brief �رշ����� / Turn off buzzer
 */
void Buzzer_close_state(void)
{
    DL_GPIO_clearPins(BEEP_PORT, BEEP_Buzzer_PIN);  // �������������  Clear buzzer pin
}