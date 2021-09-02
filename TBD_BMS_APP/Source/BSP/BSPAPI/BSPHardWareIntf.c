//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BSPHardWareIntf.c
//创建人  	: Handry
//创建日期	:
//描述	    : API函数集
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "BSPSystemCfg.h"
#include "BSPHardWareIntf.h"
//#include "BSPWatchDog.h"
//#include "BSPGPIO.h"
//#include "BSPPIT.h"
//#include "BSPSCI.h"
//#include "BSPICS.h"
//#include "BSPIIC.h"
//#include "BSPSysTick.h"
//#include "BSPKBI.h"
//#include "BSPRTC.h"
//#include "BSPLowPow.h"
//#include "BSPADC.h"
//#include "BSPFLASH.h"
//#include "BSPSPI.h"
//#include "BSPCan.h"

#include "BSP_SYSCLK.h"
#include "BSP_IWDG.h"
#include "BSP_GPIO.h"
#include "BSP_RTC.h"
#include "BSP_SPI.h"
#include "BSP_UART.h"
#include "BSP_ADC.h"
#include "BSP_CAN.h"
#include "BSP_IIC.h"
#include "BSP_TIM.h"
#include "BSP_PWR.h"
#include "cm_backtrace.h"

//=============================================================================================
//全局变量定义
//=============================================================================================


//=============================================================================================
//函数声明
//=============================================================================================

//=============================================================================================
//函数名称	: void BSPSysInit(void) 
//输入参数	: void  
//输出参数	: void
//静态变量	: void
//功	能	: 系统初始化
//注	意	:  
//=============================================================================================
void BSPSysInit(void)
{
	BSPICSInit();		//初始化系统时钟

    if(System_WDOGEN == 1)
    {
    	BSPIDogInit(WDOG_FEEDTIME);
    }

    if(System_GPIOEN == 1)
    {
        BSPGPIOInit();	//GPIO初始化
    }
    
    if(System_RTCEN == 1)
    {
    	BSPRTCInit();	//RTC初始化
    }    
	
    if(System_SPI1EN == 1)
    {
//    	BSPSPIInit(eSPI1,SPI1_MASTER,SPI1_BAUDRATE,SPI1_MODE);	//SPI1初始化
		BSPSPIInit();
    }    

    if(System_UARTWifi == 1)
    {
//    	BSPSCIInit(eSCI1,UART1_USEBPS);
		BSPUART_Init(UART_WIFI_DEBUG,UART_WIFI_DEBUG_BAUD);
        cm_backtrace_init("TBD_BMS_APP", "1.0", "1.0");	//出现hardfault故障解析  
    }
     
	if(System_UARTGps == 1)
    {
//    	BSPSCIInit(eSCI1,UART1_USEBPS);
		BSPUART_Init(UART_GPS,UART_GPS_BAUD);
    }		
    if(System_PWMEN == 1)
    {
        ;
    }

    if(System_ADCEN == 1) 
    {
        BSPADCInit(); //ADC初始化
    }

    if(System_CANEN == 1) 
    {
//        FlexCanInit();
        BSPCan_Init();
    }
    
    if(System_EEPROMEN == 1) 
    {
        ;
    }

    if(System_FLASHEN == 1) 
    {
//        BSPFlashInit(FLASH_CLOCK);	//内部flash初始化
    }

    if(System_I2C_AFE == 1)
    {
        BSPI2C_Init(SH367309_IIC_CHANNEL);	//AFE IIC初始化
    }
    
    if(System_I2C_ACC == 1)
    {
        BSPI2C_Init(DA213_IIC_CHANNEL);	//ACC IIC初始化
    }
    
    if(System_TIMEREN == 1)
    {
        BSPPITInit();		//定时器初始化
    }

    if(System_LowPowerEN == 1)
    {
    	BSPLowPowInit();	//系统低功耗使能
    }
    
    BSPChangeInterruptPrio();	//中断优先级调整  
}

/*****************************************end of BSPHardWareIntf.c*****************************************/
