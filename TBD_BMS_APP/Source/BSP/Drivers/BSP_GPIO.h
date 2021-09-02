/****************************************************************************/
/* 	File    	BSP_GPIO.h 			 
 * 	Author		Hollis
 *	Notes		MCU使用的所有管脚  位于此处定义
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__
#include <stdint.h>
#include "main.h"
//临时存放定义
//#define		HALL_CHECK_SHIFT			0X01
//#define		REMOVE_CHECK_SHIFT			0X02
//#define		PRE_DCHG_CHECK_SHIFT		0X04
//#define		EXTERN_MODULE_CHECK_SHIFT	0X08    //外置模块
//#define		MAX1720X_FULLCHG_FLAG		0X0080      //满充标志位置
//#define 	STATE_HALL 		            0x0008

////////////////////////////////////////////////////////////////////////////////////////
/*以下为普通IO口***********************************************************************/
/*GPIOA OUT*/
#define	CAN_5V_PORT 		GPIOA			//CAN通讯原边电源供电使能		PA1
#define	CAN_5V_PIN 			GPIO_PIN_1		//高电平有效 管脚描述错误，实际为CAN3.3电源控制  CAN5V由 PWR_EN_14V_5V_PIN 控制

#define	CTL_AFE_PORT 		GPIOA			//AFE充放电优先控制端			PA3
#define	CTL_AFE_PIN			GPIO_PIN_3 		//拉低之后强制关闭充放电MOS

#define	SH367309_ON_PORT 	GPIOA			//AFE工作模式选择开关			PA4
#define	SH367309_ON_PIN 	GPIO_PIN_4 		//默认拉高，拉低之后AFE进入仓运模式（休眠2uA）

#define	VCC_4G_CTL_PORT 	GPIOA			//无线模块供电控制				PA8
#define	VCC_4G_CTL_PIN 		GPIO_PIN_8 		//拉高后给内部模块供电，拉低后关闭给内部模块的供电

#define	PRE_DSG_EN_PORT 	GPIOA			//预充MOS驱动电源使能			PA10
#define	PRE_DSG_EN_PIN 		GPIO_PIN_10 	//预放电开关，默认拉低

#define	FLASH_CS_PORT 		GPIOA			//FLASH芯片CS控制端				PA15
#define	FLASH_CS_PIN 		GPIO_PIN_15 	//通讯之前先拉低  结束后拉高

/*GPIOB OUT*/	
#define	PRE_DSG_NTC_PORT 	GPIOB			//预充NTC检测电路控制端口		PB1
#define	PRE_DSG_NTC_PIN 	GPIO_PIN_1		//开启预放前先拉低，检测预放电电阻的温度低于设定值之后才可以开启预放；预放电期间也拉低使能温度检测；无预放时拉高

#define	MOS_TVS_NTC_PORT 	GPIOB			//NTC检测电路控制端口			PB2
#define	MOS_TVS_NTC_PIN 	GPIO_PIN_2		//TVS、MOS端的NTC检测控制，拉低有效

/*GPIOC OUT*/
#define	FLASH_POW_EN_PORT 	GPIOC			//Flash供电使能					PC8
#define	FLASH_POW_EN_PIN 	GPIO_PIN_8 		//Flash供电，不擦写Flash时默认输出高电平

#define	VPRO_AFE_PORT 		GPIOC			//AFE参数读写使能控制输出		PC11
#define	VPRO_AFE_PIN 		GPIO_PIN_11		//默认拉低，给AFE写EEPROM参数时必须拉高

#define	PWR_EN_14V_5V_PORT 	GPIOC			//DC/DC使能控制					PC13
#define	PWR_EN_14V_5V_PIN	GPIO_PIN_13 	//DCDC的使能引脚：MCU唤醒后立即拉高，进休眠前延时100mS拉低。14V

/*GPIOD OUT*/
#define	DSG_EN_PORT 		GPIOD			//放电管驱动关断开关			PD2
#define	DSG_EN_PIN 			GPIO_PIN_2 		//放电管驱动加速控制

#define	FUSE_DET_EN_PORT 	GPIOD			//保险丝熔断检测电路使能		PD5
#define	FUSE_DET_EN_PIN 	GPIO_PIN_5 		//默认拉低，检测FUSE状态时拉高

#define	VCC_MODULE_PORT 	GPIOD			//外部模块供电使能				PD8
#define	VCC_MODULE_PIN 		GPIO_PIN_8 		//外部4G通信供电,等于P+电压，高电平使能


/*GPIOD IN*/
#define	MODULE_SHORT_PORT 	GPIOA			//模块短路检测输入				PA9
#define	MODULE_SHORT_PIN 	GPIO_PIN_9 	  	//外部模块短路为高电平，外部模块正常为低电平，硬件会关断，使用IO查询

#define	HW_REV0_PORT 		GPIOD			//硬件版本标识口1				PD0
#define	HW_REV0_PIN 		GPIO_PIN_0 		//

#define	HW_REV1_PORT 		GPIOD			//硬件版本标识口2				PD1		
#define	HW_REV1_PIN 		GPIO_PIN_1 		//

#define	FUSE_DET_PORT 		GPIOD			//保险丝熔断检测				PD4
#define	FUSE_DET_PIN 		GPIO_PIN_4 		//FUSE异常时检测到高电平，FUSE正常时检测到低电平

//#define CAN_POWER_ENABLE()	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_SET)
//#define CAN_POWER_DISABLE()	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_RESET)
/*以上为普通IO口***********************************************************************/
////////////////////////////////////////////////////////////////////////////////////////
/*以下为中断输入IO*********************************************************************/
#define	ALARM_AFE_PORT 			GPIOA		//AFE告警信号输入					PA0
#define	ALARM_AFE_PIN 			GPIO_PIN_0 	//中断信号，低电平脉冲，AFE产生保护或完成采集，下降触发
#define	REMOVE_INT_PORT 		GPIOA		//移除检测								PA2
#define	REMOVE_INT_PIN 			GPIO_PIN_2 	//正常时为高电平，电池被拆开见光后为低电平，下降触发
#define	MODULE_INT_PORT 		GPIOA		//4G无线模块中断					PA11
#define	MODULE_INT_PIN 			GPIO_PIN_11 //外部模块接入产生低电平信号，未接入为高电平，下降触发

#define	ACC_INT1_PORT 			GPIOB		//三轴加速计中断1					PB8
#define	ACC_INT1_PIN 			GPIO_PIN_8 	//加速计中断信号1，上升沿触发

#define	INT_4G_PORT 			GPIOC		//内部无线模块中断					PC6
#define	INT_4G_PIN 				GPIO_PIN_6 	//内部4G模组接入，低电平表示接入，下降触发
#define	ACC_INT2_PORT 			GPIOC		//三轴加速计中断2					PC10
#define	ACC_INT2_PIN 			GPIO_PIN_10 //加速计中断信号2，上升沿触发

/*以上为普通IO口***********************************************************************/
////////////////////////////////////////////////////////////////////////////////////////
/*以下为模拟输入IO*********************************************************************/
#define TVS_NTC_PIN 			GPIO_PIN_5	//检测TVS的温度						PA5		
#define MOS_NTC_PIN 			GPIO_PIN_6	//检测MOS的温度						PA6		
#define PRE_NTC_PIN 			GPIO_PIN_7	//检测预充电阻的温度				PA7		
#define NTC_PORT 				GPIOA

#define PRE_DSG_SHORT_DET_PIN 	GPIO_PIN_0	//预充/电路检测输入					PB0
#define PRE_DSG_SHORT_DET_PORT 	GPIOB		//预放电短路检测，AD值待定 

/*以上为模拟输入IO*********************************************************************/
////////////////////////////////////////////////////////////////////////////////////////
/*以下为通讯口IO***********************************************************************/
/*IIC*/
#define	ACC_IIC1_PORT 			GPIOB		//ACC通讯的IIC						IIC1（DA213芯片）
#define	ACC_IIC1_SCL 			GPIO_PIN_13 //ACC通讯的时钟口					PB13
#define	ACC_IIC1_SDA 			GPIO_PIN_14 //ACC通讯的数据口					PB14

#define	AFE_IIC2_PORT 			GPIOB		//AFE的通讯IIC						IIC2（SH367309芯片）
#define	AFE_IIC2_SCL 			GPIO_PIN_6 	//AFE通讯的时钟口					PB6
#define	AFE_IIC2_SDA 			GPIO_PIN_7 	//AFE通讯的数据口					PB7

/*SPI*/
#define	FLASH_SPI1_PORT 		GPIOB		//FLASH通讯的SPI					SPI1（MX25V1606芯片 CS为PA15）
#define	FLASH_SPI1_CLK 			GPIO_PIN_3 	//FLASH通讯的时钟口					PB3
#define	FLASH_SPI1_MISO 		GPIO_PIN_4 	//FLASH通讯的数据口					PB4
#define	FLASH_SPI1_MOSI 		GPIO_PIN_5 	//FLASH通讯的数据口					PB5

/*CAN*/
#define	CAN2_PORT 				GPIOC		//CAN通讯口							CAN2（NSI1050-DSPR芯片）
#define	CAN2_RX 				GPIO_PIN_2 	//CAN通讯口数据接收					PC2
#define	CAN2_TX 				GPIO_PIN_3 	//CAN通讯口数据发送					PC3

/*UART*/
#define	GPS_UASRT1_PORT 		GPIOC		//4G串口通讯						UASRT1
#define GPS_UASRT1_TX 			GPIO_PIN_4 	//4G串口通讯数据发送				PC4
#define	GPS_UASRT1_RX 			GPIO_PIN_5 	//4G串口通讯数据接收				PC5

#define	WIFI_LPUART1_PORT 		GPIOB		//WIFI串口通讯						LPUART1
#define WIFI_LPUART1_RX 		GPIO_PIN_10 //WIFI串口通讯数据接收				PC10
#define	WIFI_LPUART1_TX 		GPIO_PIN_11 //WIFI串口通讯数据发送				PC11

/*以下为通讯口IO***********************************************************************/
////////////////////////////////////////////////////////////////////////////////////////
/*以下特殊引脚*************************************************************************/
//#define MCU_RST 	PF2		//复位脚
//#define SWDIO 		PA13	//DEBUG调试数据输入口
//#define SWCLK 		PA14	//DEBUG调试通讯时钟		同时也是boot0 拉低之后可将MCU启动拉至主FLASH
/**************************************************************************************/

#define PCB_TVS_NTC_SAMP_ON		HAL_GPIO_WritePin(MOS_TVS_NTC_PORT, MOS_TVS_NTC_PIN, GPIO_PIN_RESET);
#define PCB_TVS_NTC_SAMP_OFF	HAL_GPIO_WritePin(MOS_TVS_NTC_PORT, MOS_TVS_NTC_PIN, GPIO_PIN_SET);

#define PRE_DSG_NTC_SAMP_ON   	HAL_GPIO_WritePin(PRE_DSG_NTC_PORT, PRE_DSG_NTC_PIN, GPIO_PIN_RESET); 	//开启预放电 NTC采样控制，测预放电电阻温度
#define PRE_DSG_NTC_SAMP_OFF  	HAL_GPIO_WritePin(PRE_DSG_NTC_PORT, PRE_DSG_NTC_PIN, GPIO_PIN_SET); 	//关闭预放电 NTC采样控制

void MX_GPIO_Init(void);
void BSPGPIOInit(void);
void BSPGPIODeInit(void);

void GPIO_DefaultInit(void);
void CAN_3V_ENABLE(void);
void CAN_3V_DISABLE(void);
void FLASH_POWER_ENABLE(void);
void FLASH_POWER_DISABLE(void);
void FLASH_CS_ENABLE(void);
void FLASH_CS_DISABLE(void);
void PWR_14V_5V_ENABLE(void);
void PWR_14V_5V_DISABLE(void);

void GPIO_SET_INPUT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void GPIO_SET_RISE_EXIT(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
    
#endif /* __BSP_GPIO_H__ */

