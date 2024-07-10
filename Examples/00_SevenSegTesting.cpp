/**
  ******************************************************************************
  * @file	: 00_SevenSegTesting.cpp
  * @brief  : Test for the SevenSegDisplays_STM32 library SevenSegDisplays class
  *
  * 	The test instantiates a SevenSegDisplays class object using a SevenSegDynHC595 class object as argument:
  * 	The SevenSegDynHC595 class object as uses the following argument:
  * 		- A digital output to GPIO_PA05 to be used by the display sclk
  * 		- A digital output to GPIO_PA06 to be used by the display rclk
  * 		- A digital output to GPIO_PB12 to be used by the display dio
  *
  * 	@author	: Gabriel D. Goldman
  *
  * 	@date	: 	08/05/2024 First release
  * 				14/06/2024 Last update
  *
  ******************************************************************************
  * @attention	This file is part of the Examples folder for the MPBttnAsSwitch_ESP32
  * library. All files needed are provided as part of the source code for the library.
  *
  ******************************************************************************
  */

#ifndef MCU_SPEC
	#define MCU_SPEC 1

	#ifndef __STM32F4xx_HAL_H
		#include "stm32f4xx_hal.h"
	#endif

	#ifndef __STM32F4xx_HAL_GPIO_H
		#include "stm32f4xx_hal_gpio.h"
	#endif
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//===========================>> Next lines used to avoid CMSIS wrappers
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "event_groups.h"
//===========================>> Previous lines used to avoid CMSIS wrappers

#include "../../SevenSegDisplays_STM32/src/sevenSegDisplays.h"
#include "../../SevenSegDisplays_STM32/src/sevenSegDispHw.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
TaskHandle_t tstDefTaskHandle {NULL};
BaseType_t xReturned;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void Error_Handler(void);

/* USER CODE BEGIN PFP */
void tstDefTaskExec(void *pvParameters);
/* USER CODE END PFP */

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

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();

  /* Create the thread(s) */
  /* USER CODE BEGIN RTOS_THREADS */
  xReturned = xTaskCreate(
		  tstDefTaskExec, //taskFunction
		  "TstMainTask", //Task function legible name
		  256, // Stack depth in words
		  NULL,	//Parameters to pass as arguments to the taskFunction
		  configTIMER_TASK_PRIORITY,	//Set to the same priority level as the software timers
		  &tstDefTaskHandle);
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  while (1)
  {
  }
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
//  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}

/* USER CODE BEGIN 4 */
void tstDefTaskExec(void *pvParameters)
{
//---------------------------_sclk-------------_rclk-----------_dio
//	gpioPinId_t myDspPins[]{{GPIOA, 1 << 5}, {GPIOA, 1 << 6}, {GPIOB, 1 << 12}};

	gpioPinId_t myDspPins[]{{GPIOA, GPIO_PIN_5}, {GPIOA, GPIO_PIN_6}, {GPIOB, GPIO_PIN_12}};
	SevenSegDynHC595 myDspHw(myDspPins, 4 , true);

	SevenSegDisplays mySevenSegDisp(&myDspHw);

	mySevenSegDisp.begin();

	for(;;)
	{
		mySevenSegDisp.print("GabY");
		vTaskDelay(1000);
		mySevenSegDisp.blink();
		mySevenSegDisp.print("Pau.G.");
		vTaskDelay(3000);
		mySevenSegDisp.noBlink();
//		mySevenSegDisp.print(321);
//		vTaskDelay(1000);
//		mySevenSegDisp.print(321, true);
//		vTaskDelay(1000);
//		mySevenSegDisp.print(321, true, true);
//		vTaskDelay(1000);
//		mySevenSegDisp.print("87.6.5");
//		vTaskDelay(1000);
//		mySevenSegDisp.print(2.3456,1);
//		vTaskDelay(1000);
//		mySevenSegDisp.print(2.3456,1, true);
//		vTaskDelay(1000);
//		mySevenSegDisp.print(2.3456,1, true, true);
//		vTaskDelay(1000);
//		mySevenSegDisp.print(-2.3456,1);
//		vTaskDelay(1000);
//		mySevenSegDisp.print(-2.3456,1, true);
//		vTaskDelay(1000);
//		mySevenSegDisp.print(-2.3456,1, true, true);
//		vTaskDelay(1000);
	}
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM9 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM9) {
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
