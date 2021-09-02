//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: GSMTask.c
//创建人  	: Handry
//创建日期	:
//描述	    : GSM(SIM模块、NB模块)模块任务代码
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    :
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTask.h"
#include "BmsSystem.h"
//#include "BSPWatchDog.h"
#include "BSP_IWDG.h"
#include "ParaCfg.h"
#include "sim.h"
#include "Fsm.h"
#include "BSPTaskDef.h"
#include "GSMTask.h"
#include "DTCheck.h"
#include "DTTimer.h"
#include "GprsTlv.h"
#include "WIFIDeal.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
uint8 g_isGprsRun = TRUE;
uint8 g_isGpsRun = TRUE;
uint8 g_isLbsRun = False;

t_GSM_COND gGSMCond;    //SIM模组相关变量

//=============================================================================================
//声明静态函数
//=============================================================================================

//=============================================================================================
//定义接口函数
//=============================================================================================

//=============================================================================================
//函数名称	: void GSMInit(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SIM868初始化函数
//注    意	:
//=============================================================================================
void GSMInit(void)
{
    GSMBIDInit();    
    Fsm_Init();
    Fsm_Start();
    Sim_Init();
    
    //复位所有相关参数
    gGSMCond.gsmteststat = 0;
    gGSMCond.gsmfixtime = 0xffff;
    gGSMCond.gsmcsq = 0xff;    
    gGSMCond.gpssignal = 0xff;    
    gGSMCond.sateinview = 0xff;
    
    //gGSMCond.EnterTestEnvir = False;
    gGSMCond.NeedShdn = False;
    
    ///Sim_ShutDown();
    Sim_Start();
}

//=============================================================================================
//函数名称	: void GSMBIDInit(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SIM868 BID初始化
//注    意	:
//=============================================================================================
void GSMBIDInit(void)
{
    u16 snbuff[4] = {0};
    u8 i = 0;
    
	memset(&g_Settings, 0, sizeof(g_Settings));

    for(i = 0;i < 4;i++)
    {
        snbuff[i] = SWAP16(gConfig.sn[i]);
    }
    
    //更新 MAC ID
    memcpy(g_Settings.mac,&snbuff[1],sizeof(g_Settings.mac));

	g_Settings.IsDeactive = False;
}

//=============================================================================================
//函数名称	: void GSMMainTask(void *p)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: SIM868主任务函数
//注    意	:
//=============================================================================================
void GSMMainTask(void *p)
{
    (void)p;
    static u16 i = 0;

    BSPTaskStart(TASK_ID_NB_IOT_TASK, 3);   //3ms周期    

    Sim_Run();  //GSM模块任务；GPS和GPRS任务
    //Fsm_Run();  //GSM FSM任务 在 TIMETASK中运行
    
    if(i++ >= 300)
    {
        i = 0;
        //目的是为了避免等待定位的时候，看门狗不喂狗导致复位
        BSPFeedWDogTask();  //喂狗任务        
    }
}

#if defined(BMS_ENABLE_NB_IOT)
//=============================================================================================
//函数名称	: void IOCheckNBIoTDetectTask(void)
//函数参数	:
//输出参数	:
//静态变量	:
//功    能	: NB模块检测任务
//注    意	: 50ms周期
//=============================================================================================
void IOCheckNBIoTDetectTask(void)
{
    u8 Int4GStat = 0;   //NB 模块接入状态
    static u8 sInt4GLastStat = 2;   //NB 模块接入状态 上次
    static u8 sDelayCnt = 0;
    
    
    //Int4GStat = BSPGPIOGetPin(INT_4G_PORT,INT_4G_PIN);
    Int4GStat = HAL_GPIO_ReadPin(INT_4G_PORT,INT_4G_PIN);
    
    //高电平：不接入；低电平：接入
    if(False == Int4GStat)
    {
        sDelayCnt++;
        if(sDelayCnt >= NB_IOT_VCC_ON_DELAY)
        {
            sDelayCnt = NB_IOT_VCC_ON_DELAY;
            
            if(sInt4GLastStat != Int4GStat)
            {
                NB_IOT_VCC_ON;
                sInt4GLastStat = Int4GStat;
                BSPTaskStart(TASK_ID_NB_IOT_TASK, 500);
            }
        }
    }
    else
    {
        if(sInt4GLastStat != Int4GStat)
        {
            sDelayCnt = 0;
//            BSPSCIDeInit(GSM_UART_CHANNNEL);
//            BSPSCIPinOutputCfg(GSM_UART_CHANNNEL);
//            BSPSCIDeInit(WIFI_UART_CHANNEL);
//            BSPSCIPinOutputCfg(WIFI_UART_CHANNEL);
            BSPUART_DeInit(GSM_UART_CHANNNEL);
            BSPUART_DeInit(WIFI_UART_CHANNEL);
            BSPTaskStop(TASK_ID_NB_IOT_TASK);
            BSPTaskStop(TASK_ID_WIFI_TASK);        
            
            Sim_ShutDown();
            WifiDealInit();
            gGSMCond.NeedWifiFlag = False;
            
            NB_IOT_VCC_OFF;
            sInt4GLastStat = Int4GStat;
        }
    }
}

#endif

/*****************************************end of GSMTask.c*****************************************/
