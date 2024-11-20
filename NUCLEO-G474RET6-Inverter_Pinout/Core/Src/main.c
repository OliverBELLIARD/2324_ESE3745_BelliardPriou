/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "mylibs/shell.h"
#include <stdio.h>
#include <stdlib.h>
/* USER CODE END Includes */

/**
 * @file main.c
 * @brief Entry point of the application, containing the main loop and peripheral initializations.
 */

/* Private defines -----------------------------------------------------------*/
/** @brief Maximum PWM value, corresponding to TIM1->ARR. */
#define PWM_MAX_VAL 4250

/** @brief Rate of variation for PWM pulse adjustment. */
#define PWM_VARIATION_RATE 2

/** @brief Current sensor resolution in volts per ampere. */
#define ADC_CURRENT_RESOLUTION 0.05

/** @brief ADC supply voltage. */
#define ADC_VCC 3.3

/** @brief Maximum ADC digital value (12-bit resolution). */
#define ADC_MAX_VAL 4096

/** @brief Offset value for ADC measurements. */
#define ADC_OFFSET 2421

/** @brief Size of the ADC data buffer. */
#define ADC_BUFF_SIZE 1

/* Private variables ---------------------------------------------------------*/
/** @brief Current PWM speed value. */
int current_speed_PWM;

/** @brief Requested PWM speed value. */
int requested_speed_PWM;

/** @brief Buffer to store ADC data. */
uint32_t pData[ADC_BUFF_SIZE];

/** @brief Measured voltage from the ADC. */
static float U_Imes = 0;

/** @brief ADC voltage output. */
static float Uadc = 0;

/**
 * @brief Sends a character via UART (overrides default character transmission).
 * @param ch Character to send.
 * @return The sent character.
 */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief Adjusts the PWM pulse for TIM1 channels.
 *
 * This function modifies the `current_speed_PWM` to gradually approach
 * `requested_speed_PWM` and sets the appropriate pulse widths for TIM1 channels.
 */
void set_PWM()
{
    if (requested_speed_PWM < current_speed_PWM)
    {
        current_speed_PWM -= 1;
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, current_speed_PWM);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
                               __HAL_TIM_GET_AUTORELOAD(&htim1) - current_speed_PWM);
    }
    else if (requested_speed_PWM > current_speed_PWM)
    {
        current_speed_PWM += 1;
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, current_speed_PWM);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
                               __HAL_TIM_GET_AUTORELOAD(&htim1) - current_speed_PWM);
    }
}

/**
 * @brief Sets the PWM duty cycle ratio for TIM1 channels.
 * @param ratio Duty cycle ratio (0.0 to 1.0).
 */
void set_PWM_ratio(double ratio)
{
    if (ratio > 0 && ratio < 1)
    {
        requested_speed_PWM = (int)(ratio * PWM_MAX_VAL);
    }
}

/**
 * @brief Sets a specific PWM pulse width for TIM1 channels.
 * @param speed Desired PWM pulse width (0 to PWM_MAX_VAL).
 */
void set_PWM_speed(int speed)
{
    if (speed > 0 && speed < PWM_MAX_VAL)
    {
        requested_speed_PWM = speed;
    }
}

/**
 * @brief Starts PWM on TIM1 channels.
 *
 * Initializes the PWM channels with a base speed corresponding to a 50% duty cycle.
 */
void start_PWM()
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

    current_speed_PWM = PWM_MAX_VAL / 2 + 1;
    set_PWM_ratio(0.5);
}

/**
 * @brief Stops PWM on TIM1 channels.
 */
void stop_PWM()
{
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
}

/**
 * @brief Resets the microcontroller.
 */
void reset_inverter()
{
    NVIC_SystemReset();
}

/**
 * @brief Reads and prints the measured current using the ADC.
 *
 * Converts the ADC value to a voltage and calculates the current based
 * on the sensor's resolution.
 */
void read_current()
{
    printf("\r\nRAW ADC value: %d\r\n", (int)(pData[0]));

    Uadc = ADC_VCC * ((int)(pData[0]) - ADC_OFFSET) / ADC_MAX_VAL;
    U_Imes = Uadc / ADC_CURRENT_RESOLUTION;

    printf("\r\nMeasured tension: %f V\r\n", Uadc);
    printf("\r\nMeasured current: %f A\r\n", U_Imes);
}

/**
 * @brief ADC conversion complete callback. Restarts DMA for continuous conversion.
 * @param hadc ADC handle.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        HAL_ADC_Start_DMA(hadc, pData, ADC_BUFF_SIZE);
    }
}

/**
 * @brief Main entry point of the application.
 *
 * Initializes peripherals, starts the PWM, and enters the infinite loop
 * where the shell command interface runs.
 * @retval int Always returns 0.
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC2_Init();
    MX_ADC1_Init();
    MX_TIM1_Init();
    MX_TIM3_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();
    MX_TIM2_Init();

    HAL_TIM_Base_Start_IT(&htim2);

    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADC_Start_DMA(&hadc1, pData, ADC_BUFF_SIZE);

    start_PWM();
    Shell_Init();

    while (1)
    {
        Shell_Loop();
    }
}

/**
 * @brief Configures the system clock.
 */
void SystemClock_Config(void)
{
    // Clock configuration details omitted for brevity
}

/**
 * @brief Timer period elapsed callback. Updates PWM when TIM2 interrupt occurs.
 * @param htim Timer handle.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        set_PWM();
    }

    if (htim->Instance == TIM6)
    {
        HAL_IncTick();
    }
}

/**
 * @brief Error handler function.
 *
 * Called in case of an unrecoverable error.
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

/**
 * @brief Assert failed callback.
 * @param file File where the assertion occurred.
 * @param line Line number of the assertion.
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    printf("Wrong parameter value: file %s on line %d\r\n", file, line);
}
