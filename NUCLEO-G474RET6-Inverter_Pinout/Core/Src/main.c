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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mylibs/shell.h"
#include <stdio.h>
#include <stdlib.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define PWM_MAX_VAL 4250		// Maximum PWM value (= TIM1->ARR)
#define PWM_VARIATION_RATE 2	// Variation rate of the PWM pulse

#define ADC_CURRENT_RESOLUTION 0.05
#define ADC_VCC 3.3
#define ADC_MAX_VAL 4096
#define ADC_OFFSET 2421
#define ADC_BUFF_SIZE 1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int current_speed_PWM;
int requested_speed_PWM;

uint32_t pData[ADC_BUFF_SIZE];

double U_Imes = 0;
double Uadc = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief Sends a character via UART (overwrites the default character transmission).
 * @param ch Character to send
 * @return The sent character
 */
int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

/**
 * @brief Adjusts PWM pulse for TIM1 channels.
 * @param pulse Desired PWM pulse width
 * @attention The global variables current_speed_PWM and requested_speed_PWM must be initialized.
 */
void set_PWM()
{
	if (requested_speed_PWM < current_speed_PWM)
	{
		current_speed_PWM -= 1;
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
				current_speed_PWM);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
				__HAL_TIM_GET_AUTORELOAD(&htim1) - current_speed_PWM);
	}
	else if (requested_speed_PWM > current_speed_PWM)
	{
		current_speed_PWM += 1;
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
				current_speed_PWM);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
				__HAL_TIM_GET_AUTORELOAD(&htim1) - current_speed_PWM);
	}
}

/**
 * @brief Sets the PWM duty cycle ratio for TIM1 channels.
 * @param ratio Duty cycle ratio (0.0 to 1.0) with 12-bit resolution
 */
void set_PWM_ratio(double ratio)
{
	if (ratio < 1 && ratio > 0)
	{
		// Set main PWM pulse width for Channel 1 and Channel 2
		requested_speed_PWM = (int)(ratio * PWM_MAX_VAL);
	}
}

/**
 * @brief Sets a specific PWM pulse width for TIM1 channels.
 * @param speed Desired PWM pulse width (0 to PWM_MAX_VAL)
 */
void set_PWM_speed(int speed)
{
	if (speed < PWM_MAX_VAL && speed > 0)
	{
		// Set main PWM pulse width for Channel 1 and Channel 2
		requested_speed_PWM = speed;
	}
}

/**
 * @brief Starts PWM on TIM1 channels. Initial base speed set to 0.5 duty cycle.
 */
void start_PWM()
{
	// TIM1 Channel 1 Initialisation
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

	// TIM1 Channel 2 Initialisation
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

	current_speed_PWM = (int)(PWM_MAX_VAL/2)+1; // We initialize the base speed to 0 (cyclic rate 0.5)
	set_PWM_ratio(0.5);
}

/**
 * @brief Stops PWM on TIM1 channels.
 */
void stop_PWM()
{
	// We disable Tim1 channel 1
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

	// We disable Tim1 channel 2
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
 * @brief Reads the current U_Imes.
 */
void read_current()
{
	// Read ADC1 DMA to update pData
	printf("\r\nRAW ADC value: %d\r\n", (int)(pData[0]));

	/**
	 * Convertion taking into account the offset due to the unsigned ADC measure:
	 * 	Resolution: 50 mV/A
	 * 	Vout = 3.3/2 + 0.05*Imeasured
	 **/
	Uadc = ADC_VCC * ((int)(pData[0]) - ADC_OFFSET) / ADC_MAX_VAL;
	U_Imes = (Uadc) / ADC_CURRENT_RESOLUTION;

	printf("\r\nMeasured tension: %f V\r\n", Uadc);
	printf("\r\nMeasured current: %f A\r\n", U_Imes);
}

/**
 * @brief  Conversion complete callback in non-blocking mode. Updates the data read from the DMA.
 * @param hadc ADC handle
 * @retval None
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC1)
		HAL_ADC_Start_DMA(hadc, pData, ADC_BUFF_SIZE);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC2_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim2);

	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, pData, ADC_BUFF_SIZE);

	start_PWM();

	Shell_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
		Shell_Loop();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV6;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

	if (htim->Instance == TIM2)
	{
		set_PWM();
	}

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
