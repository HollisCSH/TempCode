/****************************************************************************/
/* 	File    	BSP_EXIT.c 			 
 * 	Author		Hollis
 *	Notes		ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
 * 				���ⲿ�ж�ʱ  ��Ҫע���жϴ�����ʽ  ԭʼ��ƽ  ���������Ÿ����Ƿ��г�ͻ
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_EXIT.h"

/* ˽�к궨�� ----------------------------------------------------------------*/
static uint8_t g_ID_flag = 0;	//����ģ������־   ���������ж���1

/********************************************************************
	* ��������: IO�жϳ�ʼ��
	* �������: ��
	* �� �� ֵ: ��
	* ˵    �����ж϶�Ӧ�ĺ������λ�� HAL_GPIO_EXTI_Falling_Callback
********************************************************************/
void BSPExitInit()
{
	GPIO_InitTypeDef gpioinitstruct;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	gpioinitstruct.Pin = MODULE_INT_PIN;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpioinitstruct.Mode = GPIO_MODE_IT_FALLING;		//�½��ش���  GPIO_MODE_IT_FALLING  GPIO_MODE_IT_RISING
	HAL_GPIO_Init(MODULE_INT_PORT, &gpioinitstruct);
	
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x03, 0x00);
//	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);		//˯��ǰ����  ��ʱ�ر�
}

//=============================================================================================
//��������	: void HAL_GPIO_EXTI_Falling_Callback(void)
//��������	: �����ж���
//�������	: void
//��̬����	: void
//��    ��	: �½����ж�
//ע    ��	: ϵͳ���� �� stm32g0xx_it.c �е��жϺ�������
//=============================================================================================
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MODULE_INT_PIN)
    {
        g_ID_flag = 1;
    }
}

/********************************************************************
	* ��������: ���� �����豸���� ״̬
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ����
********************************************************************/
void SET_MODU_STA(uint8_t flag)
{
	g_ID_flag = flag;
}

/********************************************************************
	* ��������: ��ȡ �����豸���� ״̬
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ����
********************************************************************/
uint8_t GET_MODU_INT_STA(void)
{
	return g_ID_flag;
}
