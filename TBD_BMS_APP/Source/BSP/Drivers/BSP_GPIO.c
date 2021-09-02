/****************************************************************************/
/* 	File    	BSP_GPIO.c 			 
 * 	Author		Hollis
 *	Notes		ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
 * 				
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
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
	
	__HAL_RCC_GPIOA_CLK_ENABLE();	//�˿�A��ʼ��
	gpioinitstruct.Pin = CAN_5V_PIN | CTL_AFE_PIN | SH367309_ON_PIN | VCC_4G_CTL_PIN | FLASH_CS_PIN | PRE_DSG_EN_PIN;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &gpioinitstruct);
	
	__HAL_RCC_GPIOB_CLK_ENABLE();	//�˿�B��ʼ��
	gpioinitstruct.Pin = PRE_DSG_NTC_PIN | MOS_TVS_NTC_PIN ;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &gpioinitstruct);
	
	__HAL_RCC_GPIOC_CLK_ENABLE();	//�˿�C��ʼ��
	gpioinitstruct.Pin = FLASH_POW_EN_PIN | VPRO_AFE_PIN | PWR_EN_14V_5V_PIN ;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOC, &gpioinitstruct);
	
	__HAL_RCC_GPIOD_CLK_ENABLE();	//�˿�D��ʼ��
	gpioinitstruct.Pin = DSG_EN_PIN | FUSE_DET_EN_PIN | VCC_MODULE_PIN;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	
	HAL_GPIO_Init(GPIOD, &gpioinitstruct);
  
	GPIO_DefaultInit();				//�˿������ʼֵ����
	////////////////////////////////����Ϊ�������  ����Ϊ��������
	gpioinitstruct.Pin = MODULE_SHORT_PIN ;
	gpioinitstruct.Mode = GPIO_MODE_INPUT;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	
	HAL_GPIO_Init(GPIOA, &gpioinitstruct);
  
	gpioinitstruct.Pin = HW_REV0_PIN | HW_REV1_PIN | FUSE_DET_PIN ;
	gpioinitstruct.Mode = GPIO_MODE_INPUT;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	
	HAL_GPIO_Init(GPIOD, &gpioinitstruct);
	////////////////////////////////�ж�����
	BSP_EXIT_Init();				//�ⲿ�ж�
}
/*****************************************************
	* ��������: GPIO�˿� �ض�
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void BSPGPIODeInit(void)
{
//	__HAL_RCC_GPIOA_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOA, CAN_5V_PIN | CTL_AFE_PIN | SH367309_ON_PIN | VCC_4G_CTL_PIN | PRE_DSG_EN_PIN | FLASH_CS_PIN );

//	__HAL_RCC_GPIOB_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOB, PRE_DSG_NTC_PIN | MOS_TVS_NTC_PIN );
	
//	__HAL_RCC_GPIOC_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOC, FLASH_POW_EN_PIN | VPRO_AFE_PIN | PWR_EN_14V_5V_PIN );
	
//	__HAL_RCC_GPIOD_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOD, DSG_EN_PIN | FUSE_DET_EN_PIN | VCC_MODULE_PIN );
}

/*****************************************************
	* ��������: GPIO�˿� Ĭ��״̬
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void GPIO_DefaultInit(void)
{
	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_RESET);					//Ĭ��ֵ �͵�ƽ CANͨѶԭ�ߵ�Դ����ʹ��	�ߵ�ƽ��Ч
	HAL_GPIO_WritePin(CTL_AFE_PORT, CTL_AFE_PIN, GPIO_PIN_RESET);				//Ĭ��ֵ �͵�ƽ ����֮��ǿ�ƹرճ�ŵ�MOS
	HAL_GPIO_WritePin(SH367309_ON_PORT, SH367309_ON_PIN, GPIO_PIN_SET);		  	//Ĭ��ֵ �ߵ�ƽ Ĭ�����ߣ�����֮��AFE�������ģʽ������2uA��
	HAL_GPIO_WritePin(VCC_4G_CTL_PORT, VCC_4G_CTL_PIN, GPIO_PIN_RESET);			//Ĭ��ֵ �͵�ƽ ���ߺ���ڲ�ģ�鹩�磬���ͺ�رո��ڲ�ģ��Ĺ���
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);				//Ĭ��ֵ �͵�ƽ ͨѶ֮ǰ������  ����������
	
	HAL_GPIO_WritePin(PRE_DSG_NTC_PORT, PRE_DSG_NTC_PIN, GPIO_PIN_SET);		  	//Ĭ��ֵ �ߵ�ƽ ��Ԥ��ʱ����
	HAL_GPIO_WritePin(MOS_TVS_NTC_PORT, MOS_TVS_NTC_PIN, GPIO_PIN_SET);		  	//Ĭ��ֵ �ߵ�ƽ TVS��MOS�˵�NTC�����ƣ�������Ч
	
	HAL_GPIO_WritePin(FLASH_POW_EN_PORT, FLASH_POW_EN_PIN, GPIO_PIN_SET);	  	//Ĭ��ֵ �ߵ�ƽ Flash���磬����дFlashʱĬ������ߵ�ƽ ������
	HAL_GPIO_WritePin(VPRO_AFE_PORT, VPRO_AFE_PIN, GPIO_PIN_RESET);				//Ĭ��ֵ �͵�ƽ Ĭ�����ͣ���AFEдEEPROM����ʱ��������
	HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_RESET);	//Ĭ��ֵ �͵�ƽ MCU���Ѻ��������ߣ�������ǰ��ʱ100mS����
	
	HAL_GPIO_WritePin(DSG_EN_PORT, DSG_EN_PIN, GPIO_PIN_RESET);					//Ĭ��ֵ �͵�ƽ Flash���磬�ŵ���������ٿ���++++++++++++++++++++++++++δȷ��
	HAL_GPIO_WritePin(FUSE_DET_EN_PORT, FUSE_DET_EN_PIN, GPIO_PIN_RESET);		//Ĭ��ֵ �͵�ƽ Ĭ�����ͣ����FUSE״̬ʱ����
	HAL_GPIO_WritePin(VCC_MODULE_PORT, VCC_MODULE_PIN, GPIO_PIN_RESET);			//Ĭ��ֵ �͵�ƽ �ⲿ4Gͨ�Ź���,����P+��ѹ���ߵ�ƽʹ��------------------
	HAL_GPIO_WritePin(PRE_DSG_EN_PORT, PRE_DSG_EN_PIN, GPIO_PIN_RESET);			//Ĭ��ֵ �͵�ƽ Ԥ�ŵ翪�أ�Ĭ������
	
}
/*****************************************************
	* ��������: ʹ��CANͨѶоƬ ����
	* �������: ��
	* �� �� ֵ: ��
	* ˵    �����ܽ���������ʵ�ʿ���Ϊ3.3V
******************************************************/
void CAN_3V_ENABLE(void)
{
	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_SET);
}

/*****************************************************
	* ��������: ��ֹCANͨѶоƬ �ϵ�
	* �������: ��
	* �� �� ֵ: ��
	* ˵    �����ܽ���������ʵ�ʿ���Ϊ3.3V
******************************************************/
void CAN_3V_DISABLE(void)
{
	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_RESET);
}
/*****************************************************
	* ��������: ʹ��14V��Դ���
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void PWR_14V_5V_ENABLE(void)
{
	HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_SET);
}

/*****************************************************
	* ��������: ��ֹ14V��Դ���
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void PWR_14V_5V_DISABLE(void)
{
	HAL_GPIO_WritePin(PWR_EN_14V_5V_PORT, PWR_EN_14V_5V_PIN, GPIO_PIN_RESET);
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
/*****************************************************
	* ��������: AFE������дʹ�ܿ������Ĭ�����ͣ�
				��AFEдEEPROM����ʱ��������	��ȡSH367309�Ĳ�������
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void VPRO_AFE_HI(void)
{
//	HAL_GPIO_WritePin(VPRO_AFE_PORT, VPRO_AFE_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VPRO_AFE_PORT, VPRO_AFE_PIN, GPIO_PIN_SET);
}
void VPRO_AFE_LOW(void)
{
//	HAL_GPIO_WritePin(VPRO_AFE_PORT, VPRO_AFE_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(VPRO_AFE_PORT, VPRO_AFE_PIN, GPIO_PIN_RESET);
}
/////////////////////////////////////////////////////////////////
void SH367309_ON_ENABLE(void)
{
	HAL_GPIO_WritePin(SH367309_ON_PORT, SH367309_ON_PIN, GPIO_PIN_SET);
}

void SH367309_ON_DISABLE(void)	//�˲�ģʽ
{
	HAL_GPIO_WritePin(SH367309_ON_PORT, SH367309_ON_PIN, GPIO_PIN_RESET);
}


/*****************************************************
  * ��������: ����IOΪ����״̬
  * �������: ����˿ں�
  * �� �� ֵ: ��
  * ˵    ������
******************************************************/
void GPIO_SET_INPUT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef gpioinitstruct;
    gpioinitstruct.Pin = GPIO_Pin ;
    gpioinitstruct.Mode = GPIO_MODE_INPUT;
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	
    HAL_GPIO_Init(GPIOx, &gpioinitstruct);
}
/*****************************************************
  * ��������: ����IOΪ�������ж�
  * �������: ����˿ں�
  * �� �� ֵ: ��
  * ˵    ������
******************************************************/
void GPIO_SET_RISE_EXIT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef gpioinitstruct;
    gpioinitstruct.Pin = GPIO_Pin;	                //����ģ������ж�
    gpioinitstruct.Pull = GPIO_NOPULL;
    gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpioinitstruct.Mode = GPIO_MODE_IT_RISING;		//�����ش���	
    HAL_GPIO_Init(GPIOx, &gpioinitstruct);
    
    if((GPIO_Pin == GPIO_PIN_0)||(GPIO_Pin == GPIO_PIN_1))
    {
        HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0x03, 0);
        HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);       
    }
    else if((GPIO_Pin == GPIO_PIN_2)||(GPIO_Pin == GPIO_PIN_3))
    {
        HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0x03, 0);
        HAL_NVIC_EnableIRQ(EXTI2_3_IRQn); 
    }
    else
    {
        HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x03, 0);
        HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);    
    }
}

