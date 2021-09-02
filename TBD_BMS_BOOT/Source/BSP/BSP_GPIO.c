/****************************************************************************/
/* 	File    	BSP_GPIO.c 			 
 * 	Author		Hollis
 *	Notes			ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_GPIO.h"
#include "BSP_EXIT.h"

/*****************************************************
	* ��������: GPIO�˿� ʱ�� �жϳ�ʼ��
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void BSPGPIOInit(void)
{
	GPIO_InitTypeDef  gpioinitstruct;
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	gpioinitstruct.Pin = PWR_EN_PIN;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(PWR_EN_PORT, &gpioinitstruct);
	
	gpioinitstruct.Pin = CAN_5V_PIN;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(CAN_5V_PORT, &gpioinitstruct);
	
    BSPExitInit();									//�ⲿ�ж�	
}

/*****************************************************
	* ��������: GPIO�˿� �ض�
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void BSPGPIODeInit(void)
{
    HAL_GPIO_DeInit(PWR_EN_PORT , PWR_EN_PIN);
    HAL_GPIO_DeInit(CAN_5V_PORT , CAN_5V_PIN);
}

/*****************************************************
	* ��������: ʹ��CANͨѶоƬ ����
    * �������: ��
    * �� �� ֵ: ��
    * ˵    ������
******************************************************/
void CAN_POWER_ENABLE(void)
{
	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PWR_EN_PORT, PWR_EN_PIN, GPIO_PIN_SET);
}

/*****************************************************
	* ��������: ��ֹCANͨѶоƬ �ϵ�
    * �������: ��
    * �� �� ֵ: ��
    * ˵    ������
******************************************************/
void CAN_POWER_DISABLE(void)
{
	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PWR_EN_PORT, PWR_EN_PIN, GPIO_PIN_RESET);
}

/*****************************************************
	* ��������: ʹ��14V��Դ���
    * �������: ��
    * �� �� ֵ: ��
    * ˵    ������
******************************************************/
void PWR_EN_ENABLE(void)
{
	HAL_GPIO_WritePin(PWR_EN_PORT, PWR_EN_PIN, GPIO_PIN_SET);
}

/*****************************************************
	* ��������: ��ֹ14V��Դ���
    * �������: ��
    * �� �� ֵ: ��
    * ˵    ������
******************************************************/
void PWR_EN_DISABLE(void)
{
	HAL_GPIO_WritePin(PWR_EN_PORT, PWR_EN_PIN, GPIO_PIN_RESET);
}
/*****************************************************
	* ��������: ʹ��FLASHͨѶоƬ ����
    * �������: ��
    * �� �� ֵ: ��
    * ˵    ������
******************************************************/
void FLASH_POWER_ENABLE(void)
{
	HAL_GPIO_WritePin(FLASH_POW_EN_PORT, FLASH_POW_EN_PIN, GPIO_PIN_RESET);	//�͵�ƽ��ͨ��
}

/*****************************************************
	* ��������: ��ֹFLASHͨѶоƬ �ϵ�
    * �������: ��
    * �� �� ֵ: ��
    * ˵    ������
******************************************************/
void FLASH_POWER_DISABLE(void)
{
	HAL_GPIO_WritePin(FLASH_POW_EN_PORT, FLASH_POW_EN_PIN, GPIO_PIN_SET);
}

/*****************************************************
	* ��������: ʹ��FLASHоƬ ͨѶ
    * �������: ��
    * �� �� ֵ: ��
    * ˵    ������
******************************************************/
void FLASH_CS_ENABLE(void)
{
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);
}

/*****************************************************
	* ��������: ��ֹFLASHоƬ ͨѶ
    * �������: ��
    * �� �� ֵ: ��
    * ˵    ������
******************************************************/
void FLASH_CS_DISABLE(void)
{
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);
}

