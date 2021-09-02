/****************************************************************************/
/* 	File    	BSP_SPI.c 			 
 * 	Author		Hollis
 *	Notes		ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
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
//static const uint32_t SPI_PrescalerIndex[] = 		//�������ݳ���ת��
//{
//	SPI_BAUDRATEPRESCALER_2,SPI_BAUDRATEPRESCALER_4,SPI_BAUDRATEPRESCALER_8,SPI_BAUDRATEPRESCALER_16,
//	SPI_BAUDRATEPRESCALER_32,SPI_BAUDRATEPRESCALER_64,SPI_BAUDRATEPRESCALER_128,SPI_BAUDRATEPRESCALER_256
//};
/*****************************************************
	* ��������: SPI1�������� 
	* �������: void
	* �� �� ֵ: void
	* ˵    ������Ƶϵ������SPIƵ��    �趨4M    24M����Ϊ6M
******************************************************/
void MX_SPI1_Init(void)
{
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;						//��ģʽ
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;			//ȫ˫��
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;				//����λΪ8λ
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;				//CPOL=0,low
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;							//�������NSS 
//	hspi1.Init.BaudRatePrescaler = SPI_PrescalerIndex[SPI_BaudRatePrescaler-3];	//������   Ϊϵͳʱ�ӷ�Ƶ
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;	//������   Ϊϵͳʱ�ӷ�Ƶ
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	hspi1.State = HAL_SPI_STATE_RESET;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		assert_param(0);                  					//���� ������ʾ
	}
}

/*****************************************************
	* ��������: SPI��ʼ��
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ����Ϊ�����ⲿ�������
******************************************************/
void BSPSPIInit(void)
{
	MX_SPI1_Init();
}

/*****************************************************
	* ��������: SPI �ض�
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ����Ϊ�����ⲿ�������
******************************************************/
void BSPSPIDisable(void)
{
	HAL_SPI_MspDeInit(&hspi1);
}

/*****************************************************
	* ��������: SPI ʱ�� �˿ڳ�ʼ��
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ����Ϊ�����ⲿ�������
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

        __HAL_RCC_GPIOA_CLK_ENABLE();	//�˿�A��ʼ��
        GPIO_InitStruct.Pin = FLASH_CS_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/*****************************************************
	* ��������: SPI ʱ�� �˿� �ض�
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ����Ϊ�����ⲿ�������
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
    * ��������: SPI����
	* �������: *pData ����ָ�� Size���ݳ��� Timeout��ʱ
	* �� �� ֵ: ��
	* ˵    ���������ͣ�������߳�ʱ����
******************************************************/
uint8_t SPI_Transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	return HAL_SPI_Transmit(&hspi1,  pData,  Size,  Timeout);//��������
//	return HAL_SPI_Transmit_IT(&hspi1,  pData,  Size);//��������
}

/*****************************************************
    * ��������: SPI����
	* �������: *pData ����ָ�� Size���ݳ��� Timeout��ʱ
	* �� �� ֵ: ��
	* ˵    ���������ͣ����ջ��߳�ʱ����
******************************************************/
uint8_t SPI_Receive(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	return HAL_SPI_Receive( &hspi1,  pData,  Size, Timeout);//��������
//	return HAL_SPI_Receive_IT( &hspi1,  pData,  Size);//��������
}

//uint8_t SPI_TransmitReceive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size , uint32_t Timeout)
//{
//	return HAL_SPI_TransmitReceive(&hspi1,  pTxData,  pRxData,  Size, Timeout);//�շ�����
////HAL_StatusTypeDef HAL_SPI_TransmitReceive_IT(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
//}

//void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
//{

//}
//void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
//{

//}
/*****************************************************
    * ��������: SPI���͵��ֽ�
	* �������: SPI_TypeDef* hspi ���ݣ�pData
	* �� �� ֵ: ��
	* ˵    ����
******************************************************/
uint8_t BSPSPISendOneData(SPI_TypeDef* hspi , uint8_t pData)
{
    HAL_StatusTypeDef result;
    result = HAL_SPI_Transmit(&hspi1,  &pData,  1,  10);
    if(result != HAL_OK)
    {
        assert_param(0);                  //������ʾ
    }
    return result;//��������
}

/*****************************************************
    * ��������: SPI���յ��ֽ�
	* �������: SPI_TypeDef* hspi 
	* �� �� ֵ: ��������
	* ˵    ����
******************************************************/
uint8_t BSPSPIRcvOneData(SPI_TypeDef* hspi)
{
    uint8_t pData;
    HAL_StatusTypeDef result;
    result = HAL_SPI_Receive( &hspi1,  &pData,  1, 10);
    if(result != HAL_OK)
    {
        assert_param(0);                  //������ʾ
    }
    return pData;//��������
}

/*****************************************************
    * ��������: SPI���Ͷ��ֽ�
	* �������: * hspi ���ݣ�pData ���ȣ�Size
    * �� �� ֵ: ���ͽ�� 0Ϊ�ɹ�
	* ˵    ����
******************************************************/
uint8_t	BSPSPISendNDatas(SPI_TypeDef* hspi , uint8_t *pData, uint16_t Size )
{
    HAL_StatusTypeDef result;
    result = HAL_SPI_Transmit(&hspi1,  pData,  Size,  10);
    if(result != HAL_OK)
    {
        assert_param(0);                  //������ʾ
    }               
    return result;//��������
}

/*****************************************************
    * ��������: SPI���ն��ֽ�
	* �������: * hspi ���ݣ�pData ���ȣ�Size
	* �� �� ֵ: ���ս�� 0Ϊ�ɹ�
	* ˵    ����
******************************************************/
uint8_t BSPSPIRecNDatas(SPI_TypeDef* hspi , uint8_t *pData, uint16_t Size )
{
    HAL_StatusTypeDef result;
    result = HAL_SPI_Receive( &hspi1,  pData,  Size, 10);
    if(result != HAL_OK)
    {
        assert_param(0);                  //������ʾ
    }              
    return  result;//��������
}


