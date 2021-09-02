//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BSPTaskDef.c
//创建人  	: Handry
//创建日期	:
//描述	    : 任务调度模块定义文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTimeTask.h"
#include "BSPUserHOOK.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "Comm.h"
#include "CanComm.h"
#include "SH367309.h"
#include "MAX17205.h"
#include "RTC.h"		//用于替代PCF85063.h
//#include "PCF85063.h"
#include "Storage.h"
#include "Sample.h"
#include "TestMode.h"
#include "DA213.h"
#ifdef BMS_ENABLE_NB_IOT
#include "GSMTask.h"
#include "WIFIDeal.h"
#endif

//=============================================================================================
//定义全局变量
//=============================================================================================

//=============================================================================================
//声明静态函数
//=============================================================================================

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void BSPTaskDefInit(void)
//输入参数	: void  
//输出参数	: void
//函数功能	: COS定周期初始化
//注意事项	: 
//=============================================================================================
void BSPTaskDefInit(void)
{
	//任务调度初始化
	BSPTaskInit();

	//定时任务配置
	BSPTaskCreate(TASK_ID_TIMER10MS, BSPTaskDefTimer10ms, (void *)0);
	BSPTaskCreate(TASK_ID_TIMER100MS, BSPTaskDefTimer100ms, (void *)0);
	BSPTaskCreate(TASK_ID_TIMER250MS, BSPTaskDefTimer250ms, (void *)0);

	//定时任务开启
	BSPTaskStart(TASK_ID_TIMER10MS, 4);
	BSPTaskStart(TASK_ID_TIMER100MS, 5);
	BSPTaskStart(TASK_ID_TIMER250MS, 6);

	//NFC通信任务配置
//	BSPTaskCreate(TASK_ID_COMM_TASK, CommMainTask, (void *)0);
//	BSPTaskStart(TASK_ID_COMM_TASK, 7);

//	BSPTaskCreate(TASK_ID_NFC_TASK, FM11NC08MainTask, (void *)0);
//	BSPTaskStart(TASK_ID_NFC_TASK, 8);
	
    //CAN通信任务配置
	BSPTaskCreate(TASK_ID_CAN_COMM_TASK, CanCommMainTask, (void *)0);
	BSPTaskStart(TASK_ID_CAN_COMM_TASK, 8);	
	//MAX17205电量计任务配置
    #ifdef    BMS_USE_MAX172XX
    BSPTaskCreate(TASK_ID_MAX17205_TASK, Max17205MainTask, (void *)0);
    BSPTaskStart(TASK_ID_MAX17205_TASK, 9);
    #endif
    
    //SH367309 AFE任务配置
	BSPTaskCreate(TASK_ID_SH367309_TASK, SH367309MainTask, (void *)0);
	BSPTaskStart(TASK_ID_SH367309_TASK, 10);
    
	//电压/温度采样配置
	BSPTaskCreate(TASK_ID_SAMPLE_TASK, SampleMainTask, (void *)0);
	BSPTaskStart(TASK_ID_SAMPLE_TASK, 11);

	//存储任务配置
	BSPTaskCreate(TASK_ID_STORAGE_TASK, StorageMainTask, (void *)0);
	BSPTaskStart(TASK_ID_STORAGE_TASK, 13);

	//RTC任务配置
	BSPTaskCreate(TASK_ID_PCF85063_TASK, PCF85063MainTask, (void *)0);
	BSPTaskStart(TASK_ID_PCF85063_TASK, 30);

	//PCB测试任务配置，此任务先不执行
	BSPTaskCreate(TASK_ID_PCBTEST_TASK, TestModePCBTestTask, (void *)0);
    
    //DA213加速度计任务
    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
    BSPTaskCreate(TASK_ID_ACCELERO_TASK,DA213MainTask,(void *)0);
	BSPTaskStart(TASK_ID_ACCELERO_TASK, 40);    
    #endif  
	
//外置模块任务
#ifdef CANBUS_MODE_JT808_ENABLE
	//JT808任务
	extern void JT808TaskEnter(void *p );
	BSPTaskCreate(TASK_ID_JT808_TASK,JT808TaskEnter,(void *)0);
	BSPTaskStart(TASK_ID_JT808_TASK, 30);
#endif

//漏电流检测任务
#ifdef BMS_ENABLE_LITTLE_CURRENT_DET
	BSPTaskCreate(TASK_ID_LOW_CURR_SAM_TASK,SampleLowCurrentTask,(void *)0);
    //BSPTaskStart(TASK_ID_LOW_CURR_SAM_TASK, 30);
#endif

//NB-IOT任务
#ifdef BMS_ENABLE_NB_IOT
    //NB IOT模块任务配置
    BSPTaskCreate(TASK_ID_NB_IOT_TASK, GSMMainTask, (void *)0);
//    BSPTaskStart(TASK_ID_NB_IOT_TASK, 50); //开启电源后启动
    BSPTaskCreate(TASK_ID_WIFI_TASK, WifiDealMainTask, (void *)0);
//    BSPTaskStart(TASK_ID_WIFI_TASK, 50);   //开启电源后启动
#endif

}

//=============================================================================================
//函数名称	: void BSPTaskDefTimer10ms(void *p)
//输入参数	: *p 未使用,语法需要 
//输出参数	: void
//函数功能	: 驱动10ms的定时任务 
//注意事项	:  
//=============================================================================================
void BSPTaskDefTimer10ms(void *p)
{
	static u8 state = 0;

	BSPTaskStart(TASK_ID_TIMER10MS, 10);	    //10ms启动任务
	
	(void)p;
	
	BSPTimeTask10ms();                         //10ms周期
	
	switch (state)
	{
    	case 0:
    		BSPTimeTask50msBy10ms_1();
    		BSPTimeTask100msBy10ms_1();
    		break;
    		
    	case 1:
    		BSPTimeTask50msBy10ms_2();
    		BSPTimeTask100msBy10ms_2();
    		break;
    		
    	case 2:
    		BSPTimeTask50msBy10ms_3();
    		BSPTimeTask100msBy10ms_3();
    		break;
    		
    	case 3:
    		BSPTimeTask50msBy10ms_4();
    		BSPTimeTask100msBy10ms_4();
    		break;
    		
    	case 4:
    		BSPTimeTask50msBy10ms_5();
    		BSPTimeTask100msBy10ms_5();
    		break;
    		
    	case 5:
    		BSPTimeTask50msBy10ms_1();
    		BSPTimeTask100msBy10ms_6();
    		break;
    		
    	case 6:
    		BSPTimeTask50msBy10ms_2();
    		BSPTimeTask100msBy10ms_7();
    		break;
    		
    	case 7:
    		BSPTimeTask50msBy10ms_3();
    		BSPTimeTask100msBy10ms_8();
    		break;
    		
    	case 8:
    		BSPTimeTask50msBy10ms_4();
    		BSPTimeTask100msBy10ms_9();
    		break;
    		
    	case 9:
    		BSPTimeTask50msBy10ms_5();
    		BSPTimeTask100msBy10ms_10();
    		break;

    	default:
    		state = 0;
    		break;                    //记录错误进入"case 0"
	}

	state ++;
	if(state >= 10)
	{
		state = 0;
	}
}

//=============================================================================================
//函数名称	: void BSPTaskDefTimer100ms(void *p)
//输入参数	: *p 未使用,语法需要 
//输出参数	: void
//函数功能	: 驱动100ms的定时任务 
//注意事项	:  
//=============================================================================================
void BSPTaskDefTimer100ms(void *p)
{
	static u8 state  = 0;

	BSPTaskStart(TASK_ID_TIMER100MS, 100);   //启动任务
	
	(void)p;
	
	BSPTimeTask100ms();                        //100ms任务
											
	switch(state)                       //1s周期
	{
    	case 0:
    		BSPTimeTask1sBy100ms_1();
    		break;

    	case 1:
    		;
    		break;

    	case 2:
    		BSPTimeTask1sBy100ms_2();
    		break;

    	case 3:
    		;
    		break;

    	case 4:
    		BSPTimeTask1sBy100ms_3();
    		break;

    	case 5:
    		;
    		break;

    	case 6:
    		BSPTimeTask1sBy100ms_4();
    		break;

    	case 7:
    		;
    		break;

    	case 8:
    		BSPTimeTask1sBy100ms_5();
    		break;

    	case 9:
    		;
    		break;

    	default:
    		state = 0;
    		break;                    //记录错误进入"case 0"
	}
	
	state++;
	if(state >= 10)
	{
		state = 0;
	}
}

//=============================================================================================
//函数名称	: void BSPTaskDefTimer250ms(void *p)
//输入参数	: *p 未使用,语法需要
//输出参数	: void
//函数功能	: 驱动250ms的定时任务 
//注意事项	:  
//=============================================================================================
void BSPTaskDefTimer250ms(void *p)
{
	static u8 state  = 0;
    
	BSPTaskStart(TASK_ID_TIMER250MS, 250);   //启动任务
	
	(void)p;
	
	BSPTimeTask250ms();                 //250ms周期
    
	switch(state)                       //500ms周期
	{
    	case 0:
            ;
    		break;

    	case 1:
    		BSPTimeTask500msBy250ms();
    		break;

    	default:
    		state = 0;
    		break;                    //记录错误进入"case 0"
	}
	
	state++;
	if(state >= 2)
	{
		state = 0;
	}
}

/*****************************************end of BSPTaskDef.c*****************************************/
