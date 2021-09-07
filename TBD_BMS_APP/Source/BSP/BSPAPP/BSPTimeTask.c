//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司	
//---------------------------------------文件信息----------------------------------------------
//文件名   	: BSPTimeTask.c
//创建人  	: Handry
//创建日期	:
//描述	    : 时间周期任务
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	: Handry
//版本	    : 
//修改日期	: 2020/07/14
//描述	    : 
//1.增加电流数据处理任务。
//=============================================================================================

//=============================================================================================
//头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "BSP_IWDG.h"
//#include "BSPWatchDog.h"
#include "BSPSystemCfg.h"
#include "MAX17205.h"
//#include "PCF85063.h"
#include "RTC.h"		//用于替代PCF85063.h
#include "SH367309.h"

#include "MX25L16.h"
#include "DTCheck.h"
#include "DTTimer.h"
#include "SOP.h"
#include "Storage.h"
#include "DataDeal.h"
#include "Sample.h"
#include "VoltBalance.h"
#include "IOCheck.h"
#include "UserData.h"
#include "MOSCtrl.h"
#include "Comm.h"
#include "CanComm.h"
//#include "SleepMode.h"
#include "ShutDownMode.h"
#include "EventRecord.h"
//#include "BSPFlash.h"
#include "DA213.h"
#include "Version.h"
#include "CommCtrl.h"
#include "CurrIntegral.h"
#include "SocCapCalc.h"
#include "SocOCVCorr.h"
#include "SocSlideShow.h"
#include "SocPointCorr.h"
#include "SocCapCheck.h"
#include "SocEepHook.h"
#include "SocLTCompensate.h"

//==============================================================================
//全局变量定义
//==============================================================================

//=============================================================================================
//定义接口函数
//=============================================================================================
//=============================================================================================
//函数名称	: void BSPTimeTask10ms(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的10ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask10ms(void)
{  
	CommCheckOffTimeout();  //断线检测任务   
    CanCommCheckOffTimeout();
    DTCheckRefreshFault();	//更新故障告警信息    
	#ifdef BMS_USE_SOX_MODEL
    CurrentIntTask();       //SOX算法：电流积分任务
	#endif
}

//=============================================================================================
//函数名称	: void BSPTimeTask50msBy10ms_1(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的50ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask50msBy10ms_1(void)
{
	#ifdef BMS_USE_SOX_MODEL
    SocCalcTask();          //SOX算法：SOC/SOH计算任务
	#endif
	IOCheckMainTask();		//IO检测任务
    BSPFeedWDogTask();	    //喂狗任务 
}

//=============================================================================================
//函数名称	: void BSPTimeTask50msBy10ms_2(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的50ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask50msBy10ms_2(void)
{
    EventRecordMainTask();  //事件记录任务
}

//=============================================================================================
//函数名称	: void BSPTimeTask50msBy10ms_3(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的50ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask50msBy10ms_3(void)
{
	//SleepModeCheck();		//睡眠检测任务
    ShutDownModeCheck();    //关机检测任务    
    #if defined(BMS_ENABLE_NB_IOT)
    extern void Fsm_Run(void);
    Fsm_Run();
    #endif
}

//=============================================================================================
//函数名称	: void BSPTimeTask50msBy10ms_4(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的50ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask50msBy10ms_4(void)
{
    DTCheckTask();			//故障检测任务    
}

//=============================================================================================
//函数名称	: void BSPTimeTask50msBy10ms_5(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的50ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask50msBy10ms_5(void)
{
	DTTimerTask();		    //故障滤波任务
	MOSCtrlMainTask();      //MOS控制主任务    
}

//=============================================================================================
//函数名称	: void BSPTimeTask100ms(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 100ms驱动的100ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask100ms(void)
{
    VoltBalanceTask();		//均衡任务
	#ifdef BMS_USE_SOX_MODEL
    HisCapCalcTask();       //历史电量记录任务
	#endif
}

//=============================================================================================
//函数名称	: void BSPTimeTask100msBy10ms_1(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的100ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask100msBy10ms_1(void)
{
	DataDealVoltCharaTask();	//电压数据处理
	DataDealTempCharaTask();	//温度数据处理
    DataDealFiltSOCTask();      //SOC数据处理
    DataDealFuelTask();         //电量计数据处理
}

//=============================================================================================
//函数名称	: void BSPTimeTask100msBy10ms_2(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的100ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask100msBy10ms_2(void)
{
    ;
}

//=============================================================================================
//函数名称	: BSPTimeTask100msBy10ms_3(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的100ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask100msBy10ms_3(void)
{
	UserDataLifetimeUpdate();    //更新历史数据
    SysStateCurrentFlagUpdate(); //充放电电流标志状态更新
}

//=============================================================================================
//函数名称	: void BSPTimeTask100msBy10ms_4(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的100ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask100msBy10ms_4(void)
{
	MOSCtrlMOSFaultCheck();     //MOS管故障检测任务
}

//=============================================================================================
//函数名称	: void BSPTimeTask100msBy10ms_5(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的100ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask100msBy10ms_5(void)
{
    ;
}

//=============================================================================================
//函数名称	: void BSPTimeTask100msBy10ms_6(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的100ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask100msBy10ms_6(void)
{
	DTCheckChargeOV();				//过压判断
	DTCheckChargeUV1();				//低压1级判断
	DTCheckChargeUV2();				//低压2级判断
	DTCheckOCLockStatusUpdate();	//过流保护锁定判断
	DTCheckAFEProtectFlag();		//AFE设备状态标志判断
}

//=============================================================================================
//函数名称	: void BSPTimeTask100msBy10ms_7(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的100ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask100msBy10ms_7(void)
{
	MOSCtrlPreDchgTask();       //预放电任务
}

//=============================================================================================
//函数名称	: void BSPTimeTask100msBy10ms_8(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的100ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask100msBy10ms_8(void)
{
    ;
}

//=============================================================================================
//函数名称	: void BSPTimeTask100msBy10ms_9(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的100ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask100msBy10ms_9(void)
{
    CanCommSendRequsetTask();
}

//=============================================================================================
//函数名称	: void BSPTimeTask100msBy10ms_10(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 10ms驱动的100ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask100msBy10ms_10(void)
{
	DTCheckIllegalChgUpdate();     //非法充电、电压异常上升标志更新任务
}

//=============================================================================================
//函数名称	: void BSPTimeTask250ms(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 250ms驱动的250ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask250ms(void)
{
    #ifdef    BMS_USE_MAX172XX
    Max17205SetCtrl(MAX1720X_START_SAMPLE);	    //电量计开始采样
    #endif
    
	SH367309EventFlagSet(SH367309_EVE_CTRL_SMP);//SH367309开始电压、温度、电流采样
	SOPCalcTask();          //SOP计算任务      
    DataDealFiltCurrTask(); //电流数据处理    
}

//=============================================================================================
//函数名称	: void BSPTimeTask500msBy250ms(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 250ms驱动的500ms周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask500msBy250ms(void)
{  
    PCF85063SetCtrl(PCF85063_START_READ);	      //RTC芯片读取时间
    //SysStateRunTest();                          //测试引脚输出方波，运行时调用
}

//=============================================================================================
//函数名称	: void BSPTimeTask1sBy100ms_1(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 100ms驱动的1s周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask1sBy100ms_1(void)
{
	#ifdef BMS_USE_SOX_MODEL
	SocOCVCalcTask();   	//SOX算法：SOC OCV修正任务
	#endif
    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
    DA213SetCtrl(DA213_GET_DATA);
    #endif
}

//=============================================================================================
//函数名称	: void BSPTimeTask1sBy100ms_2(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 100ms驱动的1s周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask1sBy100ms_2(void)
{
	#ifdef BMS_USE_SOX_MODEL
    #if defined(SY_PB_32000MAH_14S) || defined(SY_PB_32000MAH_16S) || defined(SY_PB_32000MAH_17S)
    
    #elif defined(LFP_HL_25000MAH_16S) || defined(LFP_TB_20000MAH_20S) || defined(LFP_GF_25000MAH_16S) || defined(LFP_PH_20000MAH_20S)
    SocPointCorrTask(); //SOX算法：点修正任务	
    #else
    
    #endif    
	#endif
}

//=============================================================================================
//函数名称	: void BSPTimeTask1sBy100ms_3(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 100ms驱动的1s周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask1sBy100ms_3(void)
{
	#ifdef BMS_USE_SOX_MODEL
    SocSlideShowTask(); //SOX算法：SOC平滑显示任务
	#endif
}

//=============================================================================================
//函数名称	: void BSPTimeTask1sBy100ms_4(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 100ms驱动的1s周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask1sBy100ms_4(void)
{
	#ifdef BMS_USE_SOX_MODEL
    TotalCapCheckTask();    //SOX算法：高低压容量修正任务
	#endif
}

//=============================================================================================
//函数名称	: void BSPTimeTask1sBy100ms_5(void)
//输入参数	: void
//输出参数	: void
//函数功能	: 100ms驱动的1s周期任务
//注意事项	:
//=============================================================================================
void BSPTimeTask1sBy100ms_5(void)
{
    #ifdef BMS_USE_SOX_MODEL
    SocLTCompensateTask();
    if(gUserPermitCmd & USER_CLEAR_SOX_DATA)
    {
        gUserPermitCmd &= ~USER_CLEAR_SOX_DATA;
        gSOCInfo.displaySOC = 0;
        gSOCInfo.realSOC = 0;
        SocWriteEepClearAllData();
        SoftReset();
    }
    #endif
    
    #ifdef BMS_ACC_ENABLE   //是否使用加速度传感器
    if(gUserPermitCmd & USER_CALI_ACC_OFFSET)
    {
        gUserPermitCmd &= ~USER_CALI_ACC_OFFSET;        
        DA213SetCtrl(DA213_CALI_OFFSET);
    }
    #endif
}

/*****************************************end of BSPTimeTask.c*****************************************/
