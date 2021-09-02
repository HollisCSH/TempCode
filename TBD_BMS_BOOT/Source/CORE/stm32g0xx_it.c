/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32g0xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef hlpuart1;
extern UART_HandleTypeDef hlpuart2;
extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;
//extern I2C_HandleTypeDef hi2c1;
//extern I2C_HandleTypeDef hi2c2;
//extern SPI_HandleTypeDef hspi1;
//extern LPTIM_HandleTypeDef hlptim1;
//extern LPTIM_HandleTypeDef hlptim2;
//extern TIM_HandleTypeDef htim2;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
//void HardFault_Handler(void)
//{
//  /* USER CODE BEGIN HardFault_IRQn 0 */

//  /* USER CODE END HardFault_IRQn 0 */
//  while (1)
//  {
//    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
//    /* USER CODE END W1_HardFault_IRQn 0 */
//  }
//}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}


void EXTI0_1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}
void EXTI2_3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}
void EXTI4_15_IRQHandler(void)	
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

/******************************************************************************/
/* STM32G0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USART1 global interrupt / USART1 wake-up interrupt through EXTI line 25.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}
void USART2_LPUART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&hlpuart2);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

void USART3_4_5_6_LPUART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&hlpuart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/******************************************************************************/
/* STM32G0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM16, FDCAN1_IT0 and FDCAN2_IT0 Interrupt.
  */
void TIM16_FDCAN_IT0_IRQHandler(void)
{
  /* USER CODE BEGIN TIM16_FDCAN_IT0_IRQn 0 */

  /* USER CODE END TIM16_FDCAN_IT0_IRQn 0 */
//  HAL_FDCAN_IRQHandler(&hfdcan1);
	HAL_FDCAN_IRQHandler(&hfdcan2);
  /* USER CODE BEGIN TIM16_FDCAN_IT0_IRQn 1 */

  /* USER CODE END TIM16_FDCAN_IT0_IRQn 1 */
}
//void I2C1_IRQHandler(void)
//{
//  /* USER CODE BEGIN I2C1_IRQn 0 */

//  /* USER CODE END I2C1_IRQn 0 */
//  if (hi2c1.Instance->ISR & (I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR)) 
//		{
//			HAL_I2C_ER_IRQHandler(&hi2c1);
//		} 
//		else 
//		{
//			HAL_I2C_EV_IRQHandler(&hi2c1);
//		}
//  /* USER CODE BEGIN I2C1_IRQn 1 */

//  /* USER CODE END I2C1_IRQn 1 */
//}
//void I2C2_3_IRQHandler(void)
//{
//  /* USER CODE BEGIN I2C1_IRQn 0 */

//  /* USER CODE END I2C1_IRQn 0 */
//  if (hi2c2.Instance->ISR & (I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR)) 
//		{
//			HAL_I2C_ER_IRQHandler(&hi2c2);
//		} 
//		else 
//		{
//			HAL_I2C_EV_IRQHandler(&hi2c2);
//		}
//  /* USER CODE BEGIN I2C1_IRQn 1 */

//  /* USER CODE END I2C1_IRQn 1 */
//}
//定時器LPTIM1中斷
//void TIM6_DAC_LPTIM1_IRQHandler(void)
//{
//  HAL_LPTIM_IRQHandler(&hlptim1);
//}
////定時器LPTIM2中斷
//void TIM7_LPTIM2_IRQHandler(void)
//{
//  HAL_LPTIM_IRQHandler(&hlptim2);
//}
////定時器TIM2中斷
//void TIM2_IRQHandler(void)
//{
//  HAL_TIM_IRQHandler(&htim2);
//}
/**
  * @brief This function handles SPI1 global interrupt.
  */
//void SPI1_IRQHandler(void)
//{
//  /* USER CODE BEGIN SPI1_IRQn 0 */

//  /* USER CODE END SPI1_IRQn 0 */
//  HAL_SPI_IRQHandler(&hspi1);
//  /* USER CODE BEGIN SPI1_IRQn 1 */

//  /* USER CODE END SPI1_IRQn 1 */
//}
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
