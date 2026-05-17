/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the LP_MSPM0G3507
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerA_backupConfig gmotor_PWMBackup;
DL_TimerA_backupConfig gTIMER_1msBackup;
DL_TimerG_backupConfig gTIMER_20msBackup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_motor_PWM_init();
    SYSCFG_DL_TIMER_1ms_init();
    SYSCFG_DL_TIMER_20ms_init();
    SYSCFG_DL_UART_0_init();
    SYSCFG_DL_SYSTICK_init();
    SYSCFG_DL_SYSCTL_CLK_init();
    /* Ensure backup structures have no valid state */
	gmotor_PWMBackup.backupRdy 	= false;
	gTIMER_1msBackup.backupRdy 	= false;
	gTIMER_20msBackup.backupRdy 	= false;


}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_saveConfiguration(motor_PWM_INST, &gmotor_PWMBackup);
	retStatus &= DL_TimerA_saveConfiguration(TIMER_1ms_INST, &gTIMER_1msBackup);
	retStatus &= DL_TimerG_saveConfiguration(TIMER_20ms_INST, &gTIMER_20msBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_restoreConfiguration(motor_PWM_INST, &gmotor_PWMBackup, false);
	retStatus &= DL_TimerA_restoreConfiguration(TIMER_1ms_INST, &gTIMER_1msBackup, false);
	retStatus &= DL_TimerG_restoreConfiguration(TIMER_20ms_INST, &gTIMER_20msBackup, false);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerA_reset(motor_PWM_INST);
    DL_TimerA_reset(TIMER_1ms_INST);
    DL_TimerG_reset(TIMER_20ms_INST);
    DL_UART_Main_reset(UART_0_INST);


    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerA_enablePower(motor_PWM_INST);
    DL_TimerA_enablePower(TIMER_1ms_INST);
    DL_TimerG_enablePower(TIMER_20ms_INST);
    DL_UART_Main_enablePower(UART_0_INST);

    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralAnalogFunction(GPIO_HFXIN_IOMUX);
    DL_GPIO_initPeripheralAnalogFunction(GPIO_HFXOUT_IOMUX);

    DL_GPIO_initPeripheralOutputFunction(GPIO_motor_PWM_C0_IOMUX,GPIO_motor_PWM_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_motor_PWM_C0_PORT, GPIO_motor_PWM_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_motor_PWM_C1_IOMUX,GPIO_motor_PWM_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_motor_PWM_C1_PORT, GPIO_motor_PWM_C1_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_motor_PWM_C2_IOMUX,GPIO_motor_PWM_C2_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_motor_PWM_C2_PORT, GPIO_motor_PWM_C2_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_motor_PWM_C3_IOMUX,GPIO_motor_PWM_C3_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_motor_PWM_C3_PORT, GPIO_motor_PWM_C3_PIN);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_0_IOMUX_TX, GPIO_UART_0_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_0_IOMUX_RX, GPIO_UART_0_IOMUX_RX_FUNC);

    DL_GPIO_initDigitalOutputFeatures(LED_MCU_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_DOWN,
		 DL_GPIO_DRIVE_STRENGTH_LOW, DL_GPIO_HIZ_DISABLE);

    DL_GPIO_initDigitalOutput(BEEP_Buzzer_IOMUX);

    DL_GPIO_initDigitalOutput(RGB_WQ2812_IOMUX);

    DL_GPIO_initDigitalOutput(OLED_SCL1_IOMUX);

    DL_GPIO_initDigitalOutput(OLED_SDA1_IOMUX);

    DL_GPIO_initDigitalInputFeatures(GPIO_ENCODER_L_H1A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_ENCODER_L_H1B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_ENCODER_R_H2A_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_ENCODER_R_H2B_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(KEY_button1_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(KEY_button2_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(KEY_button3_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(MPU6050_SDA_IOMUX);

    DL_GPIO_initDigitalOutput(MPU6050_SCL_IOMUX);

    DL_GPIO_clearPins(GPIOA, LED_MCU_PIN |
		MPU6050_SCL_PIN);
    DL_GPIO_setPins(GPIOA, OLED_SCL1_PIN |
		OLED_SDA1_PIN |
		MPU6050_SDA_PIN);
    DL_GPIO_enableOutput(GPIOA, LED_MCU_PIN |
		OLED_SCL1_PIN |
		OLED_SDA1_PIN |
		MPU6050_SDA_PIN |
		MPU6050_SCL_PIN);
    DL_GPIO_setUpperPinsPolarity(GPIOA, DL_GPIO_PIN_31_EDGE_RISE);
    DL_GPIO_clearInterruptStatus(GPIOA, GPIO_ENCODER_R_H2B_PIN);
    DL_GPIO_enableInterrupt(GPIOA, GPIO_ENCODER_R_H2B_PIN);
    DL_GPIO_clearPins(GPIOB, BEEP_Buzzer_PIN |
		RGB_WQ2812_PIN);
    DL_GPIO_enableOutput(GPIOB, BEEP_Buzzer_PIN |
		RGB_WQ2812_PIN);
    DL_GPIO_setLowerPinsPolarity(GPIOB, DL_GPIO_PIN_13_EDGE_RISE);
    DL_GPIO_setUpperPinsPolarity(GPIOB, DL_GPIO_PIN_21_EDGE_RISE |
		DL_GPIO_PIN_22_EDGE_RISE);
    DL_GPIO_clearInterruptStatus(GPIOB, GPIO_ENCODER_L_H1A_PIN |
		GPIO_ENCODER_L_H1B_PIN |
		GPIO_ENCODER_R_H2A_PIN);
    DL_GPIO_enableInterrupt(GPIOB, GPIO_ENCODER_L_H1A_PIN |
		GPIO_ENCODER_L_H1B_PIN |
		GPIO_ENCODER_R_H2A_PIN);

}


static const DL_SYSCTL_SYSPLLConfig gSYSPLLConfig = {
    .inputFreq              = DL_SYSCTL_SYSPLL_INPUT_FREQ_16_32_MHZ,
	.rDivClk2x              = 1,
	.rDivClk1               = 0,
	.rDivClk0               = 0,
	.enableCLK2x            = DL_SYSCTL_SYSPLL_CLK2X_DISABLE,
	.enableCLK1             = DL_SYSCTL_SYSPLL_CLK1_DISABLE,
	.enableCLK0             = DL_SYSCTL_SYSPLL_CLK0_ENABLE,
	.sysPLLMCLK             = DL_SYSCTL_SYSPLL_MCLK_CLK0,
	.sysPLLRef              = DL_SYSCTL_SYSPLL_REF_HFCLK,
	.qDiv                   = 7,
	.pDiv                   = DL_SYSCTL_SYSPLL_PDIV_2
};
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setFlashWaitState(DL_SYSCTL_FLASH_WAIT_STATE_2);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
	/* Set default configuration */
	DL_SYSCTL_disableHFXT();
	DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_setHFCLKSourceHFXTParams(DL_SYSCTL_HFXT_RANGE_32_48_MHZ,100, true);
    DL_SYSCTL_configSYSPLL((DL_SYSCTL_SYSPLLConfig *) &gSYSPLLConfig);
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_2);
    DL_SYSCTL_enableMFCLK();
    DL_SYSCTL_setMCLKSource(SYSOSC, HSCLK, DL_SYSCTL_HSCLK_SOURCE_SYSPLL);
    /* INT_GROUP1 Priority */
    NVIC_SetPriority(GPIOA_INT_IRQn, 0);

}
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_CLK_init(void) {
    while ((DL_SYSCTL_getClockStatus() & (DL_SYSCTL_CLK_STATUS_SYSPLL_GOOD
		 | DL_SYSCTL_CLK_STATUS_HFCLK_GOOD
		 | DL_SYSCTL_CLK_STATUS_HSCLK_GOOD
		 | DL_SYSCTL_CLK_STATUS_LFOSC_GOOD))
	       != (DL_SYSCTL_CLK_STATUS_SYSPLL_GOOD
		 | DL_SYSCTL_CLK_STATUS_HFCLK_GOOD
		 | DL_SYSCTL_CLK_STATUS_HSCLK_GOOD
		 | DL_SYSCTL_CLK_STATUS_LFOSC_GOOD))
	{
		/* Ensure that clocks are in default POR configuration before initialization.
		* Additionally once LFXT is enabled, the internal LFOSC is disabled, and cannot
		* be re-enabled other than by executing a BOOTRST. */
		;
	}
}



/*
 * Timer clock configuration to be sourced by  / 2 (40000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   40000000 Hz = 40000000 Hz / (2 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gmotor_PWMClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_2,
    .prescale = 0U
};

static const DL_TimerA_PWMConfig gmotor_PWMConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 1000,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_motor_PWM_init(void) {

    DL_TimerA_setClockConfig(
        motor_PWM_INST, (DL_TimerA_ClockConfig *) &gmotor_PWMClockConfig);

    DL_TimerA_initPWMMode(
        motor_PWM_INST, (DL_TimerA_PWMConfig *) &gmotor_PWMConfig);

    DL_TimerA_setCaptureCompareOutCtl(motor_PWM_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(motor_PWM_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(motor_PWM_INST, 1000, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(motor_PWM_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(motor_PWM_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(motor_PWM_INST, 1000, DL_TIMER_CC_1_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(motor_PWM_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_2_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(motor_PWM_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_2_INDEX);
    DL_TimerA_setCaptureCompareValue(motor_PWM_INST, 1000, DL_TIMER_CC_2_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(motor_PWM_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_3_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(motor_PWM_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_3_INDEX);
    DL_TimerA_setCaptureCompareValue(motor_PWM_INST, 1000, DL_TIMER_CC_3_INDEX);

    DL_TimerA_enableClock(motor_PWM_INST);


    
    DL_TimerA_setCCPDirection(motor_PWM_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT | DL_TIMER_CC2_OUTPUT | DL_TIMER_CC3_OUTPUT );


}



/*
 * Timer clock configuration to be sourced by MFCLK /  (800000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   800000 Hz = 800000 Hz / (5 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gTIMER_1msClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_MFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_5,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMER_1ms_INST_LOAD_VALUE = (1ms * 800000 Hz) - 1
 */
static const DL_TimerA_TimerConfig gTIMER_1msTimerConfig = {
    .period     = TIMER_1ms_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC_UP,
    .startTimer = DL_TIMER_START,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMER_1ms_init(void) {

    DL_TimerA_setClockConfig(TIMER_1ms_INST,
        (DL_TimerA_ClockConfig *) &gTIMER_1msClockConfig);

    DL_TimerA_initTimerMode(TIMER_1ms_INST,
        (DL_TimerA_TimerConfig *) &gTIMER_1msTimerConfig);
    DL_TimerA_enableInterrupt(TIMER_1ms_INST , DL_TIMERA_INTERRUPT_LOAD_EVENT);
	NVIC_SetPriority(TIMER_1ms_INST_INT_IRQN, 3);
    DL_TimerA_enableClock(TIMER_1ms_INST);





}

/*
 * Timer clock configuration to be sourced by BUSCLK /  (10000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   1000000 Hz = 10000000 Hz / (8 * (9 + 1))
 */
static const DL_TimerG_ClockConfig gTIMER_20msClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_8,
    .prescale    = 9U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMER_20ms_INST_LOAD_VALUE = (20 ms * 1000000 Hz) - 1
 */
static const DL_TimerG_TimerConfig gTIMER_20msTimerConfig = {
    .period     = TIMER_20ms_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMER_20ms_init(void) {

    DL_TimerG_setClockConfig(TIMER_20ms_INST,
        (DL_TimerG_ClockConfig *) &gTIMER_20msClockConfig);

    DL_TimerG_initTimerMode(TIMER_20ms_INST,
        (DL_TimerG_TimerConfig *) &gTIMER_20msTimerConfig);
    DL_TimerG_enableInterrupt(TIMER_20ms_INST , DL_TIMERG_INTERRUPT_ZERO_EVENT);
	NVIC_SetPriority(TIMER_20ms_INST_INT_IRQN, 2);
    DL_TimerG_enableClock(TIMER_20ms_INST);





}



static const DL_UART_Main_ClockConfig gUART_0ClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_0Config = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_0_init(void)
{
    DL_UART_Main_setClockConfig(UART_0_INST, (DL_UART_Main_ClockConfig *) &gUART_0ClockConfig);

    DL_UART_Main_init(UART_0_INST, (DL_UART_Main_Config *) &gUART_0Config);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(UART_0_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_0_INST, UART_0_IBRD_40_MHZ_115200_BAUD, UART_0_FBRD_40_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART_0_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);
    /* Setting the Interrupt Priority */
    NVIC_SetPriority(UART_0_INST_INT_IRQN, 1);


    DL_UART_Main_enable(UART_0_INST);
}

SYSCONFIG_WEAK void SYSCFG_DL_SYSTICK_init(void)
{
    /* Initialize the period to 1.00 μs */
    DL_SYSTICK_init(80);
    /* Enable the SysTick and start counting */
    DL_SYSTICK_enable();
}

