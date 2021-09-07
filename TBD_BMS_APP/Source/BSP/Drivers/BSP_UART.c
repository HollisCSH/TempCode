/****************************************************************************/
/* 	File    	BSP_UART.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏	
 * 				端口复用AFx 需参考stm32g0b1re.pdf 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_UART.h"
#include "CanComm.h"
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
uint8_t aRxBuffer[2];
uint8_t gRxBuffer[256];
uint8_t RxFlag = 0;
UART_HandleTypeDef huart1;
UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef hlpuart2;

/* 扩展变量 ------------------------------------------------------------------*/

/* 函数声明 ------------------------------------------------------------------*/

/*****************************************************
    * 函数功能: UART端口、时钟、中断初始化
    * 输入参数: UART_HandleTypeDef uart类型
    * 返 回 值: 无
    * 说    明：该类函数在调用HAL_XX_DeInit 会内联
******************************************************/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	
  if(huart->Instance==UART_WIFI_DEBUG)
	{
        __HAL_RCC_LPUART1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PB10     	------> LPUART1_RX		
        PB11    	------> LPUART1_TX		
        */
        GPIO_InitStruct.Pin = UART_WIFI_TX_PIN|UART_WIFI_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_LPUART1;
        HAL_GPIO_Init(UART_WIFI_PORT, &GPIO_InitStruct);
        HAL_NVIC_SetPriority(UART_WIFI_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(UART_WIFI_IRQn);
	} 
    else if(huart->Instance==UART_GPS)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     	------> USART1_TX		PC4
        PA10     	------> USART1_RX		PC5
        */
        GPIO_InitStruct.Pin = UART_GPS_RX_PIN|UART_GPS_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
        HAL_GPIO_Init(UART_GPS_PORT, &GPIO_InitStruct);
        HAL_NVIC_SetPriority(UART_GPS_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(UART_GPS_IRQn);
    }
	else if(huart->Instance==LPUART2)
    {
        __HAL_RCC_LPUART2_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**LPUSART1 GPIO Configuration
        PC1     ------> LPUART2_RX
        PC0     ------> LPUART2_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF3_LPUART2;	//	GPIO_AF0_USART2	GPIO_AF1_LPUART2 GPIO_AF3_LPUART2
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        HAL_NVIC_SetPriority(USART2_LPUART2_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(USART2_LPUART2_IRQn);
    }
}

/*****************************************************
    * 函数功能: UART端口、时钟、中断 关使能
    * 输入参数: UART_HandleTypeDef uart类型
    * 返 回 值: 无
    * 说    明：该类函数在调用HAL_XX_DeInit 会内联
******************************************************/
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if(huart->Instance==UART_WIFI_DEBUG)
    {
        HAL_NVIC_DisableIRQ(UART_WIFI_IRQn);
        __HAL_RCC_LPUART1_CLK_DISABLE();
        /**USART1 GPIO Configuration
        PB10     	------> LPUART1_RX		
        PB11    	------> LPUART1_TX		
        */
        HAL_GPIO_DeInit(UART_WIFI_PORT, UART_WIFI_RX_PIN);
        HAL_GPIO_DeInit(UART_WIFI_PORT, UART_WIFI_TX_PIN);
    }
    else if(huart->Instance==UART_GPS)
    {
        HAL_NVIC_DisableIRQ(UART_GPS_IRQn);
        __HAL_RCC_USART1_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     	------> USART1_TX		PC4
        PA10     	------> USART1_RX		PC5
        */
        HAL_GPIO_DeInit(UART_GPS_PORT, UART_GPS_RX_PIN);
        HAL_GPIO_DeInit(UART_GPS_PORT, UART_GPS_TX_PIN);
    } 
}

/*****************************************************
	* 函数功能: USART1初始化
    * 输入参数: uint32_t baud 波特率
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void MX_USART1_UART_Init(uint32_t baud)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = baud;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    HAL_UART_Receive_IT(&huart1,(uint8_t *)aRxBuffer,1);	//开启接收中断
}

/*****************************************************
	* 函数功能: LPUSART1初始化
    * 输入参数: uint32_t baud 波特率
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void MX_LPUSART1_UART_Init(uint32_t baud)
{
    hlpuart1.Instance = LPUART1;
    hlpuart1.Init.BaudRate = baud;
    hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
    hlpuart1.Init.StopBits = UART_STOPBITS_1;
    hlpuart1.Init.Parity = UART_PARITY_NONE;
    hlpuart1.Init.Mode = UART_MODE_TX_RX;
    hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    hlpuart1.Init.OverSampling = UART_OVERSAMPLING_16;
    hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    hlpuart1.FifoMode = UART_FIFOMODE_DISABLE;
    if (HAL_UART_Init(&hlpuart1) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    HAL_UART_Receive_IT(&hlpuart1,(uint8_t *)aRxBuffer,1);
}
/*****************************************************
	* 函数功能: LPUSART2初始化
    * 输入参数: uint32_t baud 波特率
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void MX_LPUSART2_UART_Init(uint32_t baud)
{
    hlpuart2.Instance = LPUART2;
    hlpuart2.Init.BaudRate = baud;
    hlpuart2.Init.WordLength = UART_WORDLENGTH_8B;
    hlpuart2.Init.StopBits = UART_STOPBITS_1;
    hlpuart2.Init.Parity = UART_PARITY_NONE;
    hlpuart2.Init.Mode = UART_MODE_TX_RX;
    hlpuart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    hlpuart2.Init.OverSampling = UART_OVERSAMPLING_16;
    hlpuart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    hlpuart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    hlpuart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    hlpuart2.FifoMode = UART_FIFOMODE_DISABLE;
    if (HAL_UART_Init(&hlpuart2) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    if (HAL_UARTEx_DisableFifoMode(&hlpuart2) != HAL_OK)
    {
        assert_param(0);                  //断言 错误提示
    }
    HAL_UART_Receive_IT(&hlpuart2,(uint8_t *)aRxBuffer,1);
}

/*****************************************************
	* 函数功能: 串口初始化
    * 输入参数: USART_TypeDef 类型 uint32_t baud 波特率
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void BSPUART_Init(USART_TypeDef *UartHandle , uint32_t baud)
{
	if(UartHandle == USART1)
		MX_USART1_UART_Init(baud);
	else if(UartHandle == LPUART1)
		MX_LPUSART1_UART_Init(baud);
	else if(UartHandle == LPUART2)
		MX_LPUSART2_UART_Init(baud);
}

/*****************************************************
	* 函数功能: 串口关初始化
    * 输入参数: USART_TypeDef 类型
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void BSPUART_DeInit(USART_TypeDef *UartHandle)
{
	if(UartHandle == USART1)
		HAL_UART_DeInit(&huart1);
	else if(UartHandle == LPUART1)
		HAL_UART_DeInit(&hlpuart1);
}

/*****************************************************
	* 函数功能: 接收完成中断处理
    * 输入参数: UART_HandleTypeDef *UartHandle 串口类型
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    
    if(UartHandle->Instance == USART1)
    {
        #ifdef BMS_ENABLE_NB_IOT
        extern void Sim_RxByte(uint8 data);
        Sim_RxByte(aRxBuffer[0]);
        #endif        
        
    }
    else if(UartHandle->Instance == LPUART1)
    {
        #ifdef BMS_ENABLE_NB_IOT
        extern void WifiRxByte(uint8 data);
        WifiRxByte(aRxBuffer[0]);
        #endif
        
    }
    else if(UartHandle->Instance == LPUART2)
    {
        
    }

    while(HAL_UART_Receive_IT(UartHandle,(uint8_t *)aRxBuffer,1) != HAL_OK);
}

/*****************************************************
    * 函数功能: 串口发送完成回调函数
    * 输入参数: UART_HandleTypeDef uart类型
    * 返 回 值: 无
    * 说    明：有各类发送回调可使用
******************************************************/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  //
}

/*****************************************************
    * 函数功能: Wifi数据发送函数
    * 输入参数: 数据指针 *Data , 数据长度 DataLen
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void UART_SEND_WIFI_DEBUG(uint8_t *Data , uint8_t DataLen)
{
  //HAL_UART_Transmit(&hlpuart1, (uint8_t *)&ch,1, 0xFFFF);
    HAL_UART_Transmit_IT(&hlpuart1,Data,DataLen);
}

/*****************************************************
    * 函数功能: Gps数据发送函数
    * 输入参数: 数据指针 *Data , 数据长度 DataLen
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void UART_SEND_GPS(uint8_t *Data , uint8_t DataLen)
{
  //HAL_UART_Transmit(&hlpuart1, (uint8_t *)&ch,1, 0xFFFF);
  HAL_UART_Transmit_IT(&huart1,Data,DataLen);
}

/*****************************************************
    * 函数功能: UART数据发送函数
    * 输入参数: 串口号 *UartHandle 数据指针 *Data , 数据长度 DataLen
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void LPUART_DRV_SendData(USART_TypeDef *UartHandle , uint8_t *Data , uint8_t DataLen)
{
  if(UartHandle == 	UART_WIFI_DEBUG)
		UART_SEND_WIFI_DEBUG( Data ,  DataLen);
	else if(UartHandle == UART_GPS)
		UART_SEND_GPS( Data ,  DataLen);
}

/*****************************************************
    * 函数功能: 打印函数printf重映射   可选具体打印口
    * 输入参数: 
    * 返 回 值: 无
    * 说    明：无
******************************************************/
int fputc(int ch, FILE *f) 
{
//    HAL_UART_Transmit(&hlpuart1, (uint8_t *)&ch,1, 0xFFFF);
    #ifdef BMS_ENABLE_NB_IOT
		HAL_UART_Transmit(&hlpuart2, (uint8_t *)&ch,1, 0xFFFF); //使用NB时，使用临时调试串口
    #else
		HAL_UART_Transmit(&hlpuart1, (uint8_t *)&ch,1, 0xFFFF); //没有使用NB时，使用wifi调试串口
    #endif
    return ch;
}

//void Error_Handler(void)
//{
//  /* USER CODE BEGIN Error_Handler_Debug */
//  /* User can add his own implementation to report the HAL error return state */
//  __disable_irq();	//	__enable_irq();
//  while (1)
//  {
//		_UN_NB_Printf("Error_Handler \r\n"); 
//		HAL_Delay(100);
//  }
//  /* USER CODE END Error_Handler_Debug */
//}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None  将底层错误打印出来 
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
	_UN_NB_Printf("Wrong parameters value: file %s on line %d\r\n", file, line);
}
#endif /* USE_FULL_ASSERT */
