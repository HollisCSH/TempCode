/****************************************************************************/
/* 	File    	BSP_SYSCLK.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_SYSCLK.h"


/* USER CODE BEGIN 4 */
/**
  * @brief  System Clock Speed decrease
  *         The system Clock source is shifted from PLL to HSI DIV8
  *         to go down to 2 MHz
  * @param  None
  * @retval None
  */
//void SystemClock_Decrease(void)
//{
//    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
//    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

//    /* Select HSI as system clock source */
//    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
//    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
//    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
//    {
//        assert_param(0);                  //断言 错误提示
//    }

//    /* Modify HSI to HSI DIV8 */
//    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
//    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
//    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV8;
//    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
//    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//    {
//        assert_param(0);                  //断言 错误提示
//    }
//}
#define SYSTEM_CLOCK_16M

#ifdef SYSTEM_CLOCK_64M
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;      //内部时钟16M
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;               // 2分频 得8M
  RCC_OscInitStruct.PLL.PLLN = 16;                          //16倍频 128M
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;               //PLLCLK 2分频 得到64MHz SYSCLK
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;               //PLLCLK 2分频 得到64MHz用于部分外设的时钟
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;               //PLLCLK 2分频 得到64MHz用于部分外设的时钟
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    assert_param(0);                  //断言 错误提示
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    assert_param(0);                  //断言 错误提示
  }
  /** Initializes the peripherals clocks
  */
//  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN|RCC_PERIPHCLK_RTC
//                              |RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_LPUART1
//                              |RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_I2C1
//                              |RCC_PERIPHCLK_ADC;
//  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
//  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
//  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
//  PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSI;//RCC_LPTIM1CLKSOURCE_PCLK1;
//  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
//  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
//  PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    PeriphClkInit.PeriphClockSelection  = 	RCC_PERIPHCLK_FDCAN|RCC_PERIPHCLK_RTC
                                            |RCC_PERIPHCLK_USART1
                                            |RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_LPTIM2
                                            |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_I2C2
                                            |RCC_PERIPHCLK_ADC;
    PeriphClkInit.Usart1ClockSelection  = RCC_USART1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c1ClockSelection    = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c2ClockSelection    = RCC_I2C2CLKSOURCE_PCLK1;
    PeriphClkInit.Lptim1ClockSelection  = RCC_LPTIM1CLKSOURCE_LSE;	//暂时用 RCC_LPTIM1CLKSOURCE_HSI	RCC_LPTIM1CLKSOURCE_LSE
    PeriphClkInit.Lptim2ClockSelection  = RCC_LPTIM2CLKSOURCE_LSE;
    PeriphClkInit.AdcClockSelection     = RCC_ADCCLKSOURCE_SYSCLK;
    PeriphClkInit.RTCClockSelection     = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInit.FdcanClockSelection   = RCC_FDCANCLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    assert_param(0);                  //断言 错误提示
  }
}
#endif

#ifdef SYSTEM_CLOCK_24M
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;      //内部时钟16M
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;               // 4分频 得4M
  RCC_OscInitStruct.PLL.PLLN = 12;                          //12倍频 48M
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;               //PLLCLK 2分频 得到24MHz SYSCLK
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;               //PLLCLK 2分频 得到24MHz用于部分外设的时钟
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;               //PLLCLK 2分频 得到24MHz用于部分外设的时钟
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    assert_param(0);                  //断言 错误提示
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    assert_param(0);                  //断言 错误提示
  }
  /** Initializes the peripherals clocks
  */
//  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN|RCC_PERIPHCLK_RTC
//                              |RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_LPUART1
//                              |RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_I2C1
//                              |RCC_PERIPHCLK_ADC;
//  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
//  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
//  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
//  PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSI;//RCC_LPTIM1CLKSOURCE_PCLK1;
//  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
//  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
//  PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    PeriphClkInit.PeriphClockSelection  = 	RCC_PERIPHCLK_FDCAN|RCC_PERIPHCLK_RTC
                                            |RCC_PERIPHCLK_USART1
                                            |RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_LPTIM2
                                            |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_I2C2
                                            |RCC_PERIPHCLK_ADC;
    PeriphClkInit.Usart1ClockSelection  = RCC_USART1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c1ClockSelection    = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c2ClockSelection    = RCC_I2C2CLKSOURCE_PCLK1;
    PeriphClkInit.Lptim1ClockSelection  = RCC_LPTIM1CLKSOURCE_LSE;	//暂时用 RCC_LPTIM1CLKSOURCE_HSI	RCC_LPTIM1CLKSOURCE_LSE
    PeriphClkInit.Lptim2ClockSelection  = RCC_LPTIM2CLKSOURCE_LSE;
    PeriphClkInit.AdcClockSelection     = RCC_ADCCLKSOURCE_SYSCLK;
    PeriphClkInit.RTCClockSelection     = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInit.FdcanClockSelection   = RCC_FDCANCLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    assert_param(0);                  //断言 错误提示
  }
}
#endif

#ifdef SYSTEM_CLOCK_16M
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Configure the main internal regulator output voltage
    */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Configure LSE Drive Capability
    */
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
                              
    //RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    /** Initializes the peripherals clocks
    */
    PeriphClkInit.PeriphClockSelection = 	RCC_PERIPHCLK_FDCAN|RCC_PERIPHCLK_RTC
                                                |RCC_PERIPHCLK_USART1
                                                |RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_LPTIM2
                                                |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_I2C2
                                                |RCC_PERIPHCLK_ADC;
    PeriphClkInit.Usart1ClockSelection  = RCC_USART1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c1ClockSelection    = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInit.I2c2ClockSelection    = RCC_I2C2CLKSOURCE_PCLK1;
    PeriphClkInit.Lptim1ClockSelection  = RCC_LPTIM1CLKSOURCE_LSE;	//暂时用 RCC_LPTIM1CLKSOURCE_HSI	RCC_LPTIM1CLKSOURCE_LSE
    PeriphClkInit.Lptim2ClockSelection  = RCC_LPTIM2CLKSOURCE_LSE;
    PeriphClkInit.AdcClockSelection     = RCC_ADCCLKSOURCE_SYSCLK;
    PeriphClkInit.RTCClockSelection     = RCC_RTCCLKSOURCE_LSE; 
    PeriphClkInit.FdcanClockSelection   = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
}
#endif
/*****************************************************
	* 函数功能: 系统初始化
	* 输入参数: void
	* 返 回 值: 
	* 说    明：针对睡眠后唤醒 HAL库和时钟初始化
******************************************************/
void BSPICSInit(void)
{
	HAL_Init();																//HAL库初始化
	SystemClock_Config();													//系统时钟初始化
}

//=============================================================================================
//函数名称	: void BSPChangeInterruptPrio(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 改变中断优先级 
//注    意	:  
//=============================================================================================
void BSPChangeInterruptPrio(void) //G0系列子优先级无效
{
    HAL_NVIC_DisableIRQ(USART3_4_5_6_LPUART1_IRQn);
    HAL_NVIC_SetPriority(USART3_4_5_6_LPUART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_4_5_6_LPUART1_IRQn);

    HAL_NVIC_DisableIRQ(USART1_IRQn);
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    
    HAL_NVIC_DisableIRQ(ADC1_COMP_IRQn);
    HAL_NVIC_SetPriority(ADC1_COMP_IRQn, 0x03, 0);
    HAL_NVIC_EnableIRQ(ADC1_COMP_IRQn);

    HAL_NVIC_DisableIRQ(TIM16_FDCAN_IT0_IRQn);
    HAL_NVIC_SetPriority(TIM16_FDCAN_IT0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM16_FDCAN_IT0_IRQn);

    HAL_NVIC_DisableIRQ(I2C1_IRQn);
    HAL_NVIC_SetPriority(I2C1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(I2C1_IRQn);

    HAL_NVIC_DisableIRQ(I2C2_3_IRQn);
    HAL_NVIC_SetPriority(I2C2_3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(I2C2_3_IRQn);

    HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
    HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

