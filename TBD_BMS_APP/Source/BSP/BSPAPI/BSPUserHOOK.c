//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BSPUserHOOK.c
//创建人  	: Handry
//创建日期	:
//描述	    : API钩子函数
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//头文件
//=============================================================================================
#include "stm32g0xx_hal.h"
#include "BSP_SYSCLK.h"
#include "BSP_UART.h"
#include "BSP_IWDG.h"
#include "BSP_CAN.h"
#include "BSP_GPIO.h"
#include "BSP_EXIT.h"
#include "BSP_SPI.h"
#include "BSP_FLASH.h"

#include "BSPTypeDef.h"
#include "BSPTaskDef.h"
#include "BSPSystemCfg.h"
#include "BSPUserHOOK.h"
#include "version.h"
#include "CanComm.h"
#include "MAX17205.h"
#include "MX25L16.h"
//#include "PCF85063.h"
#include "SH367309.h"
#include "Comm.h"
#include "Storage.h"
#include "DataDeal.h"
#include "VoltBalance.h"
#include "DTCheck.h"
#include "DTTimer.h"
#include "SOP.h"
#include "Sample.h"
#include "IOCheck.h"
#include "MOSCtrl.h"
#include "ShutDownMode.h"
#include "SleepMode.h"
#include "EventRecord.h"
#include "TestMode.h"
#include "SysState.h"
#include "DA213.h"
#include "SocEepHook.h"
#include "JT808.h"
#ifdef BMS_ENABLE_NB_IOT
#include "GSMTask.h"
#include "WIFIDeal.h"
#endif
//#include "BSPWatchDog.h"

//=============================================================================================
//全局变量定义
//=============================================================================================

//=============================================================================================
//静态函数声明
//=============================================================================================


//=============================================================================================
//函数名称	: void BeforeInitAllHook(void) 
//输入参数	: void
//输出参数	: void
//静态变量	: 
//功	能	: 
//注	意	: 在InitAll之前调用 
//=============================================================================================
void BeforeInitAllHook(void) 
{
    ;
}

//=============================================================================================
//函数名称	: void AfterInitAllHook(void) 
//输入参数	: void
//输出参数	: void
//静态变量	: 
//功	能	: 
//注	意	: 在InitAll之后调用 
//=============================================================================================
void AfterInitAllHook(void)
{
	DataDealDataInit();		//数据处理模块初始化
    CanCommInit();          //CAN通信模块初始化
	SOPInit();				//SOP模块初始化	
	SampleInit();			//采样模块初始化
	IOCheckInit();			//IO检测初始化
	MOSCtrlInit();			//充放mos管控制初始化
    EventRecordInit();      //事件记录初始化
    SysStateInit();         //系统工作状态初始化
    //SleepModeInit();        //睡眠模式初始化
    ShutDownModeInit();     //关机模式初始化
    TestModeInit();         //测试模式初始化
}

//=============================================================================================
//函数名称	: void BeforeEnableIntHook(void)
//输入参数	: void
//输出参数	: void
//静态变量	:
//功	能	:
//注	意	: 在中断开启之前调用
//=============================================================================================
void BeforeEnableIntHook(void)
{		
    SCB->VTOR = 0x08008000;						//跳转至APP后
    /*
    HAL_Init();									//HAL库初始化
    SystemClock_Config();						//系统时钟初始化
    BSPUART_Init(UART_WIFI_DEBUG,UART_WIFI_DEBUG_BAUD);	//WiFi串口 + 调试口
    _UN_NB_Printf("BMS App Start,Bulid Time:%s %s\r\n", __DATE__, __TIME__); //不使用NB时正常打印
	cm_backtrace_init("TBD_BMS_APP", "1.0", "1.0");	//出现hardfault故障解析  
    INT_SYS_EnableIRQGlobal();                  //开中断，跳出屏蔽区，执行任务  	
    */
    BootloaderInit();       					//BootLoader初始化
    CanCommInit();          					//CAN通信模块初始化
}

//=============================================================================================
//函数名称	: void AfterEnableIntHook(void)
//输入参数	: void
//输出参数	: void
//静态变量	:
//功	能	:
//注	意	: 在中断开启之后调用
//=============================================================================================
void AfterEnableIntHook(void)
{
	MX25L16Init();			//MX25L16	FLASH芯片初始化
	StorageInit();			//存储模块初始化
	
    #ifdef BMS_USE_SOX_MODEL
    SocReadEepInit();       //SOC 初始化
	#endif

    BSPFeedWDogTask();	    //喂狗任务
    
    #ifdef    BMS_USE_MAX172XX
	//Max17205Init();			//MAX17205  电量计芯片初始化
    #endif
    
    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
    DA213Init();            //DA213加速度计初始化
    #endif
    
//	PCF85063Init();			//PCF85063	RTC芯片初始化
    SH367309Init();         //SH367309  AFE芯片初始化
    
    //FM11NC08Init();		    //FM11NC08	NFC芯片初始化,在system init处调用
    
    BSPFeedWDogTask();	    //喂狗任务
    
    DTTimerInit();			//故障诊断滤波器初始化
	DTCheckInit();			//故障检测初始化
	VoltBalanceInit();		//均衡模块初始化

	#ifdef CANBUS_MODE_JT808_ENABLE
	JT808_init();			//JT808 初始化
	#endif
    
    #ifdef BMS_ENABLE_NB_IOT
    GSMInit();
    WifiDealInit();
    #endif

	BSPEnterCritical();     //关中断，进入屏蔽区，初始化
	BSPTaskDefInit();		//任务定义初始化----------------------------------------任务初始化入口
	BSPExitCritical();      //开中断，跳出屏蔽区，执行任务
    _UN_NB_Printf("App Init Done... \r\n"); 
}	

/*****************************************end of BSPUserHOOK.c*****************************************/
