/****************************************************************************/
/* 	File    	BSP_ADC.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include <stdint.h>
#include "main.h"
#include "BSPTypeDef.h"
#include "BSP_GPIO.h"
typedef enum
{
	eADC_Chan0 = 0,
	eADC_Chan1 = 1,
	eADC_Chan2 = 2,
	eADC_Chan3 = 3,
	eADC_Chan4 = 4,
	eADC_Chan5 = 5,
	eADC_Chan6 = 6,
	eADC_Chan7 = 7,
	eADC_Chan8 = 8,
	
	eADC_ChanNum
}e_ADCChannel;

#define	    BSP_ADC_MCU_VREF			3300		//MCU供电参考电压
//#define	    BSP_ADC_0V_OFFSET			80		    //采样0V  的数据偏移 每个板不同
#define     BSP_ADC_VREFH               3.3f
#define     BSP_ADC_VREFL               0.0f

#define 	TVS_CHANNEL 			ADC_CHANNEL_5		//TVS转换通道
#define 	MOS_CHANNEL 			ADC_CHANNEL_6		//MOS转换通道
#define 	PRE_DSG_CHANNEL 		ADC_CHANNEL_7		//预放
#define 	PRE_SHORT_CHANNEL 		ADC_CHANNEL_8		//

/*****************************************************
	* 函数功能: ADC初始化
	* 输入参数: ADC_HandleTypeDef can类型
	* 返 回 值: 无
	* 说    明：
******************************************************/
void BSPADCInit(void);

/*****************************************************
	* 函数功能: ADC关断
	* 输入参数: ADC_HandleTypeDef can类型
	* 返 回 值: 无
	* 说    明：
******************************************************/
void BSPADCDisable(void);

uint32_t ADC_Get_Average(uint8_t ch,uint8_t times);

//=============================================================================
//函数名称: u16 BSPADCCalADtoVolt(u16 ad)
//输入参数: 转换ad值为电压值
//输出参数: 转化后的电压值，单位mV
//功能描述: 
//注意事项:
//=============================================================================
float BSPADCCalADtoVolt(uint16_t ad);

//=============================================================================
//函数名称: u16 BSPADCCalADtoRealVolt(u16 ad,u16 mcu3v3)
//输入参数: 转换ad值为真实电压值
//输出参数: 转化后的电压值，单位mV
//功能描述: 根据实际采样的3.3V电压转换AD电压值
//注意事项: 
//=============================================================================
uint16_t BSPADCCalADtoRealVolt(uint16_t ad,uint16_t mcu3v3);
//=============================================================================================
//函数名称	: void BSPADCStartSample(u8 channel)
//输入参数	: channel:0-15;
//输出参数	: void
//静态变量	: void
//功    能	: ADC模块开始采样
//注    意	:
//=============================================================================================
void BSPADCStartSample(uint8_t channel);

//=============================================================================================
//函数名称	: u16 ADCGetSampleData(u8 channel)
//输入参数	: channel:ADC通道号
//输出参数	: 采样值
//函数功能	: 获取采样AD值
//注意事项	:
//=============================================================================================
uint16_t ADCGetSampleData(uint8_t channel);

//=============================================================================================
//函数名称	: u8 BSPADCGetWorkState(void)
//输入参数	: void
//输出参数	: ADC工作状态 1:忙 0:空闲
//函数功能	: 获取ADC工作状态
//注意事项	:
//=============================================================================================
uint8_t BSPADCGetWorkState(void);

//=============================================================================================
//函数名称	: u16 BSPADCDataAvgFilter(u16 *data, u16 len)
//输入参数	: data:滤波数据串 len:滤波数据串长度
//输出参数	: 滤波后的数据值
//函数功能	: 数据平均值滤波
//注意事项	: 除最大最小值
//=============================================================================================
u16 BSPADCDataAvgFilter(u16 *data, u16 len);

#endif /* __BSP_ADC_H__ */
