/****************************************************************************/
/* 	File    	BSP_ADC.c 			 
 * 	Author		Hollis
 *	Notes		ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
 * 
 *	Version		Date    	Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_ADC.h"
#include "Sample.h"
#include "stdio.h"

/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
static uint8_t 	sADCBusyStat = 0;	                    //ADC���� æ��־��0�����У�1��æ
static uint8_t 	sADCWorkingChannel = 0;                 //ADC���ڹ�����ͨ����
static uint16_t sADCSampleData[eADC_ChanNum] = {0};  	//ADC��ͨ������ֵ
/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/

/*****************************************************
	* ��������: ADC�˿ڡ�ʱ�ӡ��жϳ�ʼ��
	* �������: ADC_HandleTypeDef ����
	* �� �� ֵ: ��
	* ˵    �������ຯ���ڵ���HAL_XX_Init ������
******************************************************/
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
		GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(NTC_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = PRE_DSG_SHORT_DET_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(PRE_DSG_SHORT_DET_PORT, &GPIO_InitStruct);
			
		HAL_NVIC_SetPriority(ADC1_COMP_IRQn, 0x03, 0x00);
		HAL_NVIC_EnableIRQ(ADC1_COMP_IRQn);
	}
}
/*****************************************************
	* ��������: ADC�˿ڡ�ʱ�ӡ��ж� �ض�
	* �������: ADC_HandleTypeDef ����
	* �� �� ֵ: ��
	* ˵    �������ຯ���ڵ���HAL_XX_DeInit ������
******************************************************/
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
	if(hadc->Instance==ADC1)
	{
		__HAL_RCC_ADC_CLK_DISABLE();
		/**ADC1 GPIO Configuration
		PA5     ------> ADC1_IN5
		PA6     ------> ADC1_IN6
		PA7     ------> ADC1_IN7
		PB0     ------> ADC1_IN8	*/   
		HAL_GPIO_DeInit(GPIOA, TVS_NTC_PIN|MOS_NTC_PIN|PRE_NTC_PIN);
		HAL_GPIO_DeInit(GPIOB, PRE_DSG_SHORT_DET_PIN);
	}
}
/*****************************************************
	* ��������: ADC1��������
	* �������: void
	* �� �� ֵ: void
	* ˵    ����
******************************************************/
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
        assert_param(0);                  //���� ������ʾ
    }
}

/*****************************************************
	* ��������: ADC��ʼ��
	* �������: ADC_HandleTypeDef can����
	* �� �� ֵ: ��
	* ˵    ����Ϊ�����ⲿ�������
******************************************************/
void BSPADCInit(void)
{
	MX_ADC1_Init();
}

/*****************************************************
	* ��������: ADC�ض�
	* �������: ADC_HandleTypeDef can����
	* �� �� ֵ: ��
	* ˵    ����Ϊ�����ⲿ�������
******************************************************/
void BSPADCDisable(void)
{
	HAL_ADC_DeInit(&hadc1);
}

/*****************************************************
	* ��������: ��ȡADCת��ֵ
	* �������: ת��ͨ�� ch ת������ times
	* �� �� ֵ: ��
	* ˵    ����
******************************************************/
uint32_t ADC_Get_Average(uint8_t ch,uint8_t times)
{
	ADC_ChannelConfTypeDef sConfig;		//ͨ����ʼ��
	uint32_t value_sum=0;	
	uint8_t i;
	switch(ch)							//ѡ��ADCͨ��
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
	sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;		//��������239.5����
	sConfig.Rank = ADC_REGULAR_RANK_1;
	HAL_ADC_ConfigChannel(&hadc1,&sConfig);											
	for(i=0;i<times;i++)
	{
		HAL_ADC_Start(&hadc1);								//����ת��
//		HAL_ADC_PollForConversion(&hadc1,10);				//�ȴ�ת������
		if(HAL_ADC_PollForConversion(&hadc1,10) == HAL_OK)
		{
			value_sum += HAL_ADC_GetValue(&hadc1);			//���					
			HAL_ADC_Stop(&hadc1);							//ֹͣת��
		}
	}
	return value_sum/times;									//����ƽ��ֵ
}

////////////////////////////////////////////////////////////////////////////////
//=============================================================================================
//��������	: void BSPADCStartSample(u8 channel)
//�������	: channel:0-15;
//�������	: void
//��̬����	: void
//��		��	: ADCģ�鿪ʼ����
//ע		��	:
//=============================================================================================
void BSPADCStartSample(uint8_t channel)
{
	ADC_ChannelConfTypeDef sConfig;		//ͨ����ʼ��
	sADCBusyStat = 1;	//���Ϊæ״̬
	sADCWorkingChannel = channel;
	switch(channel)							//ѡ��ADCͨ��
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
		default:break;
	}
	sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;		//��������239.5����
	sConfig.Rank = ADC_REGULAR_RANK_1;
	HAL_ADC_ConfigChannel(&hadc1,&sConfig);			
	HAL_ADC_Start_IT(&hadc1);								//����ת��
 
}
//=============================================================================
//��������: u16 BSPADCCalADtoVolt(u16 ad)
//�������: ת��adֵΪ��ѹֵ
//�������: ת����ĵ�ѹֵ����λmV
//��������: 
//ע������:
//=============================================================================
static uint16_t BSP_ADC_0V_OFFSET = 0;
float BSPADCCalADtoVolt(uint16_t ad)
{
    uint16_t adcMax;
    uint16_t u16adval;
    float adcValue;    
    /* Get ADC max value from the resolution */  
    adcMax = (uint16_t) (1 << 12);      			//12λ����
    /* Process the result to get the value in volts */
    ad = ad>BSP_ADC_0V_OFFSET?ad-BSP_ADC_0V_OFFSET:0;   //������Ʈ
    adcMax = adcMax - BSP_ADC_0V_OFFSET;                //������Ʈ
    adcValue = ((float) ad / adcMax) * (BSP_ADC_VREFH - BSP_ADC_VREFL);    
    u16adval = adcValue * 1000;
    
    return u16adval;   
}
//=============================================================================
//��������: u16 BSPADCCalADtoRealVolt(u16 ad,u16 mcu3v3)
//�������: ת��adֵΪ��ʵ��ѹֵ
//�������: ת����ĵ�ѹֵ����λmV
//��������: ����ʵ�ʲ�����3.3V��ѹת��AD��ѹֵ
//ע������: 
//=============================================================================
uint16_t BSPADCCalADtoRealVolt(uint16_t ad,uint16_t mcu3v3)
{
    uint16_t adcMax;
    uint16_t u16adval;   
    float adcValue;    
    float adc3v3;
    
    /* Get ADC max value from the resolution */
  
    adcMax = (uint16_t) (1 << 12);
    
    if(mcu3v3 > 0)    
    {
        adc3v3 = ((float)mcu3v3 / 1000);
    }
    else
    {
        adc3v3 = BSP_ADC_VREFH - BSP_ADC_VREFL;
    }
    /* Process the result to get the value in volts */
    adcValue = ((float) ad / adcMax) * (adc3v3);    
    u16adval = adcValue * 1000;
    
    return u16adval;
//	volt = per * gSampleData.MCU3v3 / 1000;
}
//=============================================================================================
//��������	: u16 ADCGetSampleData(u8 channel)
//�������	: channel:ADCͨ����
//�������	: ����ֵ
//��������	: ��ȡ����ADֵ
//ע������	:
//=============================================================================================
uint16_t ADCGetSampleData(uint8_t channel)
{
	uint16_t data = 0;

	if(channel >= eADC_ChanNum)
	{
		return(0xffff);
	}

	data = sADCSampleData[channel];

    return(data);
}
//=============================================================================================
//��������	: u8 BSPADCGetWorkState(void)
//�������	: void
//�������	: ADC����״̬ 1:æ 0:����
//��������	: ��ȡADC����״̬
//ע������	:
//=============================================================================================
uint8_t BSPADCGetWorkState(void)
{
	return (sADCBusyStat);
}
//=============================================================================================
//��������	: u16 BSPADCDataAvgFilter(u16 *data, u16 len)
//�������	: data:�˲����ݴ� len:�˲����ݴ�����
//�������	: �˲��������ֵ
//��������	: ����ƽ��ֵ�˲�
//ע������	: �������Сֵ
//=============================================================================================
u16 BSPADCDataAvgFilter(u16 *data, u16 len)
{
	u8 i = 0;
	u16 max = 0;
	u16 min = 0;
	u32 sum = 0;
	u16 result = 0;

	if(0 == len)                //���ȴ���
	{
	    result = 0xffff;
	}
	else if(len < 3)            //ֻ��1������2����
	{
		sum = data[0] + data[len - 1];
		result = (u16)(sum / 2);
	}
	else                        //������3����
	{
    	max = data[0];
    	min = data[0];
    	sum = data[0];

    	for(i = 1; i < len; i++)
    	{
    		if(data[i] > max)   //�����ֵ
    		{
    			max = data[i];
    		}

    		if(data[i] < min)	//����Сֵ
    		{
    			min = data[i];
    		}

    		sum += data[i];	    //���
    	}

    	result = (u16)((sum - max - min) / (len - 2));
   }

	return(result);
}
/*****************************************************
	* ��������: ADCת����ɻص�����
	* �������: ADC_HandleTypeDef����
	* �� �� ֵ: void
	* ˵    ����ϵͳ���� ��stm32g0xx_it.c���жϺ�������
******************************************************/
void  HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if( hadc->Instance == ADC1 )
	{
		sADCSampleData[sADCWorkingChannel] = HAL_ADC_GetValue(hadc);
        if( (BSP_ADC_0V_OFFSET == 0) && (sADCWorkingChannel == eADC_Chan8) )
        {   
            if(sADCSampleData[sADCWorkingChannel] <200)                 //��һ�μ�¼С��200
            BSP_ADC_0V_OFFSET = sADCSampleData[sADCWorkingChannel];     //��¼��Ʈ
        }
        sADCBusyStat = 0;									//����
		HAL_ADC_Stop_IT(hadc);								//ֹͣת��
	}
}

