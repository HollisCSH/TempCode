/****************************************************************************/
/* 	File    	BSP_IWDG.c 			 
 * 	Author		Hollis
 *	Notes		ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�,ʹ��DEBUGģʽ�²�ʹ�ÿ��Ź�
 * 				
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_IWDG.h"
IWDG_HandleTypeDef hiwdg;

/**
  * ��������: �������Ź���ʼ������	(�����ڴ�����ֹͣģʽ�����У�)
  * �������: prv������[4,8,16,32,64,128,256]
  *            prv:Ԥ��Ƶ��ֵ��ȡֵ���£�
  *            ���� IWDG_PRESCALER_4: IWDG prescaler set to 4
  *            ���� IWDG_PRESCALER_8: IWDG prescaler set to 8
  *            ���� IWDG_PRESCALER_16: IWDG prescaler set to 16
  *            ���� IWDG_PRESCALER_32: IWDG prescaler set to 32
  *            ���� IWDG_PRESCALER_64: IWDG prescaler set to 64
  *            ���� IWDG_PRESCALER_128: IWDG prescaler set to 128
  *            ���� IWDG_PRESCALER_256: IWDG prescaler set to 256
  *
  *            rlv:Ԥ��Ƶ��ֵ��ȡֵ��ΧΪ��0-0XFFF
  * �� �� ֵ: ��
  * ˵    �����������þ�����Counter Reload Value = (LsiFreq(Hz) * Timeout(ms)) / (prescaler * 1000)   
  *           IWDG_Config(IWDG_Prescaler_32 ,1000);  // IWDG 1s ��ʱ��� =(32000 * 1000) / (32 * 1000)
  *			  ʱ�ӷǱ�׼32K  ���Լ���ֵֻ�Ǵ�Լֵ
  */
//  #define DEBUG
void MX_IWDG_Init(uint32_t rlv)
{
	#ifndef DEBUG
	hiwdg.Instance = IWDG;  
	hiwdg.Init.Prescaler = IWDG_PRESCALER_32;//����IWDG��Ƶϵ��   IWDG_PRESCALER_32
	hiwdg.Init.Reload = rlv;                 //��װ�� 
	hiwdg.Init.Window = IWDG_WINDOW_DISABLE; //�رմ��ڹ���

	HAL_IWDG_Init(&hiwdg);
	#endif
}	

void BSPIDogInit(uint32_t rlv)
{
	MX_IWDG_Init(rlv);
}

void  FeedIDog(void)
{
	#ifndef DEBUG
	HAL_IWDG_Refresh(&hiwdg);
	#endif
}
void BSPFeedWDogTask(void)	    //ι������ ����ԭ��  ʵ���õ���IDog
{
	FeedIDog();
}
void BSPIDogStart(void)
{
 	#ifndef DEBUG
	MX_IWDG_Init(4000);
	#endif
}


