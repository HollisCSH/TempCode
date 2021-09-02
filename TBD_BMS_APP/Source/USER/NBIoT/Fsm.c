//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Fsm.c
//创建人  	: 
//创建日期	:
//描述	    : Sim模组FSM处理文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SimCommon.h"
#include "Fsm.h"
#include "Sim.h"
#include "GprsTlv.h"
#include "SysState.h"
#include "ShutDownMode.h"
#include "ParaCfg.h"
#include "Storage.h"
#include "DataDeal.h"
#include "WIFIDeal.h"
//#include "BSPRTC.h"
#include "BSP_RTC.h"
#include "BSPTask.h"
#include "BSPTaskDef.h"
#include "DTCheck.h"
#include "DTTimer.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
static Queue_old g_FsmQueue;
static Message* g_pMsg = Null;
static FSM_BMS_STATE g_FsmBatState;
//static uint32 g_isGpsUpdated = 0;
uint32 g_ActiveFlag = 0;
int g_MaxGpsCount = MIN_GPS_COUNT;
WakeupType g_WakeupType;
static SwTimer gFsmTimer;

//=============================================================================================
//定义接口函数
//=============================================================================================

//FSM初始化
void Fsm_Init(void)
{
	static Message 	g_QueueBuff[10];
    //队列大小，10
	QUEUE_init(&g_FsmQueue, g_QueueBuff, sizeof(Message), sizeof(g_QueueBuff)/sizeof(Message));

	g_FsmBatState = FSM_BAT_IDLE;
    /* 需要WIFI定位标志 */
    gGSMCond.NeedWifiFlag = True;
    gGSMCond.gsmstartsec = GET_TICKS();
    
    SwTimer_Init(&gFsmTimer, 0, 0);
}

//FSM启动
void Fsm_Start(void)
{
    //上电时设置为空闲状态
	Fsm_BatterySetState(FSM_BAT_IDLE);
}

//设置FSM活跃标志
void Fsm_SetActiveFlag(ActiveFlag af, Bool isActive)
{
	if(isActive)
	{
		g_ActiveFlag |= af;
	}
	else
	{
		g_ActiveFlag &= ~af;
	}
}

//判断FSM是否空闲
Bool Fsm_CanGoSleep()
{ 
    return g_ActiveFlag == 0;
}

//设置电池的状态
void Fsm_BatterySetState(FSM_BMS_STATE state)
{
    //设置为对应的状态
    g_FsmBatState = state;
}

//获取FSM状态
FSM_BMS_STATE Fsm_Get(void)
{
	return g_FsmBatState;
}

//FSM放入信息id
void PostMsg(uint8 msgId)
{
	Message msg = {0};
	
	msg.m_MsgID  = msgId;
	msg.m_Param1 = 0;
	
	QUEUE_add(&g_FsmQueue, &msg, sizeof(Message));
}

//获取唤醒原因
WakeupType GetWakeUpType()
{
	return g_WakeupType;
}

//设置唤醒原因
void SetWakeUpType(WakeupType type)
{
	g_WakeupType = type;
}

//设置GPS点数，暂不使用
void Fsm_SetMaxGpsCount(int value)
{
	g_MaxGpsCount = value;
//	g_isGpsUpdated = 0;
	Sim_Wakeup();
}

//FSM对msgID的信息处理
void Fsm_MsgDeal(uint8 msgID)
{
    #define    POWER_OFF_DELAY    3000
    
    //WIFI点数据更新
    if(MSG_WIFI_MAC_UPDATE == msgID)
    {
        gGSMCond.gsmstartsec = GET_TICKS();
        
        SwTimer_Start(&gFsmTimer,POWER_OFF_DELAY,0);
    }
    
    //测试模式
    if(MSG_PCBA_TEST_START == msgID)
    {      
        gGSMCond.gsmstartsec = GET_TICKS();
        gGSMCond.IsPCBTest = True;
        //启动一次
        BSPRTCSetAlarmAfterSeconds(1); 
    }
    
    #if 0
//	static uint32 tlvticks = 0,gpsticks = 0,RefreshTick = 0;    
//    u8 pbuff[100];
//    u8 size = 100;
//    static u8 poweroff = 0;    //下电标志
//    
//    //TLV事件发生 配合使用PostMsg(MSG_TLV_HAPPEN);
//    if(MSG_TLV_HAPPEN == msgID)
//    {     
//        //在充电时发生热失控、过热
//        if(gBatteryInfo.Status.ChgDchgSta == eChgDchg_Chg)
//        {
//            tlvticks = 0;
//            //不把信息更新向服务器
//            if(GprsTlv_GetChangedTlv(pbuff,size))
//            {
//                GprsTlv_UpdateSentTlv();        //就算超时也需要更新tlv缓存，避免下次无法更新  
//            }
//            
//            gGSMCond.keepupdate = False;
//            gGSMCond.needgpsflag = False;            
//            gGSMCond.needtlvflag = False;            
//            gGSMCond.tlvupdflg = False;
//            gGSMCond.RefreshNum = 0;
//            return ;            
//        }
//        
//        //重新上电
//        if(g_pSimCard->isShutDown)
//        {    
//            g_SimPowerOffDelay = POWERE_OFF_DEFAULT;            
//            Sim_Start();
//            tlvticks = 0;
//            if(gGSMCond.RefreshNum < 360)
//            {
//                gGSMCond.RefreshNum++;
//            }
//            gGSMCond.needtlvflag = True;  //更新TLV标志
//        }  
//        //睡眠唤醒
//        else if(g_pSimCard->isLowPower)
//        {
//            g_SimPowerOffDelay = POWERE_OFF_DEFAULT;     
//            //Sim_Wakeup();
//            Sim_Start();
//            tlvticks = 0;
//            if(gGSMCond.RefreshNum < 360)
//            {
//                gGSMCond.RefreshNum++;
//            }
//            gGSMCond.needtlvflag = True;  //更新TLV标志
//        }        
//        //已启动
//        else
//        {
//            ;//不需要做什么，已经启动的程序会发送tlv数据
//        }
//    }    
//    
//    if(True == gGSMCond.needtlvflag)
//    {
//        if(MSG_TLV_UPDATE == msgID)
//        {
//            tlvticks = 0;   
//            gGSMCond.tlvupdflg = 0;              //已经更新了            
//            gGSMCond.needtlvflag = 0;            
//            if(gGSMCond.RefreshNum > 0)
//            {
//                gGSMCond.keepupdate = True;
//            }
//            else
//            {          
//                gGSMCond.keepupdate = False;
//            }
//        }
//        else
//        {
//            //超过6分钟依旧无法更新tlv包
//            if(0 == tlvticks)
//            {
//                tlvticks = GET_TICKS();
//            }
//            else if(SwTimer_isTimerOutEx(tlvticks, REFRESH_OVER_TIME))    
//            {
//                tlvticks = 0;
//                gGSMCond.keepupdate = False;
//                gGSMCond.RefreshNum = 0;
//                gGSMCond.needtlvflag = False;            
//                gGSMCond.tlvupdflg = False;         //更新超时        
//                GprsTlv_UpdateSentTlv();        //就算超时也需要更新tlv缓存，避免下次无法更新
//                gGSMCond.NeedShdn = True;
//            }  
//        } 
//    }
//    //点数更新完毕后，则下电
//    else if(gGSMCond.keepupdate == True)
//    {
//        if(MSG_GPS_UPDATE == msgID)
//        {   
//            //更新一次清除一次
//            tlvticks = 0;
//        }
//        
//        if(gGSMCond.RefreshNum == 0)
//        {
//            gGSMCond.keepupdate = False;
//        } 
//        else
//        {
//            //超过6分钟依旧无法更新GPS点
//            if(0 == tlvticks)
//            {
//                tlvticks = GET_TICKS();
//            }
//            else if(SwTimer_isTimerOutEx(tlvticks, REFRESH_OVER_TIME))    
//            {
//                tlvticks = 0;
//                gGSMCond.keepupdate = False;
//                gGSMCond.RefreshNum = 0;
//                gGSMCond.NeedShdn = True;
//            }         
//        }
//    }
//    
//    //休眠唤醒 配合使用PostMsg(MSG_SIM_WAKEUP_IN_TIME);
//    //此时是定周期的唤醒，需要发送定位
//    if((MSG_SIM_WAKEUP_IN_TIME == msgID) || (MSG_SIM_ACTIVE_WAKEUP == msgID))
//    {     
//        if(gBatteryInfo.Status.ChgDchgSta == eChgDchg_Chg)
//        {
//            gpsticks = 0; 
//            
//            gGSMCond.keepupdate = False;
//            gGSMCond.needgpsflag = False;            
//            gGSMCond.needtlvflag = False;            
//            gGSMCond.tlvupdflg = False;
//            gGSMCond.RefreshNum = 0;
//            return ;            
//        }        
//        
//        gpsticks = 0;  
//        //重新上电
//        if(g_pSimCard->isShutDown)
//        {     
//            g_SimPowerOffDelay = POWERE_OFF_DEFAULT;             
//            Sim_Start();
//        }    
//        //睡眠唤醒
//        else if(g_pSimCard->isLowPower)
//        {
//            g_SimPowerOffDelay = POWERE_OFF_DEFAULT;     
//            Sim_Start();
//            //Sim_Wakeup();
//        }      
//        
//        gGSMCond.needgpsflag = 1;
//    }

//    if(1 == gGSMCond.needgpsflag)
//    {
//        //此处需要发送最后一个点才能进行休眠
//        if(MSG_GPS_UPDATE == msgID)
//        {       
//            //直到刷新足够的点数，才下电
//            if(gGSMCond.RefreshNum > 0)
//            {
//                gpsticks = 0; 
//            }
//            else
//            {
//                gpsticks = 0;   
//                gGSMCond.needgpsflag = 0;
//            }                
//        }
//        else
//        {
//            //超过6分钟依旧无法下电，则下电退出定位，并准备进入休眠状态
//            if(0 == gpsticks)
//            {
//                gpsticks = GET_TICKS();
//            }
//            else if(SwTimer_isTimerOutEx(gpsticks, REFRESH_OVER_TIME))    
//            {
//                gpsticks = 0;
//                gGSMCond.RefreshNum = 0;
//                gGSMCond.needgpsflag = False;      //超时，不更新了 
//                gGSMCond.NeedShdn = True;                
//            }  
//        } 
//    }
//    
//    //无TLV和唤醒的gps点了，等待更新gGSMCond.RefreshNum完毕
//    if((False == gGSMCond.keepupdate) && (False == gGSMCond.needtlvflag) 
//        && (False == gGSMCond.needgpsflag))
//    {
//        if(MSG_GPS_UPDATE == msgID)
//        {
//            RefreshTick = 0;          
//        }
//        else
//        {
//            //超过6分钟依旧无法下电，则下电退出定位，并准备进入休眠状态
//            if(0 == RefreshTick)
//            {
//                RefreshTick = GET_TICKS();
//            }
//            else if(SwTimer_isTimerOutEx(RefreshTick, REFRESH_OVER_TIME))    
//            {
//                RefreshTick = 0;
//                gGSMCond.RefreshNum = 0;
//                gGSMCond.NeedShdn = True;
//            }  
//        }     
//    }        
//    
//    //测试模式，直接开启模组一直上电
//    if(MSG_PCBA_TEST_START == msgID)
//    {  
//        //重新上电
//        if(g_pSimCard->isShutDown)
//        {
//            if(GprsTlv_GetChangedTlv(pbuff,size))
//            {
//                GprsTlv_UpdateSentTlv();        //就算超时也需要更新tlv缓存，避免下次无法更新  
//            }            
//            gGSMCond.needgpsflag = False;
//            gGSMCond.keepupdate = False;
//            gGSMCond.RefreshNum = 0;
//            gGSMCond.needtlvflag = False;            
//            gGSMCond.tlvupdflg = False;         //更新超时                    
//            Sim_Start();
//        }     
//        //睡眠唤醒
//        else if(g_pSimCard->isLowPower)
//        {
//            Sim_Wakeup();
//        }        
//    } 
//    else if(MSG_PCBA_TEST_END == msgID)
//    {
//        poweroff = 1;
//    }    
//    
//    if(1 == poweroff)
//    {
//        //测试模式退出后，重新激活一次
//        poweroff = 0;   
//        gGSMCond.RefreshNum = 0;
//        PostMsg(MSG_SIM_ACTIVE_WAKEUP);     
//    }
//    
//    if((GET_TICKS() - gSimStartTim > 10000) && (g_pSimCard->isSimStartOK == Sim_Wake_Fail))   //超过5秒没唤醒
//    {
//        g_pSimCard->isSimStartOK = Sim_Wake_Init;
//        g_SimPowerOffDelay = POWERE_OFF_DEFAULT; 
//        Sim_Start();
//    }    
    #endif
}
void Fsm_StopNBTask(void)
{
    BSPTaskStop(TASK_ID_NB_IOT_TASK);
    BSPTaskStop(TASK_ID_WIFI_TASK);        
    
    //BSPSCIDeInit(GSM_UART_CHANNNEL);
    //BSPSCIPinOutputCfg(GSM_UART_CHANNNEL);  //展示不使用测试
    //BSPSCIDeInit(WIFI_UART_CHANNEL);
    //BSPSCIPinOutputCfg(WIFI_UART_CHANNEL); 
    BSPUART_DeInit(GSM_UART_CHANNNEL);
    BSPUART_DeInit(WIFI_UART_CHANNEL);
    
    //需要WIFI定位标志清除
    gGSMCond.NeedWifiFlag = False;
    g_SimPowerOffDelay = POWERE_OFF_DEFAULT;
    SwTimer_Stop(&gFsmTimer);
    
    //测试标志清除
    gGSMCond.IsPCBTest = False;
    
    Sim_ShutDown();
    WifiDealInit();     
}


//电池低电量状态处理
void Fsm_StateBatteryLowSOC(uint8 msgID)
{
    static u8 IsStop = True;
    
    //停止NB任务
    if(IsStop == True)
    {
        IsStop = False;
        Fsm_StopNBTask();
        BSPRTCClrTimeUp();
        BSPRTCStop();
    }
    
    if(IsStop == False)
    {
        //从欠压恢复
        if(!(gBatteryInfo.Fault.FaultInfo[2] & (OPT_FAULT1_UV1|OPT_FAULT1_UV2)))
        {
            IsStop = True;
            Fsm_BatterySetState(FSM_BAT_IDLE);
            gGSMCond.gsmstartsec = GET_TICKS();
            //从欠压恢复后，上报一次点数据
            BSPRTCSetAlarmAfterSeconds(1); 
        }
    }
}

//电池空闲时状态处理
void Fsm_StateBatteryIdle(uint8 msgID)
{
    if((gBatteryInfo.Fault.FaultInfo[2] & (OPT_FAULT1_UV1|OPT_FAULT1_UV2)))
    {
       Fsm_BatterySetState(FSM_BAT_LOWSOC);
        return;
    }
    
    //超时5分钟都没有定位成功
    //无休眠
    if(False == Sim_IsShutDown() && ((GET_TICKS() - gGSMCond.gsmstartsec) > NB_IOT_MAX_WAIT_TIME))
    {
        gGSMCond.gsmstartsec = GET_TICKS();
        
        SwTimer_Start(&gFsmTimer,100,0);
    }
    
    //到达关电延时时间
    if(SwTimer_isTimerOutId(&gFsmTimer,0))
    {
        Fsm_StopNBTask();      
        /* 下一个周期 */
        BSPRTCSetAlarmAfterSeconds(NB_IOT_SEND_WIFI_PEROID); 
    }
    
    if(1 == BSPRTCGetTimeUp())
    {
        BSPRTCClrTimeUp();
        
        GSMInit();
        WifiDealInit();
        
        //启动NB模块任务
        BSPTaskStart(TASK_ID_NB_IOT_TASK, 50);
    }
    #if 0
    static e_ChgDchgStatus laststatus = eChgDchg_MAX;
    
    //首次进入
    if((gBatteryInfo.Status.ChgDchgSta == eChgDchg_Idle) &&
       (laststatus == eChgDchg_MAX))
    {
        laststatus = eChgDchg_Idle;
    }
    //一直处于空闲状态    
    else if((gBatteryInfo.Status.ChgDchgSta == eChgDchg_Idle) &&
       (laststatus == eChgDchg_Idle))
    {   
        //SIM模组睡眠
        if((!g_pSimCard->isLowPower) && (gPCBTest.presentstep < 0x0A))
        {
            Sim_Sleep();
        }
        
        if((gGSMCond.NeedShdn == True) && (gPCBTest.presentstep < 0x0A))
        {
            gGSMCond.NeedShdn = False;
            Sim_ShutDown();
        }
        //SIM模块下电
//        else if((!g_pSimCard->isShutDown) && (gPCBTest.presentstep < 0x0A))
//        {        
//            Sim_PowerOff();
//        }         
        laststatus = eChgDchg_Idle;   
    }
    //空闲状态切换到充电状态
    else if(gBatteryInfo.Status.ChgDchgSta == eChgDchg_Chg)
    {
        //SIM模块下电,如果需要sim模组发送tlv包，则等待发送完毕或超时
//        if(0 == gGSMCond.needtlvflag)
//        {
            Fsm_BatterySetState(FSM_BAT_CHG);
            laststatus = eChgDchg_MAX;
//        }
    }
    //空闲状态切换到放电状态
    else if(gBatteryInfo.Status.ChgDchgSta == eChgDchg_Dchg)
    {
        //重新开始启动
        //Sim_Start();
        Fsm_BatterySetState(FSM_BAT_DCHG);
        laststatus = eChgDchg_MAX;        
    }    
    #endif
}

#if 0
//电池充电时状态处理
void Fsm_StateBatteryChg(uint8 msgID)
{
    (void)msgID;
    
    //切换到充电状态时，一切参数都复位
    //下一次上电的时候，再重新告诉服务器复位的参数
        //标志不需要使用这个参数来进行唤醒间隔
    gGSMCond.ParaWriteFlg = False;          
    gGSMCond.RefreshNum = 0;      
    
    //一直处于充电状态
    if(gBatteryInfo.Status.ChgDchgSta == eChgDchg_Chg)
    {
        //SIM模组睡眠
        if(!g_pSimCard->isLowPower)
        {
            Sim_Sleep();
        }
        
        if((gGSMCond.NeedShdn == True) && (gPCBTest.presentstep < 0x0A))
        {
            gGSMCond.NeedShdn = False;
            Sim_ShutDown();
        }        
        //SIM模块下电
//        else if(!g_pSimCard->isShutDown)
//        {        
//            Sim_PowerOff();
//        }  
    }    
    //充电状态切换到空闲状态
    else if(gBatteryInfo.Status.ChgDchgSta == eChgDchg_Idle)
    {
        Fsm_BatterySetState(FSM_BAT_IDLE);
    }       
    
    //充电状态切换到放电状态，正常情况下不会发生这种情况
    else if(gBatteryInfo.Status.ChgDchgSta == eChgDchg_Dchg)
    {
        Fsm_BatterySetState(FSM_BAT_DCHG);
    }          
}

//电池放电时状态处理
void Fsm_StateBatteryDchg(uint8 msgID)
{
	static uint32 ticks = 0;    
    static e_ChgDchgStatus laststatus = eChgDchg_MAX;    

    
    //首次进入 或者 车停下后，进入了空闲状态，过一会车又开始启动
    if((gBatteryInfo.Status.ChgDchgSta == eChgDchg_Dchg) &&
       ((laststatus == eChgDchg_MAX) || (laststatus == eChgDchg_Idle)))
    {
        //重新上电 
        if(g_pSimCard->isShutDown)
        {  
            g_SimPowerOffDelay = POWERE_OFF_DEFAULT;             
            Sim_Start();
            gGSMCond.needtlvflag = 0; 
        }  
        //睡眠唤醒
        else if(g_pSimCard->isLowPower)
        {
            g_SimPowerOffDelay = POWERE_OFF_DEFAULT;    
            //Sim_Wakeup();
            Sim_Start();
            gGSMCond.needtlvflag = 0; 
        }   
        
        laststatus = eChgDchg_Dchg;
    }
    //一直处于放电状态
    else if((gBatteryInfo.Status.ChgDchgSta == eChgDchg_Dchg) &&
       (laststatus == eChgDchg_Dchg)
    )
    {    
        gGSMCond.RefreshNum = 0;    //清零，开始放电了
        laststatus = eChgDchg_Dchg;   
    }
    //放电状态切换到空闲状态
    else if(gBatteryInfo.Status.ChgDchgSta == eChgDchg_Idle)
    {
        laststatus = eChgDchg_Idle;   
        //此处需要发送最后一个点才能进行休眠
        if(MSG_GPS_UPDATE == msgID)
        {       
            Fsm_BatterySetState(FSM_BAT_IDLE); 
            ticks = 0;            
            laststatus = eChgDchg_MAX;
        }
        else
        {
            //超过6分钟依旧无法下电，则下电退出定位，并准备进入休眠状态
            if(0 == ticks)
            {
                ticks = GET_TICKS();
            }
            else if(SwTimer_isTimerOutEx(ticks, REFRESH_OVER_TIME))    
            {
                Fsm_BatterySetState(FSM_BAT_IDLE);  
                ticks = 0;
                laststatus = eChgDchg_MAX;
                gGSMCond.NeedShdn = True;
            }  
        }        
    }       
    //放电状态切换到充电状态，正常情况下不会发生这种情况,这种情况下直接关闭模组，因为已经在充电柜中
    else if(gBatteryInfo.Status.ChgDchgSta == eChgDchg_Chg)
    {
        Fsm_BatterySetState(FSM_BAT_CHG);     
        ticks = 0;
        laststatus = eChgDchg_MAX;        
    }         
}
#endif

//状态过程处理
void Fsm_BMSMsgProc(void)
{
	int i = 0;
    u8 runflag = 0;    
    
	struct
	{
		uint8 state;
		void (*Handler)(uint8 msgID);
	}
	static const map[] = 
	{
		 {FSM_BAT_IDLE	, Fsm_StateBatteryIdle}
        ,{FSM_BAT_LOWSOC, Fsm_StateBatteryLowSOC}
//		,{FSM_BAT_CHG	, Fsm_StateBatteryChg}
//		,{FSM_BAT_DCHG	, Fsm_StateBatteryDchg}
	};
    
//    //电压太低，则需要关闭模组
//    if((DataDealGetBMSDataRdy(0x20)) && (
//        gBatteryInfo.VoltChara.MinVolt < WAKE_UP_INTIME_VOLT2
//    || (gBatteryInfo.Data.SOC <= WAKE_UP_INTIME_SOC5 )       //电量低于1%
//    ))
//    {
//        if(!g_pSimCard->isShutDown)
//        {
//            Sim_PowerOff(); 
//            gGSMCond.needgpsflag = False;
//            gGSMCond.keepupdate = False;
//            gGSMCond.RefreshNum = 0;
//            gGSMCond.needtlvflag = False;            
//            gGSMCond.tlvupdflg = False;         //更新超时                 
//        }
//        return;
//    }

	//消息处理
	for(g_pMsg = (Message*)QUEUE_getHead(&g_FsmQueue); g_pMsg; QUEUE_removeHead(&g_FsmQueue), g_pMsg = (Message*)QUEUE_getHead(&g_FsmQueue))
	{
        //信息处理
        Fsm_MsgDeal(g_pMsg->m_MsgID);
        runflag++;        
        //空闲关机/开机处理   
        for(i = 0; i < GET_ELEMENT_COUNT(map); i++)
        {
            //根据电池响应的状态去进行处理
            if(g_FsmBatState == map[i].state)
            {
                map[i].Handler(g_pMsg->m_MsgID);
                break;
            }
        }
	}
    
    //空跑
    if(0 == runflag)
    {    
        for(i = 0; i < GET_ELEMENT_COUNT(map); i++)
        {
            //根据电池响应的状态去进行处理
            if(g_FsmBatState == map[i].state)
            {
                map[i].Handler(MSG_MAX);
                break;
            }
        }
    }    
}

//FSM运行处理函数
void Fsm_Run(void)
{
	Fsm_BMSMsgProc();
}

/*****************************************end of Fsm.c*****************************************/
