/****************************************************************************/
/* 	File    	BSP_GPIO.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include <stdint.h>
#include "main.h"
/*GPIOA OUT**********************************************************************/
#define	CAN_5V_PORT 		GPIOA			//CAN通讯原边电源供电使能		管脚描述错误，实际为CAN3.3电源控制  CAN5V由 PWR_EN_PIN 控制
#define	CAN_5V_PIN 			GPIO_PIN_1		//高电平有效

#define	CTL_AFE_PORT 		GPIOA			//AFE充放电优先控制端
#define	CTL_AFE_PIN 		GPIO_PIN_3 		//拉低之后强制关闭充放电MOS

#define	SH367309_ON_PORT 	GPIOA			//AFE工作模式选择开关
#define	SH367309_ON_PIN 	GPIO_PIN_4 		//默认拉高，拉低之后AFE进入仓运模式（休眠2uA）

#define	VCC_4G_CTL_PORT 	GPIOA			//无线模块供电控制	
#define	VCC_4G_CTL_PIN 		GPIO_PIN_8 		//

#define	FLASH_CS_PORT 		GPIOA			//FLASH芯片CS控制端
#define	FLASH_CS_PIN 		GPIO_PIN_15 	//通讯之前先拉低  结束后拉高

/*GPIOB OUT**********************************************************************/	
#define	PRE_DSG_NTC_PORT 	GPIOB			//预充NTC检测电路控制端口
#define	PRE_DSG_NTC_PIN 	GPIO_PIN_1		//开启预放前先拉低，检测预放电电阻的温度低于设定值之后才可以开启预放；预放电期间也拉低使能温度检测；无预放时拉高

#define	MOS_TVS_NTC_PORT 	GPIOB			//NTC检测电路控制端口
#define	MOS_TVS_NTC_PIN 	GPIO_PIN_2		//TVS、MOS端的NTC检测控制，拉低有效

/*GPIOC OUT**********************************************************************/	
#define	FLASH_POW_EN_PORT   GPIOC			//Flash供电使能
#define	FLASH_POW_EN_PIN 	GPIO_PIN_8 		//Flash供电，不擦写Flash时默认输出高电平

#define	VPRO_AFE_PORT 		GPIOC			//AFE参数读写使能控制输出
#define	VPRO_AFE_PIN 		GPIO_PIN_11		//默认拉低，给AFE写EEPROM参数时必须拉高	读取SH367309的参数拉低

#define	PWR_EN_PORT 		GPIOC			//DC/DC使能控制 输出14V
#define	PWR_EN_PIN 			GPIO_PIN_13 	//DCDC的使能引脚：MCU唤醒后立即拉高，进休眠前延时100mS拉低。

/*GPIOD OUT**********************************************************************/	
#define	DSG_EN_PORT 		GPIOD			//放电管驱动关断开关
#define	DSG_EN_PIN 			GPIO_PIN_2 		//放电管驱动加速控制

#define	FUSE_DET_EN_PORT 	GPIOD			//保险丝熔断检测电路使能
#define	FUSE_DET_EN_PIN 	GPIO_PIN_5 		//默认拉低，检测FUSE状态时拉高

#define	VCC_MODULE_PORT 	GPIOD			//外部模块供电使能
#define	VCC_MODULE_PIN 		GPIO_PIN_8 		//外部4G通信供电,等于P+电压，高电平使能

#define	PRE_DSG_EN_PORT 	GPIOD			//预充MOS驱动电源使能
#define	PRE_DSG_EN_PIN 		GPIO_PIN_11 	//预放电开关，默认拉低

/*GPIOD IN**********************************************************************/	
#define	HW_REV0_PORT 		GPIOD			//硬件版本标识口1
#define	HW_REV0_PIN 		GPIO_PIN_0 		//

#define	HW_REV1_PORT 		GPIOD			//硬件版本标识口2
#define	HW_REV1_PIN 		GPIO_PIN_1 		//

#define	FUSE_DET_PORT 		GPIOD			//保险丝熔断检测
#define	FUSE_DET_PIN 		GPIO_PIN_4 		//FUSE异常时检测到高电平，FUSE正常时检测到低电平


/*****************************************************
	* 函数功能: GPIO端口 关断
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void BSPGPIOInit(void);

/*****************************************************
	* 函数功能: GPIO端口 关断
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void BSPGPIODeInit(void);

/*****************************************************
	* 函数功能: 使能CAN通讯芯片 供电
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void CAN_POWER_ENABLE(void);

/*****************************************************
	* 函数功能: 禁止CAN通讯芯片 断电
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void CAN_POWER_DISABLE(void);

/*****************************************************
	* 函数功能: 禁止FLASH通讯芯片 断电
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void FLASH_POWER_ENABLE(void);

/*****************************************************
	* 函数功能: 禁止FLASH通讯芯片 断电
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void FLASH_POWER_DISABLE(void);

/*****************************************************
	* 函数功能: 使能FLASH芯片 通讯
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void FLASH_CS_ENABLE(void);

/*****************************************************
	* 函数功能: 禁止FLASH芯片 通讯
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void FLASH_CS_DISABLE(void);

/*****************************************************
	* 函数功能: 使能14V电源输出
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void PWR_EN_ENABLE(void);

/*****************************************************
	* 函数功能: 禁止14V电源输出
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void PWR_EN_DISABLE(void);

#endif /* __BSP_GPIO_H__ */

