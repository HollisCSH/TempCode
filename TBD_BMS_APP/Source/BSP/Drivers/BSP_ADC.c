/****************************************************************************/
/* 	File    	BSP_ADC.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    	Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_ADC.h"
#include "Sample.h"
#include "stdio.h"

/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
static uint8_t 	sADCBusyStat = 0;	                    //ADC采样 忙标志，0：空闲；1：忙
static uint8_t 	sADCWorkingChannel = 0;                 //ADC正在工作的通道号
static uint16_t sADCSampleData[eADC_ChanNum] = {0};  	//ADC各通道采样值
/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/

/*****************************************************
	* 函数功能: ADC端口、时钟、中断初始化
	* 输入参数: ADC_HandleTypeDef 类型
	* 返 回 值: 无
	* 说    明：该类函数在调用HAL_XX_Init 会内联
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
	* 函数功能: ADC端口、时钟、中断 关断
	* 输入参数: ADC_HandleTypeDef 类型
	* 返 回 值: 无
	* 说    明：该类函数在调用HAL_XX_DeInit 会内联
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
	* 函数功能: ADC1经典配置
	* 输入参数: void
	* 返 回 值: void
	* 说    明：
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
        assert_param(0);                  //断言 错误提示
    }
}

/*****************************************************
	* 函数功能: ADC初始化
	* 输入参数: ADC_HandleTypeDef can类型
	* 返 回 值: 无
	* 说    明：为兼容外部代码改名
******************************************************/
void BSPADCInit(void)
{
	MX_ADC1_Init();
}

/*****************************************************
	* 函数功能: ADC关断
	* 输入参数: ADC_HandleTypeDef can类型
	* 返 回 值: 无
	* 说    明：为兼容外部代码改名
******************************************************/
void BSPADCDisable(void)
{
	HAL_ADC_DeInit(&hadc1);
}

/*****************************************************
	* 函数功能: 获取ADC转换值
	* 输入参数: 转换通道 ch 转换次数 times
	* 返 回 值: 无
	* 说    明：
******************************************************/
uint32_t ADC_Get_Average(uint8_t ch,uint8_t times)
{
	ADC_ChannelConfTypeDef sConfig;		//通道初始化
	uint32_t value_sum=0;	
	uint8_t i;
	switch(ch)							//选择ADC通道
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
	sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;		//采用周期239.5周期
	sConfig.Rank = ADC_REGULAR_RANK_1;
	HAL_ADC_ConfigChannel(&hadc1,&sConfig);											
	for(i=0;i<times;i++)
	{
		HAL_ADC_Start(&hadc1);								//启动转换
//		HAL_ADC_PollForConversion(&hadc1,10);				//等待转化结束
		if(HAL_ADC_PollForConversion(&hadc1,10) == HAL_OK)
		{
			value_sum += HAL_ADC_GetValue(&hadc1);			//求和					
			HAL_ADC_Stop(&hadc1);							//停止转换
		}
	}
	return value_sum/times;									//返回平均值
}

////////////////////////////////////////////////////////////////////////////////
//=============================================================================================
//函数名称	: void BSPADCStartSample(u8 channel)
//输入参数	: channel:0-15;
//输出参数	: void
//静态变量	: void
//功		能	: ADC模块开始采样
//注		意	:
//=============================================================================================
void BSPADCStartSample(uint8_t channel)
{
	ADC_ChannelConfTypeDef sConfig;		//通道初始化
	sADCBusyStat = 1;	//标记为忙状态
	sADCWorkingChannel = channel;
	switch(channel)							//选择ADC通道
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
	sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;		//采用周期239.5周期
	sConfig.Rank = ADC_REGULAR_RANK_1;
	HAL_ADC_ConfigChannel(&hadc1,&sConfig);			
	HAL_ADC_Start_IT(&hadc1);								//启动转换
 
}
//=============================================================================
//函数名称: u16 BSPADCCalADtoVolt(u16 ad)
//输入参数: 转换ad值为电压值
//输出参数: 转化后的电压值，单位mV
//功能描述: 
//注意事项:
//=============================================================================
static uint16_t BSP_ADC_0V_OFFSET = 0;
float BSPADCCalADtoVolt(uint16_t ad)
{
    uint16_t adcMax;
    uint16_t u16adval;
    float adcValue;    
    /* Get ADC max value from the resolution */  
    adcMax = (uint16_t) (1 << 12);      			//12位采样
    /* Process the result to get the value in volts */
    ad = ad>BSP_ADC_0V_OFFSET?ad-BSP_ADC_0V_OFFSET:0;   //消除零飘
    adcMax = adcMax - BSP_ADC_0V_OFFSET;                //消除零飘
    adcValue = ((float) ad / adcMax) * (BSP_ADC_VREFH - BSP_ADC_VREFL);    
    u16adval = adcValue * 1000;
    
    return u16adval;   
}
//=============================================================================
//函数名称: u16 BSPADCCalADtoRealVolt(u16 ad,u16 mcu3v3)
//输入参数: 转换ad值为真实电压值
//输出参数: 转化后的电压值，单位mV
//功能描述: 根据实际采样的3.3V电压转换AD电压值
//注意事项: 
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
//函数名称	: u16 ADCGetSampleData(u8 channel)
//输入参数	: channel:ADC通道号
//输出参数	: 采样值
//函数功能	: 获取采样AD值
//注意事项	:
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
//函数名称	: u8 BSPADCGetWorkState(void)
//输入参数	: void
//输出参数	: ADC工作状态 1:忙 0:空闲
//函数功能	: 获取ADC工作状态
//注意事项	:
//=============================================================================================
uint8_t BSPADCGetWorkState(void)
{
	return (sADCBusyStat);
}
//=============================================================================================
//函数名称	: u16 BSPADCDataAvgFilter(u16 *data, u16 len)
//输入参数	: data:滤波数据串 len:滤波数据串长度
//输出参数	: 滤波后的数据值
//函数功能	: 数据平均值滤波
//注意事项	: 除最大最小值
//=============================================================================================
u16 BSPADCDataAvgFilter(u16 *data, u16 len)
{
	u8 i = 0;
	u16 max = 0;
	u16 min = 0;
	u32 sum = 0;
	u16 result = 0;

	if(0 == len)                //长度错误
	{
	    result = 0xffff;
	}
	else if(len < 3)            //只有1个或者2个数
	{
		sum = data[0] + data[len - 1];
		result = (u16)(sum / 2);
	}
	else                        //不少于3个数
	{
    	max = data[0];
    	min = data[0];
    	sum = data[0];

    	for(i = 1; i < len; i++)
    	{
    		if(data[i] > max)   //求最大值
    		{
    			max = data[i];
    		}

    		if(data[i] < min)	//求最小值
    		{
    			min = data[i];
    		}

    		sum += data[i];	    //求和
    	}

    	result = (u16)((sum - max - min) / (len - 2));
   }

	return(result);
}
/*****************************************************
	* 函数功能: ADC转换完成回调函数
	* 输入参数: ADC_HandleTypeDef类型
	* 返 回 值: void
	* 说    明：系统函数 由stm32g0xx_it.c的中断函数引入
******************************************************/
void  HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if( hadc->Instance == ADC1 )
	{
		sADCSampleData[sADCWorkingChannel] = HAL_ADC_GetValue(hadc);
        if( (BSP_ADC_0V_OFFSET == 0) && (sADCWorkingChannel == eADC_Chan8) )
        {   
            if(sADCSampleData[sADCWorkingChannel] <200)                 //第一次记录小于200
            BSP_ADC_0V_OFFSET = sADCSampleData[sADCWorkingChannel];     //记录零飘
        }
        sADCBusyStat = 0;									//空闲
		HAL_ADC_Stop_IT(hadc);								//停止转换
	}
}

