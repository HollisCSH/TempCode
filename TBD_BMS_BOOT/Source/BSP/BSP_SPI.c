/****************************************************************************/
/* 	File    	BSP_SPI.c 			 
 * 	Author		Hollis
 *	Notes			使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_SPI.h"
#include "BSP_GPIO.h"
SPI_HandleTypeDef hspi1;
void MX_SPI1_Init(void)
{
	hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;						//主模式
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;	//全双工
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;			//数据位为8位
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;		//CPOL=0,low
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;								//软件控制NSS
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;	//波特率   为系统时钟分频
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    assert_param(0);
  }
}

/**
* @brief SPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hspi->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA15     ------> SPI1_NSS		PA15
    PB3     ------> SPI1_SCK		PB3
    PB4     ------> SPI1_MISO		PB4
    PB5     ------> SPI1_MOSI		PB5
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		
  }
	
}

/**
* @brief SPI MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
  if(hspi->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI3_MspDeInit 0 */

  /* USER CODE END SPI3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI3 GPIO Configuration
    PA15     ------> SPI3_NSS
    PB3     ------> SPI3_SCK
    PB4     ------> SPI3_MISO
    PB5     ------> SPI3_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);

  /* USER CODE BEGIN SPI3_MspDeInit 1 */

  /* USER CODE END SPI3_MspDeInit 1 */
  }
}
uint8_t SPI_Transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	return HAL_SPI_Transmit(&hspi1,  pData,  Size,  Timeout);//发送数据
//	return HAL_SPI_Transmit_IT(&hspi1,  pData,  Size);//发送数据
}
uint8_t SPI_Receive(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	return HAL_SPI_Receive( &hspi1,  pData,  Size, Timeout);//接收数据
//	return HAL_SPI_Receive_IT( &hspi1,  pData,  Size);//接收数据
}
uint8_t SPI_TransmitReceive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size , uint32_t Timeout)
{
	return HAL_SPI_TransmitReceive(&hspi1,  pTxData,  pRxData,  Size, Timeout);//收发数据
//HAL_StatusTypeDef HAL_SPI_TransmitReceive_IT(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}
