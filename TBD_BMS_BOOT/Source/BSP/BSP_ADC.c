/****************************************************************************/
/* 	File    	BSP_ADC.c 			 
 * 	Author		Hollis
 *	Notes			使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_ADC.h"
#include "stdio.h"
#define TVS_NTC_PIN 		GPIO_PIN_5
#define MOS_NTC_PIN 		GPIO_PIN_6
#define PRE_DSG_NTC_PIN GPIO_PIN_7
#define NTC_PORT 				GPIOA

#define PRE_DSG_SHORT_DET_PIN 		GPIO_PIN_0
#define PRE_DSG_SHORT_DET_PORT 		GPIOB

#define TVS_CHANNEL 			ADC_CHANNEL_5
#define MOS_CHANNEL 			ADC_CHANNEL_6
#define PRE_DSG_CHANNEL 	ADC_CHANNEL_7
#define PRE_SHORT_CHANNEL ADC_CHANNEL_8
ADC_HandleTypeDef hadc1;

/**
* @brief ADC MSP Initialization
* This function configures the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hadc->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_ADC_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA5     ------> ADC1_IN5
    PA6     ------> ADC1_IN6
    PA7     ------> ADC1_IN7
    PB0     ------> ADC1_IN8
    */
    GPIO_InitStruct.Pin = MOS_NTC_PIN|PRE_DSG_NTC_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(NTC_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PRE_DSG_SHORT_DET_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(PRE_DSG_SHORT_DET_PORT, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }

}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
void MX_ADC1_Init(void)
{
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    assert_param(0);
  }
//	 HAL_ADCEx_Calibration_Start(&hadc1);
  /** Configure Regular Channel
  */
//  sConfig.Channel = ADC_CHANNEL_0;
//  sConfig.Rank = ADC_REGULAR_RANK_1;
//  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
//  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /* USER CODE BEGIN ADC1_Init 2 */

//  /* USER CODE END ADC1_Init 2 */
//	HAL_ADC_Start(&hadc1);

}

uint32_t ADC_Get_Average(uint8_t ch,uint8_t times)
{
	ADC_ChannelConfTypeDef sConfig;		//通道初始化
	uint32_t value_sum=0;	
	uint8_t i;
	switch(ch)							//选择ADC通道
	{
		case 0:sConfig.Channel = ADC_CHANNEL_0;break;	
		case 1:sConfig.Channel = ADC_CHANNEL_1;break;	
		case 2:sConfig.Channel = ADC_CHANNEL_2;break;
		case 3:sConfig.Channel = ADC_CHANNEL_3;break;
		case 4:sConfig.Channel = ADC_CHANNEL_4;break;
		case 5:sConfig.Channel = ADC_CHANNEL_5;break;
		case 6:sConfig.Channel = ADC_CHANNEL_6;break;
		case 7:sConfig.Channel = ADC_CHANNEL_7;break;
		case 8:sConfig.Channel = ADC_CHANNEL_8;break;
	}
	sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;		//采用周期239.5周期
	sConfig.Rank = ADC_REGULAR_RANK_1;
	HAL_ADC_ConfigChannel(&hadc1,&sConfig);											
	for(i=0;i<times;i++)
	{
		HAL_ADC_Start(&hadc1);								//启动转换
//		HAL_ADC_PollForConversion(&hadc1,10);				//等待转化结束
		if(HAL_ADC_PollForConversion(&hadc1,10) == HAL_OK)
			{
		value_sum += HAL_ADC_GetValue(&hadc1);				//求和					
		HAL_ADC_Stop(&hadc1);								//停止转换
			}
	}
	return value_sum/times;									//返回平均值
}
