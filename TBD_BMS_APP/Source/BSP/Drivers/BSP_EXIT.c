/****************************************************************************/
/* 	File    	BSP_EXIT.c 			 
 * 	Author		Hollis
 *	Notes		ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
 * 				ʹ���ⲿ�ж�ʱ  ��Ҫע���жϴ�����ʽ  ԭʼ��ƽ  ���������Ÿ����Ƿ��г�ͻ
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_EXIT.h"
#include "BSP_GPIO.h"
#include "IOCheck.h"

/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/

/*****************************************************
	* ��������: IO�жϳ�ʼ��
	* �������: ��
	* �� �� ֵ: ��
	* ˵    �����ж϶�Ӧ�ĺ������λ���ⲿ
******************************************************/
void BSP_EXIT_Init()
{
	GPIO_InitTypeDef gpioinitstruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	/**************************************************/
	gpioinitstruct.Pin = ALARM_AFE_PIN|REMOVE_INT_PIN|MODULE_INT_PIN;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpioinitstruct.Mode = GPIO_MODE_IT_FALLING;		//�½��ش���	
	HAL_GPIO_Init(GPIOA, &gpioinitstruct);
	
//	gpioinitstruct.Pin = MODULE_SHORT_PIN;	//����ģ������ж�
//	gpioinitstruct.Pull = GPIO_NOPULL;
//	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//	gpioinitstruct.Mode = GPIO_MODE_IT_RISING;		//�����ش���	
//	HAL_GPIO_Init(GPIOA, &gpioinitstruct);	
	/**************************************************/
	gpioinitstruct.Pin = ACC_INT1_PIN;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpioinitstruct.Mode = GPIO_MODE_IT_RISING;		//�����ش���
	HAL_GPIO_Init(GPIOB, &gpioinitstruct);
	/**************************************************/
	gpioinitstruct.Pin = INT_4G_PIN ;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpioinitstruct.Mode = GPIO_MODE_IT_FALLING;		//�½��ش���
	HAL_GPIO_Init(GPIOC, &gpioinitstruct);
	
	gpioinitstruct.Pin = ACC_INT2_PIN;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpioinitstruct.Mode = GPIO_MODE_IT_RISING;		//�����ش���
	HAL_GPIO_Init(ACC_INT2_PORT, &gpioinitstruct);
	/**************************************************/
	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0x03, 0);
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
	HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0x03, 0);
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x03, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}
