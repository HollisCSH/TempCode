/****************************************************************************/
/* 	File    	BSP_SPI.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_SPI.h"
#include "BSP_GPIO.h"
#include "BSP_UART.h"
SPI_HandleTypeDef hspi1;
//static const uint32_t SPI_PrescalerIndex[] = 		//发送数据长度转换
//{
//	SPI_BAUDRATEPRESCALER_2,SPI_BAUDRATEPRESCALER_4,SPI_BAUDRATEPRESCALER_8,SPI_BAUDRATEPRESCALER_16,
//	SPI_BAUDRATEPRESCALER_32,SPI_BAUDRATEPRESCALER_64,SPI_BAUDRATEPRESCALER_128,SPI_BAUDRATEPRESCALER_256
//};
/*****************************************************
	* 函数功能: SPI1经典配置 
	* 输入参数: void
	* 返 回 值: void
	* 说    明：分频系数决定SPI频率    设定4M    24M晶振为6M
******************************************************/
void MX_SPI1_Init(void)
{
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;						//主模式
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;			//全双工
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;				//数据位为8位
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;				//CPOL=0,low
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;							//软件控制NSS 
//	hspi1.Init.BaudRatePrescaler = SPI_PrescalerIndex[SPI_BaudRatePrescaler-3];	//波特率   为系统时钟分频
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;	//波特率   为系统时钟分频
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	hspi1.State = HAL_SPI_STATE_RESET;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		assert_param(0);                  					//断言 错误提示
	}
}

/*****************************************************
	* 函数功能: SPI初始化
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：为兼容外部代码改名
******************************************************/
void BSPSPIInit(void)
{
	MX_SPI1_Init();
}

/*****************************************************
	* 函数功能: SPI 关断
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：为兼容外部代码改名
******************************************************/
void BSPSPIDisable(void)
{
	HAL_SPI_MspDeInit(&hspi1);
}

/*****************************************************
	* 函数功能: SPI 时钟 端口初始化
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：为兼容外部代码改名
******************************************************/
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hspi->Instance==SPI1)
    {
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

        __HAL_RCC_GPIOA_CLK_ENABLE();	//端口A初始化
        GPIO_InitStruct.Pin = FLASH_CS_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/*****************************************************
	* 函数功能: SPI 时钟 端口 关断
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：为兼容外部代码改名
******************************************************/
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
    if(hspi->Instance==SPI1)
    {
        __HAL_RCC_SPI1_CLK_DISABLE();

        /**SPI3 GPIO Configuration
        PA15     ------> SPI3_NSS
        PB3     ------> SPI3_SCK
        PB4     ------> SPI3_MISO
        PB5     ------> SPI3_MOSI */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);

        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
    }
}


/*****************************************************
    * 函数功能: SPI发送
	* 输入参数: *pData 数据指针 Size数据长度 Timeout超时
	* 返 回 值: 无
	* 说    明：死等型，发完或者超时结束
******************************************************/
uint8_t SPI_Transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	return HAL_SPI_Transmit(&hspi1,  pData,  Size,  Timeout);//发送数据
//	return HAL_SPI_Transmit_IT(&hspi1,  pData,  Size);//发送数据
}

/*****************************************************
    * 函数功能: SPI接收
	* 输入参数: *pData 数据指针 Size数据长度 Timeout超时
	* 返 回 值: 无
	* 说    明：死等型，接收或者超时结束
******************************************************/
uint8_t SPI_Receive(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	return HAL_SPI_Receive( &hspi1,  pData,  Size, Timeout);//接收数据
//	return HAL_SPI_Receive_IT( &hspi1,  pData,  Size);//接收数据
}

//uint8_t SPI_TransmitReceive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size , uint32_t Timeout)
//{
//	return HAL_SPI_TransmitReceive(&hspi1,  pTxData,  pRxData,  Size, Timeout);//收发数据
////HAL_StatusTypeDef HAL_SPI_TransmitReceive_IT(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
//}

//void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
//{

//}
//void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
//{

//}
/*****************************************************
    * 函数功能: SPI发送单字节
	* 输入参数: SPI_TypeDef* hspi 数据：pData
	* 返 回 值: 无
	* 说    明：
******************************************************/
uint8_t BSPSPISendOneData(SPI_TypeDef* hspi , uint8_t pData)
{
    HAL_StatusTypeDef result;
    result = HAL_SPI_Transmit(&hspi1,  &pData,  1,  10);
    if(result != HAL_OK)
    {
        assert_param(0);                  //断言提示
    }
    return result;//发送数据
}

/*****************************************************
    * 函数功能: SPI接收单字节
	* 输入参数: SPI_TypeDef* hspi 
	* 返 回 值: 接收内容
	* 说    明：
******************************************************/
uint8_t BSPSPIRcvOneData(SPI_TypeDef* hspi)
{
    uint8_t pData;
    HAL_StatusTypeDef result;
    result = HAL_SPI_Receive( &hspi1,  &pData,  1, 10);
    if(result != HAL_OK)
    {
        assert_param(0);                  //断言提示
    }
    return pData;//发送数据
}

/*****************************************************
    * 函数功能: SPI发送多字节
	* 输入参数: * hspi 数据：pData 长度：Size
    * 返 回 值: 发送结果 0为成功
	* 说    明：
******************************************************/
uint8_t	BSPSPISendNDatas(SPI_TypeDef* hspi , uint8_t *pData, uint16_t Size )
{
    HAL_StatusTypeDef result;
    result = HAL_SPI_Transmit(&hspi1,  pData,  Size,  10);
    if(result != HAL_OK)
    {
        assert_param(0);                  //断言提示
    }               
    return result;//发送数据
}

/*****************************************************
    * 函数功能: SPI接收多字节
	* 输入参数: * hspi 数据：pData 长度：Size
	* 返 回 值: 接收结果 0为成功
	* 说    明：
******************************************************/
uint8_t BSPSPIRecNDatas(SPI_TypeDef* hspi , uint8_t *pData, uint16_t Size )
{
    HAL_StatusTypeDef result;
    result = HAL_SPI_Receive( &hspi1,  pData,  Size, 10);
    if(result != HAL_OK)
    {
        assert_param(0);                  //断言提示
    }              
    return  result;//接收数据
}


