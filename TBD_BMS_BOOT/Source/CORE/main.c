//=======================================Copyright(c)==========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: main.c
//创建人  	: Hollis
//创建日期	:
//描述	    : 主函数
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//头文件定义
//=============================================================================================
#include "main.h"
#include "BSP_CAN.h"
#include "BSP_GPIO.h"
#include "BSP_IWDG.h"
#include "BSP_SYSCLK.h"
#include "BootLoader.h"
#include "CanComm.h"
//#define 	BOOT_DIRECT_JUMP		    //直接跳转，取消boot检验，方便测试
#ifdef DEBUG
#include "BSP_UART.h"
#include "cm_backtrace.h"
#endif

int main(void)
{
    HAL_Init();							//HAL库初始化
    SystemClock_Config();				//系统时钟初始化	
	#ifdef BOOT_DIRECT_JUMP
    JumpToApplication();			    //直接跳转版本boot
	#endif	
    
    #ifdef DEBUG
	BSPUART_Init(UART_WIFI_DEBUG,UART_WIFI_DEBUG_BAUD);	//WiFi串口 + 调试口
    _Debug_Printf("BMS Boot Start,Bulid Time:%s %s\r\n", __DATE__, __TIME__);
	cm_backtrace_init("TBD_BMS_BOOT", "1.0", "1.0");
    #endif   
    
	BootloaderInit();                   //BootLoader初始化 如果APP存在则直接跳转 不会执行以下代码
	BSPGPIOInit();	                    //GPIO初始化	
	MX_FDCAN2_Init();					//CAN口
	MX_IWDG_Init(4095);					//看门狗初始化	
	INT_SYS_EnableIRQGlobal();          //开中断	
	CanCommInit();                      //CAN数据存储初始化
	CAN_POWER_ENABLE();                 //相关电源使能
    HAL_Delay(10);
    while (1)
    {
        BootLoaderDealTask();           //BootLoader升级处理
    }
}

/************************ (C) COPYRIGHT IMMOTOR *****END OF FILE****/
