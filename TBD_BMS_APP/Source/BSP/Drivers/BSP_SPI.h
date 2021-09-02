/****************************************************************************/
/* 	File    	BSP_SPI.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_SPI_H__
#define __BSP_SPI_H__

#include <stdint.h>
#include "main.h"
/**SPI1 GPIO Configuration
PA15     ------> SPI1_NSS		PA15
PB3     ------> SPI1_SCK		PB3
PB4     ------> SPI1_MISO		PB4
PB5     ------> SPI1_MOSI		PB5
*/
#define SPI_BAUD                4000000                          //4M
#define SPI_BaudRatePrescaler   (SystemCoreClock/SPI_BAUD)       //结果必须为大于3的整数 16M 即为4分频     

/*****************************************************  
	* 函数功能: SPI初始化
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：为兼容外部代码改名
******************************************************/
void BSPSPIInit(void);

/*****************************************************
	* 函数功能: SPI 关断
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：为兼容外部代码改名
******************************************************/
void BSPSPIDisable(void);

/*****************************************************
    * 函数功能: SPI发送单字节
	* 输入参数: SPI_TypeDef* hspi 数据：pData
	* 返 回 值: 无
	* 说    明：
******************************************************/
uint8_t BSPSPISendOneData(SPI_TypeDef* hspi , uint8_t pData);

/*****************************************************
    * 函数功能: SPI接收单字节
	* 输入参数: SPI_TypeDef* hspi 
	* 返 回 值: 接收内容
	* 说    明：
******************************************************/
uint8_t BSPSPIRcvOneData(SPI_TypeDef* hspi);

/*****************************************************
    * 函数功能: SPI发送多字节
	* 输入参数: * hspi 数据：pData 长度：Size
    * 返 回 值: 发送结果 0为成功
	* 说    明：
******************************************************/
uint8_t	BSPSPISendNDatas(SPI_TypeDef* hspi , uint8_t *pData, uint16_t Size );

/*****************************************************
    * 函数功能: SPI接收多字节
	* 输入参数: * hspi 数据：pData 长度：Size
	* 返 回 值: 接收结果 0为成功
	* 说    明：
******************************************************/
uint8_t BSPSPIRecNDatas(SPI_TypeDef* hspi , uint8_t *pData, uint16_t Size );

#endif /* __BSP_SPI_H__ */
