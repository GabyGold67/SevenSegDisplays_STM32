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

/* USER CODE BEGIN PV */
TaskHandle_t mainCtrlTaskHandle {NULL};
BaseType_t xReturned;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void Error_Handler(void);

/* USER CODE BEGIN PFP */
void mainCtrlTsk(void *pvParameters);
/* USER CODE END PFP */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  /* Create the thread(s) */
  /* USER CODE BEGIN RTOS_THREADS */
  xReturned = xTaskCreate(
		  mainCtrlTsk, //taskFunction
		  "MainControlTask", //Task function legible name
		  512, // Stack depth in words
		  NULL,	//Parameters to pass as arguments to the taskFunction
		  configTIMER_TASK_PRIORITY,	//Set to the same priority level as the software timers
		  &mainCtrlTaskHandle
		  );
  if(xReturned != pdPASS)
	  Error_Handler();

  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  while (1)
  {
  }
}

/* USER CODE BEGIN */
void mainCtrlTsk(void *pvParameters)
{

	gpioPinId_t dps00Sclk {GPIOA, GPIO_PIN_5};
	gpioPinId_t dsp00Rclk {GPIOA, GPIO_PIN_6};
	gpioPinId_t dsp00Dio {GPIOB, GPIO_PIN_12};
	gpioPinId_t myDspPins[]{dps00Sclk, dsp00Rclk, dsp00Dio};
	//gpioPinId_t myDspPins[]{{GPIOA, GPIO_PIN_5}, {GPIOA, GPIO_PIN_6}, {GPIOB, GPIO_PIN_12}};	//Optional shorthand valid array construction

	SevenSegDynHC595 myDspHw(myDspPins, 4 , true);
	SevenSegDisplays mySevenSegDisp(&myDspHw);
	mySevenSegDisp.begin();

	const int frstTstNum{0};
	int tstNum{frstTstNum};

	unsigned long tstTmStrt{0};
	unsigned long tstTmLngth{0};
	unsigned long dfltTstTm{2000};
	bool tstEnded{true};
	bool tmpBlnkMsk[4];

	for(;;)
	{
		if(tstEnded){
			tstTmStrt = xTaskGetTickCount() / portTICK_RATE_MS;
			tstTmLngth = dfltTstTm;
			tstEnded = false;

			switch(tstNum){
				case 0:
					mySevenSegDisp.print("Pau.G.");
					break;
				case 1:
					mySevenSegDisp.print("GabY");
					break;
				case 2:
					mySevenSegDisp.blink(500);
					break;
				case 3:
					tmpBlnkMsk[0] = true;
					tmpBlnkMsk[1] = true;
					tmpBlnkMsk[2] = false;
					tmpBlnkMsk[3] = false;
					mySevenSegDisp.setBlinkMask(tmpBlnkMsk);
					mySevenSegDisp.setBlinkRate(250);
					break;
				case 4:
					tmpBlnkMsk[0] = false;
					tmpBlnkMsk[1] = false;
					tmpBlnkMsk[2] = true;
					tmpBlnkMsk[3] = true;
					mySevenSegDisp.setBlinkMask(tmpBlnkMsk);
					mySevenSegDisp.setBlinkRate(100);
					break;
				case 5:
					mySevenSegDisp.resetBlinkMask();
					mySevenSegDisp.noBlink();
					mySevenSegDisp.print(321);
					break;
				case 6:
					mySevenSegDisp.print(321, true);
					break;
				case 7:
					mySevenSegDisp.print(321, true, true);
					break;
				case 8:
					mySevenSegDisp.wait(500);
					break;
				case 9:
					mySevenSegDisp.setWaitRate(250);
					break;
				case 10:
					mySevenSegDisp.setWaitRate(100);
					break;
				case 11:
					mySevenSegDisp.noWait();
					mySevenSegDisp.print(2.3456, 1, true);
					break;
				case 12:
					mySevenSegDisp.print(2.3456, 1, true, true);
					break;
				case 13:
					mySevenSegDisp.print(-2.3456, 1);
					break;
				case 14:
					mySevenSegDisp.print(-2.3456, 1, true);
					break;
				case 15:
					mySevenSegDisp.print(-2.3456, 1, true, true);
					break;
				case 16:
					mySevenSegDisp.print(-2.3456, 2, true, true);
					break;
				case 17:
					mySevenSegDisp.gauge(3, 'b');
					break;
				case 18:
					mySevenSegDisp.gauge(2, 'b');
					break;
				case 19:
					mySevenSegDisp.gauge(1, 'b');
					break;
				case 20:
					mySevenSegDisp.gauge(0, 'b');
					break;
				default:
					mySevenSegDisp.noBlink();
					mySevenSegDisp.noWait();
					tstNum = frstTstNum - 1;
					tstTmStrt = 0;
			};
		}

		if((xTaskGetTickCount() / portTICK_RATE_MS - tstTmStrt) > tstTmLngth){
			++tstNum;
			tstEnded = true;
		}
	}
}
/* USER CODE END */

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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
//  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}

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
