//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司	
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BSPSystemCfg.h
//创建人  	: Handry
//创建日期	:
//描述	    : 系统配置文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    : 
//修改日期	:
//描述	    :
//=============================================================================================
#ifndef _BSPSYSTEMCFG_H
#define _BSPSYSTEMCFG_H

//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"

//=============================================================================================
//用户类型自定义
//=============================================================================================
//调试的时候，打开，否则屏蔽，除了此处屏蔽，打开设置，点击Linker,编辑S32K118_25_flash.sct也屏蔽此处
//#define     DEBUG_MODE_ENABLE

#define     BMS_USE_SOX_MODEL

#if defined(LFP_TB_20000MAH_20S)
//使用拓邦，双AFE PCBA
#define     USE_B20_TOPBAND_PCBA
#define     USE_DA217

#elif defined(LFP_HL_25000MAH_16S) || defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S)
//使用自研，单AFE PCBA
#define     USE_B21_IM_PCBA

#elif defined(SY_PB_32000MAH_17S)
//使用自研，双AFE PCBA
#define     USE_B22_IM_PCBA

#endif

//-----------------------------------用户模块使能配置----------------------------------------
#define 	System_WDOGEN       1
#define 	System_ADCEN        1
#define 	System_PWMEN        0
#define 	System_CANEN        1
#define 	System_SPI0EN       1
#define 	System_SPI1EN       1
#define 	System_TIMEREN      1	//定时器任务调度使能时开启
#define 	System_EEPROMEN     0
#define 	System_FLASHEN      1
#define 	System_IIC0EN       1
#define 	System_IIC1EN       1
#define 	System_RTCEN        1
#define 	System_UART0EN      0
#define 	System_UART1EN      0
#define 	System_GPIOEN       1
#define 	System_SysTickEN    0
#define 	System_LowPowerEN   1
#define 	System_UARTWifi   	1
#define 	System_UARTGps   		1	
#define 	System_I2C_AFE   		1
#define 	System_I2C_ACC   		1

//总线时钟频率
#define 	BUS_CLK_HZ		 12000000

//----------------------------------WDOG模块配置-----------------------------------------------
#define 	WDOG_FEEDTIME    4000     	//看门狗溢出时间，单位1ms 由10000更为4000


//----------------------------------FLASH模块配置-----------------------------------------------
#define 	FLASH_CLOCK    	BUS_CLK_HZ  //flash配置使用时钟，使用总线时钟


//----------------------------------SCI模块配置------------------------------------------------

#define 	UART_USEBPS     115200     //SCI0波特率

#define 	UART1_USEBPS     115200		//SCI1波特率

#define 	UART2_USEBPS     115200		//SCI2波特率

//----------------------------------SPI模块配置------------------------------------------------
#define		SPI0_BAUDRATE	 4000000		//SPI0波特率		单位:  bps
#define		SPI1_BAUDRATE	 6000000		//SPI1波特率		单位:  bps

#define		SPI0_MASTER		 1    		//SPI0主从机选择  1：主机  0：从机
#define		SPI1_MASTER		 1    		//SPI1主从机选择  1：主机  0：从机

#define		SPI0_MODE		 1    		//SPI0模式选择  	0：mode0	 1：mode1 2：mode2  3：mode3
#define		SPI1_MODE		 0    		//SPI1模式选择  	0：mode0	 1：mode1 2：mode2  3：mode3

#endif

/*****************************************end of BSPSystemCfg.h*****************************************/
